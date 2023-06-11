// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_PLATFORMS_STM32F1_CORE_HXX
#define USB_PLATFORMS_STM32F1_CORE_HXX

#include "usb/descriptors.hxx"

namespace usb::core::internal
{
	using usb::descriptors::usbEndpointType_t;

	void setupEndpoint(uint8_t endpoint, usbEndpointType_t type, uint16_t bufferAddress, uint16_t bufferLength) noexcept;
} // namespace usb::core::internal

#endif /*USB_PLATFORMS_STM32F1_CORE_HXX*/
