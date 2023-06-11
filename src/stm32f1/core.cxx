// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/internal/core.hxx"
#include "usb/platforms/stm32f1/core.hxx"
#include "usb/device.hxx"
#include <substrate/index_sequence>

/*!
 * USB pinout:
 * PA11 - D-
 * PA12 - D+
 *
 * PA8 - FS Pull-Up resistor
 * PA15 - VBus
 */

using namespace usb::constants;
using namespace usb::types;
using namespace usb::core::internal;

namespace usb::core
{
	void init() noexcept
	{
		// Enable the clocks for the USB peripheral
		rcc.apb1PeriphClockEn |= vals::rcc::apb1PeriphClockEnUSB;
		// Enable the clocks for the GPIO Port A peripheral
		rcc.apb2PeriphClockEn |= vals::rcc::apb2PeriphClockEnGPIOPortA;

		// Set up the port pins used by the USB controller so they're in the right modes
		// with the USB controller connected through.
		vals::gpio::clear(gpioA, vals::gpio_t::pin8);
		vals::gpio::config<vals::gpio_t::pin8>(gpioA, vals::gpio::mode_t::input, vals::gpio::config_t::inputFloating);

		// Having configured the pins, we now need to set up the USB controller
		// Clear the control register, releasing power down and forced reset on the controller,
		// set the buffer table pointer to the start of the USB SRAM, and clear pending interrupts.
		usbCtrl.ctrl &= vals::usb::controlMask;
		usbCtrl.bufferTablePtr = 0;
		usbCtrl.intStatus &= vals::usb::itrStatusClearMask;

		// Enable the USB NVIC slot we use
		nvic.enableInterrupt(vals::irqs::usbLowPriority);

		// Initialise the state machine
		usbState = deviceState_t::detached;
		usbCtrlState = ctrlState_t::idle;
		usbDeferalFlags = 0;
	}

	void attach() noexcept
	{
		// Reset all USB interrupts
		usbCtrl.ctrl &= vals::usb::controlItrMask;
		// And their flags
		usbCtrl.intStatus &= vals::usb::itrStatusClearMask;

		// Ensure the device address is 0
		usbCtrl.address = 0 | vals::usb::addressUSBEnable;
		// Ensure we're in the unconfigured configuration
		usb::device::activeConfig = 0;
		// Ensure we can respond to reset interrupts
		usbCtrl.ctrl |= vals::usb::controlResetItrEn;
		// Attach to the bus
		vals::gpio::set(gpioA, vals::gpio_t::pin8);
		vals::gpio::config<vals::gpio_t::pin8>(gpioA, vals::gpio::mode_t::output2MHz,
			vals::gpio::config_t::outputNormalPushPull);
	}

	void detach() noexcept
	{
		// Detach from the bus
		usbCtrl.address = 0;
		// Reset all USB interrupts
		usbCtrl.ctrl &= vals::usb::controlItrMask;
		// Ensure that the current configuration is torn down
		deinitHandlers();
		// Switch to the unconfigured configuration
		usb::device::activeConfig = 0;
	}

	void address(const uint8_t value) noexcept
	{
		usbCtrl.address = (usbCtrl.address & vals::usb::addressClrMask) |
			(value & vals::usb::addressMask);
	}

	uint8_t address() noexcept { return usbCtrl.address & vals::usb::addressMask; }

	void reset() noexcept
	{
		// Set up only EP0.
		resetEPs(epReset_t::all);
		// Because of the 512 byte memory limit of the peripheral buffers,
		// we overlay the TX and RX buffers on each other for EP0.
		internal::setupEndpoint(vals::usb::endpoint(vals::usb::endpointDir_t::controllerOut, 0),
			usbEndpointType_t::control, 0, 64);
		internal::setupEndpoint(vals::usb::endpoint(vals::usb::endpointDir_t::controllerIn, 0),
			usbEndpointType_t::control, 0, 64);
		// Enable the endpoint for receiving SETUP packets
		usbCtrl.epCtrlStat[0] = (usbCtrl.epCtrlStat[0] & vals::usb::epCtrlRXMask) | vals::usb::epCtrlRXValid;

		// Once we get done, idle the peripheral
		usbCtrl.address = 0;
		usbState = deviceState_t::attached;
		usbCtrl.ctrl |= vals::usb::controlSOFItrEn | vals::usb::controlCorrectXferItrEn |
			vals::usb::controlWakeupItrEn;
		usb::device::activeConfig = 0;
	}

	void resetEPs(const epReset_t what) noexcept
	{
		for (const auto endpoint : substrate::indexSequence_t{vals::usb::endpoints})
		{
			if (what == epReset_t::user && endpoint == 0)
				continue;
			usbCtrl.epCtrlStat[endpoint] &= vals::usb::epClearMask;
			usbCtrl.epCtrlStat[endpoint] |= vals::usb::epAddress(endpoint);
		}
		usb::core::common::resetEPs(what);
	}

	namespace internal
	{
		void setupEndpoint(const uint8_t endpoint, const usbEndpointType_t type, const uint16_t bufferAddress,
			const uint16_t bufferLength) noexcept
		{
			auto &epTable{*reinterpret_cast<stm32::usbEPTable_t *>(stm32::packetBufferBase +
				static_cast<uintptr_t>(usbCtrl.bufferTablePtr))};
			const auto direction{static_cast<endpointDir_t>(endpoint & ~vals::usb::endpointDirMask)};
			const auto endpointNumber{uint8_t(endpoint & vals::usb::endpointDirMask)};

			auto epCtrl{usbCtrl.epCtrlStat[endpointNumber]};
			auto &epConfig{epTable[endpointNumber]};

			// NB: we assume both IN and OUT endpoints have a consistent type here as there are only 8 endpoint
			// registers and the types are shared between the two halves
			epCtrl &= vals::usb::epCtrlTypeMask;
			epCtrl |= [&]()
			{
				switch (type)
				{
					case usbEndpointType_t::control:
						return vals::usb::epCtrlTypeControl;
					case usbEndpointType_t::bulk:
						return vals::usb::epCtrlTypeBulk;
					case usbEndpointType_t::interrupt:
						return vals::usb::epCtrlTypeInterrupt;
					case usbEndpointType_t::isochronous:
						return vals::usb::epCtrlTypeIsochronous;
				}
				// This should never bit hit.. but.. just in case.
				return vals::usb::epCtrlTypeBulk;
			}();

			if (direction == endpointDir_t::controllerIn)
			{
				epCtrl &= vals::usb::epCtrlTXMask;
				epCtrl |= vals::usb::epCtrlTXNack;
				epConfig.txAddress = bufferAddress;
			}
			else
			{
				epCtrl &= vals::usb::epCtrlRXMask;
				epCtrl |= vals::usb::epCtrlRXNack;
				epConfig.rxAddress = bufferAddress;
				epConfig.rxCount = vals::usb::rxBufferSize(bufferLength);
			}

			usbCtrl.epCtrlStat[endpointNumber] = epCtrl;
		}
	} // namespace internal

	void wakeup() noexcept
	{
		usbSuspended = false;
		// Clear forced suspend (low-power mode is already cancelled at this point by hardware)
		usbCtrl.ctrl &= ~vals::usb::controlForceSuspend;
		// Switch over the interrupt source being used
		usbCtrl.ctrl = (usbCtrl.ctrl & ~vals::usb::controlWakeupItrEn) | vals::usb::controlSuspendItrEn;
	}

	void suspend() noexcept
	{
		// Suspend the controller, setting it in low power mode
		usbCtrl.ctrl |= vals::usb::controlForceSuspend | vals::usb::controlLowPowerMode;
		// Switch o ver the interrupt source being used
		usbCtrl.ctrl = (usbCtrl.ctrl & ~vals::usb::controlSuspendItrEn) | vals::usb::controlWakeupItrEn;
		usbSuspended = true;
	}

	void handleIRQ() noexcept
	{
		const auto status{usbCtrl.intStatus & vals::usb::itrStatusMask};
		usbCtrl.intStatus &= vals::usb::itrStatusClearMask;

		if (usbState == deviceState_t::attached)
		{
			usbCtrl.ctrl |= vals::usb::controlSuspendItrEn;
			usbState = deviceState_t::powered;
		}

		if (status & vals::usb::itrStatusWakeup)
			wakeup();
		else if (usbSuspended)
			return;

		if (status & vals::usb::itrStatusReset)
		{
			reset();
			usbState = deviceState_t::waiting;
			return;
		}

		if (status & vals::usb::itrStatusSuspend)
			suspend();

		if (usbState == deviceState_t::detached ||
			usbState == deviceState_t::attached ||
			usbState == deviceState_t::powered)
			return;
	}
}
