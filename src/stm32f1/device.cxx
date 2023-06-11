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
	void handleControlPacket() noexcept
	{
		// If we received a packet..
		if (usbPacket.dir() == endpointDir_t::controllerOut)
		{
			if (usbCtrl.epCtrlStat[0] & vals::usb::epStatusSetup)
				handleSetupPacket();
			else
				handleControllerOutPacket();
		}
		else
			handleControllerInPacket();
	}
} // namespace usb::device
