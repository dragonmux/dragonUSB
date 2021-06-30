// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/core.hxx"
#include "usb/device.hxx"

using namespace usb::constants;
using namespace usb::types;

namespace usb::core
{
	namespace internal
	{
		deviceState_t usbState;
		usbEP_t usbPacket;
		bool usbSuspended;
		ctrlState_t usbCtrlState;
		uint8_t usbDeferalFlags;

		std::array<std::array<handler_t, endpointCount - 1U>, configsCount> inHandlers{};
		std::array<std::array<handler_t, endpointCount - 1U>, configsCount> outHandlers{};
	} // namespace internal

	std::array<usbEPStatus_t<const void>, endpointCount> epStatusControllerIn{};
	std::array<usbEPStatus_t<void>, endpointCount> epStatusControllerOut{};
} // namespace usb::core
