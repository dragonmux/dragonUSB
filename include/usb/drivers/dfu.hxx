#ifndef USB_DRIVERS_DFU___HXX
#define USB_DRIVERS_DFU___HXX

#include <cstdint>
#include <substrate/span>

namespace usb::dfu
{
	struct zone_t final
	{
		std::uintptr_t start;
		std::uintptr_t end;
	};

	extern void registerHandlers(substrate::span<zone_t> flashZones,
		uint8_t interface, uint8_t config) noexcept;
	extern void detached(bool state) noexcept;

	// This must be defined by the user firmware so the DFU driver knows how to reboot the device correctly
	[[noreturn]] extern void reboot() noexcept;
}

#endif /*USB_DRIVERS_DFU___HXX*/
