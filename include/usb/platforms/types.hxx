// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_PLATFORMS_TYPES___HXX
#define USB_PLATFORMS_TYPES___HXX

#include <cstdint>

namespace usb::types
{
	enum class endpointDir_t : uint8_t
	{
		controllerOut = 0x00U,
		controllerIn = 0x80U
	};
} // namespace usb::types

namespace usb::descriptors
{
	struct usbMultiPartDesc_t final
	{
		uint8_t length;
		const void *descriptor;
	};
} // namespace usb::descriptors

#if defined(TM4C123GH6PM) || defined(STM32F1)
#include "usb/platforms/aarch32/types.hxx"
#elif defined(ATXMEGA256A3U)
#include "usb/platforms/atxmega256a3u/types.hxx"
#define USB_MEM_SEGMENTED
#endif

#endif /*USB_PLATFORMS_TYPES___HXX*/
