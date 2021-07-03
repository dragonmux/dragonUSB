// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/internal/core.hxx"
#include "usb/device.hxx"
#include <substrate/indexed_iterator>

using namespace usb::constants;
using namespace usb::types;
using namespace usb::core::internal;

namespace usb::core
{
	namespace internal
	{
		deviceState_t usbState;
		usbEP_t usbPacket;
		bool usbSuspended;
		ctrlState_t usbCtrlState;
		uint8_t usbDeferalFlags;

		std::array<std::array<handler_t, endpointCount - 1U>, configsCount> inHandlers{};
		std::array<std::array<handler_t, endpointCount - 1U>, configsCount> outHandlers{};
		std::array<sofHandler_t, interfaceCount> sofHandlers{};
	} // namespace internal

	std::array<usbEPStatus_t<const void>, endpointCount> epStatusControllerIn{};
	std::array<usbEPStatus_t<void>, endpointCount> epStatusControllerOut{};

	void registerHandler(usbEP_t ep, const uint8_t config, handler_t handler) noexcept
	{
		const auto endpoint{ep.endpoint()};
		const auto direction{ep.dir()};
		if (!endpoint || endpoint >= endpointCount || !config || config > configsCount)
			return;
		if (direction == endpointDir_t::controllerIn)
			inHandlers[config - 1U][endpoint - 1U] = std::move(handler);
		else
			outHandlers[config - 1U][endpoint - 1U] = std::move(handler);
	}

	void unregisterHandler(usbEP_t ep, const uint8_t config) noexcept
	{
		const auto endpoint{ep.endpoint()};
		const auto direction{ep.dir()};
		if (!endpoint || endpoint >= endpointCount || !config || config > configsCount)
			return;
		if (direction == endpointDir_t::controllerIn)
			inHandlers[config - 1U][endpoint - 1U] = {};
		else
			outHandlers[config - 1U][endpoint - 1U] = {};
	}

	void initHandlers() noexcept
	{
		if (!usb::device::activeConfig)
			return; // Nothing to do for the unconfigured configuration (activeConfig == 0)
		const auto config{usb::device::activeConfig - 1U};
		for (const auto &[i, handler] : substrate::indexedIterator_t{inHandlers[config]})
		{
			if (handler.init)
				handler.init(uint8_t(i + 1U)); // i + 1 is the endpoint the handler is registered on
		}
		for (const auto &[i, handler] : substrate::indexedIterator_t{outHandlers[config]})
		{
			if (handler.init)
				handler.init(uint8_t(i + 1U)); // i + 1 is the endpoint the handler is registered on
		}
	}

	void deinitHandlers() noexcept
	{
		if (!usb::device::activeConfig)
			return; // Nothing to do for the unconfigured configuration (activeConfig == 0)
		const auto config{usb::device::activeConfig - 1U};
		for (const auto &[i, handler] : substrate::indexedIterator_t{inHandlers[config]})
		{
			if (handler.deinit)
				handler.deinit(uint8_t(i + 1U)); // i + 1 is the endpoint the handler is registered on
		}
		for (const auto &[i, handler] : substrate::indexedIterator_t{outHandlers[config]})
		{
			if (handler.deinit)
				handler.deinit(uint8_t(i + 1U)); // i + 1 is the endpoint the handler is registered on
		}
	}

	usb::types::handler_t handlerFor(usb::types::usbEP_t ep, uint8_t config) noexcept
	{
		const auto endpoint{ep.endpoint()};
		const auto direction{ep.dir()};
		if (!endpoint || endpoint > endpointCount || !config || config >= configsCount)
			return {};
		if (direction == endpointDir_t::controllerIn)
			return inHandlers[config][endpoint - 1U];
		else
			return outHandlers[config][endpoint - 1U];
	}

	void registerSOFHandler(const uint16_t interface, const sofHandler_t handler) noexcept
	{
		if (interface >= interfaceCount)
			return;
		sofHandlers[interface] = handler;
	}

	void unregsiterSOFHandler(const uint16_t interface) noexcept
	{
		if (interface >= interfaceCount)
			return;
		sofHandlers[interface] = nullptr;
	}
} // namespace usb::core
