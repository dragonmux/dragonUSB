// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/core.hxx"
#include "usb/device.hxx"
#include "usb/internal/device.hxx"

using namespace usb::constants;
using namespace usb::types;
using namespace usb::core;
using namespace usb::descriptors;
using namespace usb::device::internal;

namespace usb::device
{
	void setupEndpoint(const usbEndpointDescriptor_t &endpoint, uint16_t &startAddress)
	{
		if (endpoint.endpointType == usbEndpointType_t::control)
			return;

		const auto direction{static_cast<endpointDir_t>(endpoint.endpointAddress & ~vals::usb::endpointDirMask)};
		const auto endpointNumber{uint8_t(endpoint.endpointAddress & vals::usb::endpointDirMask)};
		usbCtrl.epIndex = endpointNumber;
		auto &epCtrl{usbCtrl.epCtrls[endpointNumber - 1]};
		if (direction == endpointDir_t::controllerIn)
		{
			const auto statusCtrlH
			{
				[](const usbEndpointType_t type) noexcept
				{
					switch (type)
					{
						case usbEndpointType_t::isochronous:
							return vals::usb::epTxStatusCtrlHModeIsochronous;
						default:
							break;
					}
					return vals::usb::epTxStatusCtrlHModeBulkIntr;
				}(endpoint.endpointType)
			};
			epCtrl.txStatusCtrlH = (epCtrl.txStatusCtrlH & vals::usb::epTxStatusCtrlHMask) | statusCtrlH;
			epCtrl.txDataMax = endpoint.maxPacketSize;
			usbCtrl.txFIFOSize = vals::usb::fifoMapMaxSize(endpoint.maxPacketSize, vals::usb::fifoSizeDoubleBuffered);
			usbCtrl.txFIFOAddr = vals::usb::fifoAddr(startAddress);
			usbCtrl.txIntEnable |= uint16_t(1U << endpointNumber);
		}
		else
		{
			epCtrl.rxStatusCtrlH |= vals::usb::epRxStatusCtrlHDTSWriteEn;
			epCtrl.rxStatusCtrlH = (epCtrl.rxStatusCtrlH & vals::usb::epRxStatusCtrlHMask);
			epCtrl.rxDataMax = endpoint.maxPacketSize;
			usbCtrl.rxFIFOSize = vals::usb::fifoMapMaxSize(endpoint.maxPacketSize, vals::usb::fifoSizeDoubleBuffered);
			usbCtrl.rxFIFOAddr = vals::usb::fifoAddr(startAddress);
			usbCtrl.rxIntEnable |= uint16_t(1U << endpointNumber);
		}
		startAddress += uint16_t(endpoint.maxPacketSize * 2U);
	}

	bool handleSetConfiguration() noexcept
	{
		usb::core::resetEPs(epReset_t::user);
		usb::core::deinitHandlers();

		const auto config{packet.value.asConfiguration()};
		if (config > configsCount)
			return false;
		activeConfig = config;

		if (activeConfig == 0)
			usbState = deviceState_t::addressed;
		else if (activeConfig <= configsCount)
		{
			// EP0 consumes the first 256 bytes of USB RAM.
			uint16_t startAddress{256};
			usbCtrl.txIntEnable &= vals::usb::txItrEnableMask;
			usbCtrl.rxIntEnable &= vals::usb::rxItrEnableMask;
			usbCtrl.txIntEnable |= vals::usb::txItrEnableEP0;

			const auto descriptors{configDescriptors[activeConfig - 1U]};
			for (const auto &part : descriptors)
			{
				const auto *const descriptor{static_cast<const std::byte *>(part.descriptor)};
				usbDescriptor_t type{usbDescriptor_t::invalid};
				memcpy(&type, descriptor + 1, 1);
				if (type == usbDescriptor_t::endpoint)
				{
					const auto endpoint{*static_cast<const usbEndpointDescriptor_t *>(part.descriptor)};
					setupEndpoint(endpoint, startAddress);
				}
			}
			usb::core::initHandlers();
		}
		else
			return false;
		return true;
	}

	/*!
	* @returns true when the all the data to be read has been retreived,
	* false if there is more left to fetch.
	*/
	bool readCtrlEP() noexcept
	{
		auto &epStatus{epStatusControllerOut[0]};
		auto readCount{usbCtrl.ep0Ctrl.rxCount};
		// Bounds sanity and then adjust how much is left to transfer
		if (readCount > epStatus.transferCount)
			readCount = uint8_t(epStatus.transferCount);
		epStatus.transferCount -= readCount;
		epStatus.memBuffer = recvData(0, static_cast<uint8_t *>(epStatus.memBuffer), readCount);
		// Mark the FIFO contents as done with
		if (epStatus.transferCount || usbCtrlState == ctrlState_t::statusRX)
			usbCtrl.ep0Ctrl.statusCtrlL |= vals::usb::epStatusCtrlLRxReadyClr;
		else
			usbCtrl.ep0Ctrl.statusCtrlL |= vals::usb::epStatusCtrlLRxReadyClr | vals::usb::epStatusCtrlLDataEnd;
		return !epStatus.transferCount;
	}

	/*!
	* @returns true when the data to be transmitted is entirely sent,
	* false if there is more left to send.
	*/
	bool writeCtrlEP() noexcept
	{
		auto &epStatus{epStatusControllerIn[0]};
		const auto sendCount{[&]() noexcept -> uint8_t
		{
			// Bounds sanity and then adjust how much is left to transfer
			if (epStatus.transferCount < epBufferSize)
				return uint8_t(epStatus.transferCount);
			return epBufferSize;
		}()};
		epStatus.transferCount -= sendCount;

		if (!epStatus.isMultiPart())
			epStatus.memBuffer = sendData(0, static_cast<const uint8_t *>(epStatus.memBuffer), sendCount);
		else
		{
			std::array<uint8_t, 4> leftoverBytes{};
			uint8_t leftoverCount{};

			if (!epStatus.memBuffer)
				epStatus.memBuffer = epStatus.partsData->part(0).descriptor;
			auto sendAmount{sendCount};
			while (sendAmount)
			{
				const auto &part{epStatus.partsData->part(epStatus.partNumber)};
				const auto *const begin{static_cast<const uint8_t *>(part.descriptor)};
				const auto partAmount{[&]() -> uint8_t
				{
					const auto *const buffer{static_cast<const uint8_t *>(epStatus.memBuffer)};
					const auto amount{part.length - uint16_t(buffer - begin)};
					if (amount > sendAmount)
						return sendAmount;
					return uint8_t(amount);
				}()};
				sendAmount -= partAmount;
				// If we have bytes left over from the previous loop
				if (leftoverCount)
				{
					// How many bytes do we need to completely fill the leftovers buffer
					const auto diffAmount{uint8_t(leftoverBytes.size() - leftoverCount)};
					// Copy that in and queue it from the front of the new chunk
					memcpy(leftoverBytes.data() + leftoverCount, epStatus.memBuffer, diffAmount);
					sendData(0, leftoverBytes.data(), leftoverBytes.size());

					// Now compute how many bytes will be left at the end of this new chunk
					// in queueing only amounts divisable-by-4
					const auto remainder{uint8_t((partAmount - diffAmount) & 0x03U)};
					// Queue as much as we can
					epStatus.memBuffer = sendData(0, static_cast<const uint8_t *>(epStatus.memBuffer) + diffAmount,
						uint8_t((partAmount - diffAmount) - remainder)) + remainder;
					// And copy any new leftovers to the leftovers buffer.
					memcpy(leftoverBytes.data(), static_cast<const uint8_t *>(epStatus.memBuffer) - remainder, remainder);
					leftoverCount = remainder;
				}
				else
				{
					// How many bytes will be left over by queueing only a divisible-by-4 amount
					const auto remainder{uint8_t(partAmount & 0x03U)};
					// Queue as much as we can
					epStatus.memBuffer = sendData(0, static_cast<const uint8_t *>(epStatus.memBuffer),
						partAmount - remainder) + remainder;
					// And copy the leftovers to the leftovers buffer.
					memcpy(leftoverBytes.data(), static_cast<const uint8_t *>(epStatus.memBuffer) - remainder, remainder);
					leftoverCount = remainder;
				}
				// Get the buffer back to check if we exhausted it
				const auto *const buffer{static_cast<const uint8_t *>(epStatus.memBuffer)};
				if (buffer - begin == part.length &&
						epStatus.partNumber + 1 < epStatus.partsData->count())
					// We exhausted the chunk's buffer, so grab the next chunk
					epStatus.memBuffer = epStatus.partsData->part(++epStatus.partNumber).descriptor;
			}
			if (!epStatus.transferCount)
			{
				if (leftoverCount)
					sendData(0, leftoverBytes.data(), leftoverCount);
				epStatus.isMultiPart(false);
			}
		}
		// Mark the FIFO contents as done with
		if (epStatus.transferCount || usbCtrlState == ctrlState_t::statusTX)
			usbCtrl.ep0Ctrl.statusCtrlL |= vals::usb::ep0StatusCtrlLTxReady;
		else
			usbCtrl.ep0Ctrl.statusCtrlL |= vals::usb::ep0StatusCtrlLTxReady | vals::usb::epStatusCtrlLDataEnd;
		return !epStatus.transferCount;
	}

	void handleControlPacket() noexcept
	{
		if (usbCtrl.ep0Ctrl.statusCtrlL & vals::usb::epStatusCtrlLSetupEnd)
			usbCtrl.ep0Ctrl.statusCtrlL |= vals::usb::epStatusCtrlLSetupEndClr;
		// If we received a packet..
		if (usbPacket.dir() == endpointDir_t::controllerOut)
		{
			if (usbCtrlState == ctrlState_t::idle)
				handleSetupPacket();
			else
				handleControllerOutPacket();
		}
		else
			handleControllerInPacket();
	}
} // namespace usb::device
