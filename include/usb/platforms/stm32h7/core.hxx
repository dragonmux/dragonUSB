// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_PLATFORMS_STM32H7_CORE_HXX
#define USB_PLATFORMS_STM32H7_CORE_HXX

#include "usb/platform.hxx"
#include "usb/dwc2/otg.hxx"

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTBEGIN(performance-no-int-to-ptr
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static auto &usb1HS{*reinterpret_cast<usb::dwc2::otg_t *>(stm32::usb1HSBase)};
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTEND(performance-no-int-to-ptr
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

#endif /*USB_PLATFORMS_STM32H7_CORE_HXX*/
