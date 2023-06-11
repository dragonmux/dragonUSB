// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_PLATFORMS_ATXMEGA256A3U_CORE_HXX
#define USB_PLATFORMS_ATXMEGA256A3U_CORE_HXX

#include <avr/io.h>
#include "usb/core.hxx"

namespace usb::core::internal
{
	struct endpointCtrl_t final
	{
		static_assert(sizeof(::USB_EP_t) == 8);

		::USB_EP_t controllerOut;
		::USB_EP_t controllerIn;
	};
	static_assert(sizeof(endpointCtrl_t) == sizeof(::USB_EP_t) * 2U);

	extern std::array<endpointCtrl_t, endpointCount> endpoints;
} // namespace usb::core::internal

#endif /*USB_PLATFORMS_ATXMEGA256A3U_CORE_HXX*/
