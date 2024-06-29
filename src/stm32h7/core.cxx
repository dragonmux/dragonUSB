// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/platforms/stm32h7/core.hxx"

/*!
 * USB pinout:
 * PA9 - VBus
 * PA11 - D-
 * PA12 - D+
 */

namespace usb::core
{
	void init() noexcept
	{
		// Ensure the 3.3V for USB is bought up
		pwr.ctrl3 |= vals::pwr::ctrl3USB33RegulatorEnable;
		while (!(pwr.ctrl3 & vals::pwr::ctrl3USB33Ready))
			continue;

		// Now bring up the HSI48
		rcc.ctrl |= vals::rcc::ctrlHSI48Enable;
		while (!(rcc.ctrl & vals::rcc::ctrlHSI48Ready))
			continue;

		// Ensure that CRS-based auto-trim is enabled and uses USB SOF packets as the trim source
		rcc.apb1Enable[1U] |= vals::rcc::apb1Enable2CRS;
		crs.config &= ~vals::crs::configSyncSourceMask;
		crs.config |= vals::crs::configSyncSourceUSBSOF;
		crs.ctrl |= vals::crs::ctrlAutoTrimEnable;
		crs.ctrl |= vals::crs::ctrlErrorCounterEnable;

		// Switch the clocks for the USB controller to the HSI48
		rcc.domain2ClockConfig2 = (rcc.domain2ClockConfig2 & ~vals::rcc::domain2ClockConfig2USBMask) |
			vals::rcc::domain2ClockConfig2USBHSI48;

		// Enable the clocks for the USB and GPIOA peripherals
		rcc.ahb4Enable |= vals::rcc::ahb4EnableGPIOA;
		rcc.ahb1Enable |= vals::rcc::ahb1EnableUSB1HS;

		// Set up the port pins used by the USB controller so they're
		// in the right modes with the USB controller connected through.
		vals::gpio::config<vals::gpio_t::pin11>(gpioA, vals::gpio::mode_t::alternateFunction,
			vals::gpio::resistor_t::none, vals::gpio::outputSpeed_t::speed100MHz);
		vals::gpio::config<vals::gpio_t::pin12>(gpioA, vals::gpio::mode_t::alternateFunction,
			vals::gpio::resistor_t::none, vals::gpio::outputSpeed_t::speed100MHz);
		vals::gpio::config<vals::gpio_t::pin9>(gpioA, vals::gpio::mode_t::input, vals::gpio::resistor_t::none);
		// NB: This is not documented, but AF10 is the USB controller for these pins.
		vals::gpio::altFunction<vals::gpio_t::pin11>(gpioA, 10);
		vals::gpio::altFunction<vals::gpio_t::pin12>(gpioA, 10);

		// Configure the DWC2 for use in the stack and begining enumeration
		rcc.ahb1Enable |= vals::rcc::ahb1EnableUSB1HS;
		usb1HS.globalItrStatus = dwc2::globalItrModeMismatch;

		usb1HS.globalUSBConfig |= dwc2::globalUSBConfigPHYSelUSB1_1;
		// Enable VBus sensing in device mode, data connection detection, and power up the FS PHY
		usb1HS.globalCoreConfig &= ~dwc2::globalCoreConfigPrimaryDetectEnable;
		usb1HS.globalCoreConfig |= dwc2::globalCoreConfigVBusDetectEnable | dwc2::globalCoreConfigDataDetectEnable |
			dwc2::globalCoreConfigIntPHYEnable;

		// Wait for AHB idle
		while (!(usb1HS.globalResetCtrl & dwc2::globalResetCtrlAHBIdle))
			continue;
		// Do a core soft reset
		usb1HS.globalResetCtrl |= dwc2::globalResetCtrlCore;
		while (usb1HS.globalResetCtrl & dwc2::globalResetCtrlCore)
			continue;

		// Force peripheral only mode
		usb1HS.globalUSBConfig |= dwc2::globalUSBConfigForceDeviceMode | dwc2::globalUSBConfigTurnaroundTime(15U);

		// Full speed device

		// Restart the PHY clock

		usb1HS.globalRxFIFOSize = dwc2::rxFIFOSize;
		// Unmask interrupts for TX and RX
		usb1HS.globalAHBConfig = dwc2::globalAHBConfigGlobalIntUnmask;
		usb1HS.globalItrMask = dwc2::globalItrSOF | dwc2::globalItrRxFIFONonEmpty | dwc2::globalItrUSBSuspend |
			dwc2::globalItrUSBReset | dwc2::globalItrEnumDone | dwc2::globalItrInEndpoint |
			dwc2::globalItrOutEndpoint | dwc2::globalItrDisconnected | dwc2::globalItrWakeupDetected;
	}
} // namespace usb::core
