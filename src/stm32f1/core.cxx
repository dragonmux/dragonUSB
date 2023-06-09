// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/internal/core.hxx"
#include "usb/device.hxx"

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
	void address(const uint8_t value) noexcept
	{
		usbCtrl.address = (usbCtrl.address & vals::usb::addressClrMask) |
			(value & vals::usb::addressMask);
	}

	uint8_t address() noexcept { return usbCtrl.address & vals::usb::addressMask; }
}
