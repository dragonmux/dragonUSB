// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_CORE___HXX
#define USB_CORE___HXX

#include "usb/constants.hxx"
#include "usb/types.hxx"

namespace usb::core
{
	using usb::constants::endpointCount;

	extern std::array<usb::types::usbEPStatus_t<const void>, endpointCount> epStatusControllerIn;
	extern std::array<usb::types::usbEPStatus_t<void>, endpointCount> epStatusControllerOut;

	enum class epReset_t : uint8_t
	{
		all,
		user
	};

	using sofHandler_t = void (*)();

	extern void init() noexcept;
	extern void handleIRQ() noexcept;
	extern void attach() noexcept;
	extern void detach() noexcept;
	extern void address(uint8_t value) noexcept;
	extern uint8_t address() noexcept;

	extern void resetEPs(epReset_t what) noexcept;

	extern bool readEP(uint8_t endpoint) noexcept;
	extern bool writeEP(uint8_t endpoint) noexcept;

	extern bool readEPReady(uint8_t endpoint) noexcept;
	extern bool writeEPBusy(uint8_t endpoint) noexcept;
	extern void stallEP(uint8_t endpoint) noexcept;
	extern uint16_t readEPDataAvail(uint8_t endpoint) noexcept;
	extern void flushWriteEP(uint8_t endpoint) noexcept;

	extern void registerHandler(usb::types::usbEP_t ep, uint8_t config, usb::types::handler_t handler) noexcept;
	extern void unregisterHandler(usb::types::usbEP_t ep, uint8_t config) noexcept;
	extern void initHandlers() noexcept;
	extern void deinitHandlers() noexcept;
	extern usb::types::handler_t handlerFor(usb::types::usbEP_t ep, uint8_t config) noexcept;

	extern void registerSOFHandler(uint16_t interface, sofHandler_t handler) noexcept;
	extern void unregsiterSOFHandler(uint16_t interface) noexcept;
} // namespace usb::core

#endif /*USB_CORE___HXX*/
