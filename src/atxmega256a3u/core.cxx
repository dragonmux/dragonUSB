// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/internal/core.hxx"
#include "usb/platforms/atxmega256a3u/core.hxx"
#include "usb/device.hxx"
#include <substrate/indexed_iterator>

/*!
 * "USB IN" transfers == Controller In
 * "USB OUT" transfers == Controller Out
 * We are always the Peripheral, so this is unabiguous.
 */

using namespace usb::constants;
using namespace usb::types;
using namespace usb::core::internal;

namespace usb::core
{
	namespace internal
	{
		// These are organised EPxOut, EPxIn, etc
		alignas(2) std::array<endpointCtrl_t, endpointCount> endpoints{};
		std::array<std::array<uint8_t, epBufferSize>, endpointCount * 2> epBuffer{};
	} // namespace internal

	void init() noexcept
	{
		// Enable the USB peripheral
		USB.CTRLB &= uint8_t(~vals::usb::ctrlBAttach);
		USB.CTRLA = vals::usb::ctrlAUSBEnable | vals::usb::ctrlAModeFullSpeed | vals::usb::ctrlAMaxEP(2);

		for (auto [i, endpoint] : substrate::indexedIterator_t{endpoints})
		{
			endpoint.controllerOut.DATAPTR = reinterpret_cast<uint16_t>(epBuffer[i << 1U].data());
			endpoint.controllerOut.CNT = 0;
			endpoint.controllerOut.STATUS = vals::usb::usbEPStatusNACK0 | vals::usb::usbEPStatusNACK1;
			endpoint.controllerIn.DATAPTR = reinterpret_cast<uint16_t>(epBuffer[(i << 1U) + 1U].data());
			endpoint.controllerIn.CNT = 0;
			endpoint.controllerIn.STATUS = vals::usb::usbEPStatusNACK0 | vals::usb::usbEPStatusNACK1;
			if (i)
			{
				// Stall yet to be configured endpoints
				endpoint.controllerOut.CTRL = USB_EP_TYPE_DISABLE_gc | vals::usb::usbEPCtrlStall;
				endpoint.controllerIn.CTRL = USB_EP_TYPE_DISABLE_gc | vals::usb::usbEPCtrlStall;
			}
			else
			{
				// Configure EP0 as the control endpoint
				endpoint.controllerOut.CTRL = USB_EP_TYPE_CONTROL_gc | USB_EP_BUFSIZE_64_gc |
					vals::usb::usbEPCtrlItrDisable;
				endpoint.controllerIn.CTRL = USB_EP_TYPE_CONTROL_gc | USB_EP_BUFSIZE_64_gc |
					vals::usb::usbEPCtrlItrDisable;
			}
		}

		USB.EPPTR = reinterpret_cast<uint16_t>(endpoints.data());

		// Initialise the state machine
		usbState = deviceState_t::detached;
		usbCtrlState = ctrlState_t::idle;
		usbDeferalFlags = 0;
	}

	void attach() noexcept
	{
		// Reset all USB interrupts
		USB.INTCTRLA &= vals::usb::intCtrlAClearMask;
		USB.INTCTRLB &= vals::usb::intCtrlBClearMask;

		// Ensure the device address is 0
		USB.ADDR &= uint8_t(~vals::usb::addressMask);
		// Ensure we're in the unconfigured configuration
		usb::device::activeConfig = 0;
		// Enable the USB reset interrupt
		USB.INTCTRLA |= vals::usb::intCtrlAEnableBusEvent | USB_INTLVL_LO_gc;
		// Attach to the bus
		USB.CTRLB = vals::usb::ctrlBAttach;
	}

	void detach() noexcept
	{
		// Detach from the bus
		USB.CTRLB &= uint8_t(~vals::usb::ctrlBAttach);
		// Reset all USB interrupts
		USB.INTCTRLA &= vals::usb::intCtrlAClearMask;
		USB.INTCTRLB &= vals::usb::intCtrlBClearMask;
		// Ensure that the current configuration is torn down
		deinitHandlers();
		// Switch to the unconfigured configuration
		usb::device::activeConfig = 0;
	}

	void address(const uint8_t value) noexcept { USB.ADDR = value & vals::usb::addressMask; }
	uint8_t address() noexcept { return USB.ADDR & uint8_t(~vals::usb::addressMask); }

	void reset()
	{
		resetEPs(epReset_t::all);

		// Once we get done, idle the peripheral
		USB.ADDR &= uint8_t(~vals::usb::addressMask);
		usbState = deviceState_t::attached;
		USB.INTCTRLA |= vals::usb::intCtrlAEnableBusEvent;
		USB.INTCTRLB |= vals::usb::intCtrlBEnableIOComplete | vals::usb::intCtrlBEnableSetupComplete;
		endpoints[0].controllerOut.CTRL &= uint8_t(~vals::usb::usbEPCtrlItrDisable);
		endpoints[0].controllerIn.CTRL &= uint8_t(~vals::usb::usbEPCtrlItrDisable);
		USB.INTFLAGSACLR = vals::usb::itrStatusReset;
	}

	void resetEPs(const epReset_t what) noexcept
	{
		for (auto [i, endpoint] : substrate::indexedIterator_t{endpoints})
		{
			if (what == epReset_t::user && i == 0)
				continue;

			endpoint.controllerOut.CTRL |= vals::usb::usbEPCtrlItrDisable;
			endpoint.controllerOut.CTRL &= uint8_t(~vals::usb::usbEPCtrlStall);
			if (i != 0)
			{
				endpoint.controllerOut.CTRL &= uint8_t(~vals::usb::usbEPCtrlTypeMask);
				endpoint.controllerIn.CTRL &= uint8_t(~vals::usb::usbEPCtrlTypeMask);
				endpoint.controllerOut.STATUS |= vals::usb::usbEPStatusNACK0 | vals::usb::usbEPStatusNACK1;
				endpoint.controllerOut.STATUS &= uint8_t(~(vals::usb::usbEPStatusNotReady |
					vals::usb::usbEPStatusStall | vals::usb::usbEPStatusIOComplete |
					vals::usb::usbEPStatusSetupComplete));
			}
			else
			{
				endpoint.controllerOut.STATUS &= uint8_t(~vals::usb::usbEPStatusNACK0);
				endpoint.controllerOut.STATUS |= vals::usb::usbEPStatusNACK1;
			}

			endpoint.controllerIn.CTRL |= vals::usb::usbEPCtrlItrDisable;
			endpoint.controllerIn.CTRL &= uint8_t(~vals::usb::usbEPCtrlStall);
			endpoint.controllerIn.STATUS |= vals::usb::usbEPStatusNACK0 | vals::usb::usbEPStatusNACK1;
			endpoint.controllerIn.STATUS &= uint8_t(~(vals::usb::usbEPStatusNotReady |
				vals::usb::usbEPStatusStall | vals::usb::usbEPStatusIOComplete |
				vals::usb::usbEPStatusSetupComplete));
		}
		usb::core::common::resetEPs(what);
	}

	void wakeup()
	{
		usbSuspended = false;
		//USB.CTRLB |= vals::usb::ctrlBRemoteWakeUp;
		USB.INTFLAGSACLR = vals::usb::itrStatusResume;
	}

	void suspend()
	{
		usbSuspended = true;
		USB.INTFLAGSACLR = vals::usb::itrStatusSuspend;
	}

	const void *sendData(const uint8_t endpoint, const void *const bufferPtr, const uint8_t length,
		const uint8_t offset = 0) noexcept
	{
		auto *const inBuffer{static_cast<const uint8_t *>(bufferPtr)};
		auto *const outBuffer{epBuffer[(endpoint << 1U) + 1U].data() + offset};
		// Copy the data to tranmit from the user buffer
		if (epStatusControllerIn[endpoint].memoryType() == memory_t::sram)
		{
			for (uint8_t i{0}; i < length; ++i)
				outBuffer[i] = inBuffer[i];
		}
		else
		{
			flash_t<char *> flashBuffer{static_cast<const char *>(bufferPtr)};
			for (uint8_t i{0}; i < length; ++i)
			{
				outBuffer[i] = uint8_t(*flashBuffer);
				++flashBuffer;
			}
		}
		return inBuffer + length;
	}

	void *recvData(const uint8_t endpoint, void *const bufferPtr, const uint16_t length) noexcept
	{
		const auto *const inBuffer{epBuffer[(endpoint << 1U)].data()};
		auto *const outBuffer{static_cast<uint8_t *>(bufferPtr)};
		// Copy the received data to the user buffer
		for (uint8_t i{0}; i < length; ++i)
			outBuffer[i] = inBuffer[i];
		return outBuffer + length;
	}

	uint16_t readEPDataAvail(const uint8_t endpoint) noexcept
		{ return endpoints[endpoint].controllerOut.CNT; }

	/*!
	 * @returns true when the all the data to be read has been retreived,
	 * false if there is more left to fetch.
	 */
	bool readEP(const uint8_t endpoint) noexcept
	{
		auto &epStatus{epStatusControllerOut[endpoint]};
		auto &epCtrl{endpoints[endpoint].controllerOut};
		auto readCount{epCtrl.CNT};
		// Bounds sanity and then adjust how much is left to transfer
		if (readCount > epStatus.transferCount)
			readCount = epStatus.transferCount;
		epStatus.transferCount -= readCount;
		epStatus.memBuffer = recvData(endpoint, epStatus.memBuffer, readCount);
		// Mark the recv buffer contents as done with
		epCtrl.CNT = 0;
		epCtrl.STATUS &= (vals::usb::usbEPStatusNACK1 | vals::usb::usbEPStatusDTS);
		return !epStatus.transferCount;
	}

	/*!
	 * @returns true when the data to be transmitted is entirely sent,
	 * false if there is more left to send.
	 */
	bool writeEP(const uint8_t endpoint) noexcept
	{
		auto &epStatus{epStatusControllerIn[endpoint]};
		auto &epCtrl{endpoints[endpoint].controllerIn};
		const auto sendCount{[&]() noexcept -> uint8_t
		{
			// Bounds sanity and then adjust how much is left to transfer
			if (epStatus.transferCount < epBufferSize)
				return uint8_t(epStatus.transferCount);
			return epBufferSize;
		}()};
		epStatus.transferCount -= sendCount;

		if (!epStatus.isMultiPart())
			epStatus.memBuffer = sendData(endpoint, epStatus.memBuffer, sendCount);
		else
		{
			if (!epStatus.memBuffer)
				epStatus.memBuffer = (*epStatus.partsData.part(0)).descriptor;
			auto sendAmount{sendCount};
			uint8_t sendOffset{0};
			while (sendAmount)
			{
				const auto part{*epStatus.partsData.part(epStatus.partNumber)};
				auto *const begin{static_cast<const uint8_t *>(part.descriptor)};
				const auto partAmount{[&]() -> uint8_t
				{
					auto *const buffer{static_cast<const uint8_t *>(epStatus.memBuffer)};
					const auto amount{uint8_t(part.length - uint8_t(buffer - begin))};
					if (amount > sendAmount)
						return sendAmount;
					return amount;
				}()};
				sendAmount -= partAmount;
				epStatus.memBuffer = sendData(endpoint, epStatus.memBuffer, partAmount, sendOffset);
				sendOffset += partAmount;
				// Get the buffer back to check if we exhausted it
				auto *const buffer{static_cast<const uint8_t *>(epStatus.memBuffer)};
				if (uint8_t(buffer - begin) == part.length &&
						epStatus.partNumber + 1 < epStatus.partsData.count())
					// We exhausted the chunk's buffer, so grab the next chunk
					epStatus.memBuffer = (*epStatus.partsData.part(++epStatus.partNumber)).descriptor;
			}
			if (!epStatus.transferCount)
				epStatus.isMultiPart(false);
		}
		// Mark the buffer as ready to send
		epCtrl.CNT = sendCount;
		epCtrl.STATUS &= uint8_t(~(vals::usb::usbEPStatusNotReady | vals::usb::usbEPStatusNACK0));
		return !epStatus.transferCount;
	}

	bool readEPReady(const uint8_t endpoint) noexcept
	{
		auto &epCtrl{endpoints[endpoint].controllerOut};
		return epCtrl.STATUS & uint8_t(vals::usb::usbEPStatusIOComplete | vals::usb::usbEPStatusSetupComplete);
	}

	bool writeEPBusy(const uint8_t endpoint) noexcept
	{
		auto &epCtrl{endpoints[endpoint].controllerIn};
		return !(epCtrl.STATUS & uint8_t(vals::usb::usbEPStatusIOComplete | vals::usb::usbEPStatusSetupComplete));
	}

	void stallEP(const uint8_t endpoint) noexcept
	{
		auto &epCtrl{endpoints[endpoint].controllerIn};
		epCtrl.CTRL |= vals::usb::usbEPCtrlStall;
	}

	void flushWriteEP(const uint8_t endpoint) noexcept
	{
		auto &epCtrl{endpoints[endpoint].controllerIn};
		epCtrl.STATUS |= vals::usb::usbEPStatusNotReady | vals::usb::usbEPStatusNACK0;
	}

	uint8_t statusEP(const uint8_t endpoint, const endpointDir_t dir) noexcept
	{
		auto &epCtrl
		{
			[dir](endpointCtrl_t &endpointCtrl) -> USB_EP_t &
			{
				if (dir == endpointDir_t::controllerIn)
					return endpointCtrl.controllerIn;
				else
					return endpointCtrl.controllerOut;
			}(endpoints[endpoint])
		};
		return (epCtrl.STATUS & vals::usb::usbEPStatusStall) ? 1 : 0;
	}

	template<endpointDir_t direction> void handlePacket(const uint8_t endpoint)
	{
		const auto status{[=]()
		{
			if constexpr (direction == endpointDir_t::controllerOut)
				return endpoints[endpoint].controllerOut.STATUS;
			else
				return endpoints[endpoint].controllerIn.STATUS;
		}()};

		if ((status & vals::usb::usbEPStatusIOComplete) ||
			(direction == endpointDir_t::controllerOut &&
			(status & vals::usb::usbEPStatusSetupComplete)))
		{
			usbPacket.endpoint(endpoint);
			usbPacket.dir(direction);

			if (endpoint == 0)
				usb::device::handleControlPacket();
		}
	}

	void handleIRQ() noexcept
	{
		const auto intCtrl{USB.INTCTRLA};
		const auto status{USB.INTFLAGSASET};

		if (usbState == deviceState_t::attached)
			usbState = deviceState_t::powered;

		if ((status & vals::usb::itrStatusResume) && (intCtrl & vals::usb::intCtrlAEnableBusEvent))
			usb::core::wakeup();
		/*else if (usbSuspended)
			return;*/

		if ((status & vals::usb::itrStatusReset) && (intCtrl & vals::usb::intCtrlAEnableBusEvent))
		{
			usb::core::reset();
			usbState = deviceState_t::waiting;
			return;
		}
		else if ((status & vals::usb::itrStatusSuspend) && (intCtrl & vals::usb::intCtrlAEnableBusEvent))
			usb::core::suspend();

		// Handle SOF here.

		if (usbState == deviceState_t::detached ||
			usbState == deviceState_t::attached ||
			usbState == deviceState_t::powered)
		{
			USB.INTFLAGSBCLR = vals::usb::itrStatusSetup | vals::usb::itrStatusIOComplete;
			return;
		}

		USB.INTFLAGSBCLR = vals::usb::itrStatusIOComplete;

		for (uint8_t endpoint{}; endpoint < /*usb::endpointCount*/1; ++endpoint)
		{
			handlePacket<endpointDir_t::controllerIn>(endpoint);
			handlePacket<endpointDir_t::controllerOut>(endpoint);
		}
	}
} // namespace usb::core
