#ifndef USB_DRIVERS_DFU___HXX
#define USB_DRIVERS_DFU___HXX

#include <cstdint>
#include <substrate/span>

namespace usb::dfu
{
	extern void registerHandlers(substrate::span<std::intptr_t> zones,
		uint8_t interface, uint8_t config) noexcept;
}

#endif /*USB_DRIVERS_DFU___HXX*/
