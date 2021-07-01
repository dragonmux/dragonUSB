// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/core.hxx"
#include "usb/device.hxx"
#include "usb/internal/device.hxx"

using namespace usb::constants;
using namespace usb::types;
using namespace usb::core;
using namespace usb::descriptors;
using namespace usb::device::internal;

namespace usb::device
{
	namespace endpoint
	{
		uint8_t mapType(const usbEndpointType_t type)
		{
			switch (type)
			{
				case usbEndpointType_t::isochronous:
					return USB_EP_TYPE_ISOCHRONOUS_gc;
				case usbEndpointType_t::control:
					return USB_EP_TYPE_CONTROL_gc;
				default:
					break;
			}
			return USB_EP_TYPE_BULK_gc;
		}

		uint8_t mapMaxSize(const uint16_t size)
		{
			if (size <= 8)
				return USB_EP_BUFSIZE_8_gc;
			else if (size <= 16)
				return USB_EP_BUFSIZE_16_gc;
			else if (size <= 32)
				return USB_EP_BUFSIZE_32_gc;
			else if (size <= 64)
				return USB_EP_BUFSIZE_64_gc;
			// This should never happen..
			return USB_EP_BUFSIZE_1023_gc;
		}
	} // namespace endpoint

	void setupEndpoint(const usbEndpointDescriptor_t &endpoint)
	{
		if (endpoint.endpointType == usbEndpointType_t::control)
			return;

		const auto direction{static_cast<endpointDir_t>(endpoint.endpointAddress & ~usb::descriptors::endpointDirMask)};
		const auto endpointNumber{uint8_t(endpoint.endpointAddress & usb::descriptors::endpointDirMask)};
		auto &epCtrl
		{
			[direction](endpointCtrl_t &endpoint) -> USB_EP_t &
			{
				if (direction == endpointDir_t::controllerIn)
					return endpoint.controllerIn;
				else
					return endpoint.controllerOut;
			}(endpoints[endpointNumber])
		};

		epCtrl.CNT = 0;
		epCtrl.CTRL = endpoint::mapType(endpoint.endpointType) | endpoint::mapMaxSize(endpoint.maxPacketSize);
	}

	bool handleSetConfiguration() noexcept
	{
		usb::core::resetEPs(epReset_t::user);
		usb::core::deinitHandlers();

		const auto config{packet.value.asConfiguration()};
		if (config > configsCount)
			return false;
		activeConfig = config;

		if (activeConfig == 0)
			usbState = deviceState_t::addressed;
		else if (activeConfig <= configsCount)
		{
			const auto descriptors{*configDescriptors[activeConfig - 1U]};
			for (const auto &part : descriptors)
			{
				flash_t<char *> descriptor{static_cast<const std::byte *>(part.descriptor)};
				usbDescriptor_t type{static_cast<usbDescriptor_t>(descriptor[1])};
				if (type == usbDescriptor_t::endpoint)
				{
					const auto endpoint{*static_cast<const usbEndpointDescriptor_t *>(part.descriptor)};
					setupEndpoint(endpoint);
				}
			}
			usb::core::initHandlers();
		}
		else
			return false;
		return true;
	}
} // namespace usb::device
