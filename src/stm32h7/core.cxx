// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
//#include "usb/platforms/dwc2/core.hxx"

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

		// Ensure that CRS-based auto-trim is enabled

		// Switch the clocks for the USB controller to the HSI48

		// Enable the clocks for the USB and GPIOA peripherals

		// Set up the port pins used by the USB controller so they're
		// in the right modes with the USB controller connected through.

		// Configure the DWC2 for use in the stack and begining enumeration
	}
} // namespace usb::core
