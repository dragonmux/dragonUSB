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

	constexpr static std::size_t flashPageSize{USB_DFU_FLASH_PAGE_SIZE};
	constexpr static std::size_t flashBufferSize{USB_DFU_FLASH_PAGE_SIZE};

	extern void registerHandlers(substrate::span<zone_t> flashZones,
		uint8_t interface, uint8_t config) noexcept;
	extern void detached(bool state) noexcept;

	// These must be defined by the user firmware.
	// Called by the DFU driver to reboot the device correctly
	[[noreturn]] extern void reboot() noexcept;
	// Called by the DFU driver to erase Flash space ready for writing data
	extern void erase(std::uintptr_t address) noexcept;
	// Called by the DFU driver to perform the Flash write
	extern void write(std::uintptr_t address, std::size_t count, const uint8_t *buffer) noexcept;
	// Tells the driver if Flash is currently busy as a result of one of the above.
	extern bool flashBusy() noexcept;
} // namespace usb::dfu

#endif /*USB_DRIVERS_DFU___HXX*/
