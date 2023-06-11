// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/internal/core.hxx"
#include "usb/internal/device.hxx"

using namespace usb::constants;
using namespace usb::types;
using namespace usb::core;
using namespace usb::core::internal;
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

	namespace internal
	{
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
			else
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
			return true;
		}
	} // namespace internal

	void handleControlPacket() noexcept
	{
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
