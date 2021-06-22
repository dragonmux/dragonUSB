// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_CORE___HXX
#define USB_CORE___HXX

#include "usb/types.hxx"

namespace usb::core
{
	using usb::types::endpointCount;
	using usb::types::configsCount;

	extern usb::types::deviceState_t usbState;
	extern usb::types::usbEP_t usbPacket;
	extern usb::types::ctrlState_t usbCtrlState;

	extern std::array<usb::types::usbEPStatus_t<const void>, endpointCount> epStatusControllerIn;
	extern std::array<usb::types::usbEPStatus_t<void>, endpointCount> epStatusControllerOut;

	extern std::array<std::array<usb::types::handler_t, endpointCount - 1>, configsCount> inHandlers;
	extern std::array<std::array<usb::types::handler_t, endpointCount - 1>, configsCount> outHandlers;

	extern const uint8_t *sendData(uint8_t ep, const uint8_t *buffer, uint8_t length) noexcept;
	extern uint8_t *recvData(uint8_t ep, uint8_t *buffer, uint8_t length) noexcept;

	enum class epReset_t : uint8_t
	{
		all,
		user
	};

	extern void init() noexcept;
	extern void handleIRQ() noexcept;

	extern void resetEPs(epReset_t what) noexcept;

	extern bool readEP(uint8_t endpoint) noexcept;
	extern bool writeEP(uint8_t endpoint) noexcept;

	extern bool readEPReady(uint8_t endpoint) noexcept;
	extern bool writeEPBusy(uint8_t endpoint) noexcept;
	extern void clearWaitingRXIRQs() noexcept;

	extern void registerHandler(usb::types::usbEP_t ep, uint8_t config,
		const usb::types::handler_t &handler) noexcept;
	extern void unregisterHandler(usb::types::usbEP_t ep, uint8_t config) noexcept;
	extern void initHandlers() noexcept;
	extern void deinitHandlers() noexcept;
	extern usb::types::handler_t handlerFor(usb::types::usbEP_t ep, uint8_t config) noexcept;
} // namespace usb::core

#endif /*USB_CORE___HXX*/
