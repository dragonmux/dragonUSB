// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_CONSTANTS___HXX
#define USB_CONSTANTS___HXX

#include <cstdint>

namespace usb::constants
{
	constexpr static uint8_t configsCount{USB_CONFIG_DESCRIPTORS};
	constexpr static uint8_t interfaceCount{USB_INTERFACES};
	constexpr static uint8_t endpointCount{1 + USB_ENDPOINTS};
	constexpr static uint8_t epBufferSize{USB_BUFFER_SIZE};

	constexpr static uint8_t interfaceDescriptorCount{USB_INTERFACE_DESCRIPTORS};
	constexpr static uint8_t endpointDescriptorCount{USB_ENDPOINT_DESCRIPTORS};
	constexpr static uint8_t stringCount{USB_STRINGS};
} // namespace ubs::constants

#endif /*USB_CONSTANTS___HXX*/
