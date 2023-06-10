// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_INTERNAL_CORE___HXX
#define USB_INTERNAL_CORE___HXX

#include "usb/core.hxx"

namespace usb::core::internal
{
	using usb::types::handler_t;
	using usb::constants::configsCount;
	using usb::constants::interfaceCount;

	extern usb::types::deviceState_t usbState;
	extern usb::types::usbEP_t usbPacket;
	extern bool usbSuspended;
	extern usb::types::ctrlState_t usbCtrlState;
	extern uint8_t usbDeferalFlags;

	extern std::array<std::array<handler_t, endpointCount - 1U>, configsCount> inHandlers;
	extern std::array<std::array<handler_t, endpointCount - 1U>, configsCount> outHandlers;
	extern std::array<sofHandler_t, interfaceCount> sofHandlers;
} // namespace usb::core::internal

namespace usb::core::common
{
	void resetEPs(epReset_t what) noexcept;
} // namespace usb::core::common

#endif /*USB_INTERNAL_CORE___HXX*/
