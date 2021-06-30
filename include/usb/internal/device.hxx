// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_INTERNAL_DEVICE___HXX
#define USB_INTERNAL_DEVICE___HXX

#include <array>
#include "usb/types.hxx"
#include "usb/device.hxx"

namespace usb::device::internal
{
	using usb::constants::configsCount;
	using usb::constants::interfaceCount;
	using usb::types::answer_t;

	extern std::array<std::array<controlHandler_t, interfaceCount>, configsCount> controlHandlers;

	extern bool handleSetConfiguration() noexcept;
	extern void handleControllerInPacket() noexcept;
	extern void handleControllerOutPacket() noexcept;
	extern void handleSetupPacket() noexcept;

	extern bool writeCtrlEP() noexcept;
} // namespace usb::device::internal

#endif /*USB_INTERNAL_DEVICE___HXX*/
