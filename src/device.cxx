// SPDX-License-Identifier: BSD-3-Clause
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
	setupPacket_t packet{};
	uint8_t activeConfig{};
	static std::array<uint8_t, 2> statusResponse{};

	static std::array<std::array<uint8_t, interfaceCount>, configsCount> alternateModes{};

	namespace internal
	{
		std::array<std::array<controlHandler_t, interfaceCount>, configsCount> controlHandlers{};
	}

	static const usbStringDesc_t stringLangIDDescriptor{{u"\x0904", 1}};
	static const auto stringLangIDParts{stringLangIDDescriptor.asParts()};
	static const usbMultiPartTable_t stringLangID{stringLangIDParts.begin(), stringLangIDParts.end()};

	answer_t handleGetDescriptor() noexcept
	{
		using namespace usb::descriptors;
		if (packet.requestType.dir() == endpointDir_t::controllerOut)
			return {response_t::unhandled, nullptr, 0};
		const auto descriptor = packet.value.asDescriptor();

		switch (descriptor.type)
		{
			// Handle device descriptor requests
			case usbDescriptor_t::device:
				return {response_t::data, &deviceDescriptor, deviceDescriptor.length};
			case usbDescriptor_t::deviceQualifier:
				return {response_t::stall, nullptr, 0};
			// Handle configuration descriptor requests
			case usbDescriptor_t::configuration:
			{
				if (descriptor.index >= configsCount)
					break;
				static_assert(sizeof(usbConfigDescriptor_t) == 9);
				static_assert(sizeof(usbInterfaceDescriptor_t) == 9);
				static_assert(sizeof(usbEndpointDescriptor_t) == 7);
				const auto &configDescriptor{configDescriptors[descriptor.index]};
				epStatusControllerIn[0].isMultiPart(true);
				epStatusControllerIn[0].partNumber = 0;
				epStatusControllerIn[0].partsData = &configDescriptor;
				return {response_t::data, nullptr, configDescriptor.totalLength()};
			}
			// Handle interface descriptor requests
			case usbDescriptor_t::interface:
			{
				if (descriptor.index >= interfaceDescriptorCount)
					break;
				const auto &interfaceDescriptor{interfaceDescriptors[descriptor.index]};
				return {response_t::data, &interfaceDescriptor, interfaceDescriptor.length};
			}
			// Handle endpoint descriptor requests
			case usbDescriptor_t::endpoint:
			{
				if (descriptor.index >= endpointDescriptorCount)
					break;
				const auto &endpointDescriptor{endpointDescriptors[descriptor.index]};
				return {response_t::data, &endpointDescriptor, endpointDescriptor.length};
			}
			// Handle string requests
			case usbDescriptor_t::string:
			{
				if (descriptor.index > stringCount)
					break;
				epStatusControllerIn[0].isMultiPart(true);
				epStatusControllerIn[0].partNumber = 0;
				if (descriptor.index)
					epStatusControllerIn[0].partsData = &strings[descriptor.index - 1U];
				else
					epStatusControllerIn[0].partsData = &stringLangID;
				return {response_t::data, nullptr, epStatusControllerIn[0].partsData->totalLength()};
			}
			default:
				break;
		}
		return {response_t::unhandled, nullptr, 0};
	}

	answer_t handleGetStatus() noexcept
	{
		switch (packet.requestType.recipient())
		{
		case setupPacket::recipient_t::device:
			statusResponse[0] = 0; // We are bus-powered and don't support remote wakeup
			statusResponse[1] = 0;
			return {response_t::data, statusResponse.data(), statusResponse.size()};
		case setupPacket::recipient_t::interface:
			// Interface requests are required to answer with all 0's
			statusResponse[0] = 0;
			statusResponse[1] = 0;
			return {response_t::data, statusResponse.data(), statusResponse.size()};
		case setupPacket::recipient_t::endpoint:
			// TODO: Figure out how to signal stalled/halted endpoints.
		default:
			// Bad request? Stall.
			return {response_t::stall, nullptr, 0};
		}
	}

	answer_t handleStandardRequest() noexcept
	{
		//const auto &epStatus{epStatusControllerIn[0]};

		switch (packet.request)
		{
			case request_t::setAddress:
				usbState = deviceState_t::addressing;
				return {response_t::zeroLength, nullptr, 0};
			case request_t::setDescriptor:
				// We do not support setting descriptors.
				return {response_t::stall, nullptr, 0};
			case request_t::getDescriptor:
				return handleGetDescriptor();
			case request_t::setConfiguration:
				if (handleSetConfiguration())
					// Acknowledge the request.
					return {response_t::zeroLength, nullptr, 0};
				// Bad request? Stall.
				return {response_t::stall, nullptr, 0};
			case request_t::getConfiguration:
				return {response_t::data, &activeConfig, 1};
			case request_t::getStatus:
				return handleGetStatus();
			case request_t::getInterface:
				if (packet.index < interfaceCount && packet.length == 1 && !packet.value && activeConfig)
					return {response_t::data, &alternateModes[activeConfig - 1U][packet.index], 1};
				return {response_t::stall, nullptr, 0};
			case request_t::setInterface:
				// If the interface is valid and we're configured
				if (packet.index < interfaceCount && !packet.length && packet.value < 0x0100U && activeConfig)
				{
					alternateModes[activeConfig - 1U][packet.index] = uint8_t(packet.value);
					// Acknowledge the request.
					return {response_t::zeroLength, nullptr, 0};
				}
				// Bad request? Stall.
				return {response_t::stall, nullptr, 0};
			case request_t::syncFrame: // Only used for isochronous stuff anyway.
				return {response_t::stall, nullptr, 0};
		}

		return {response_t::unhandled, nullptr, 0};
	}

	void registerHandler(const uint8_t interface, const uint8_t config, controlHandler_t handler) noexcept
	{
		if (!interface || interface > interfaceCount || !config || config > configsCount)
			return;
		controlHandlers[config - 1U][interface - 1U] = handler;
	}

	void unregisterHandler(const uint8_t interface, const uint8_t config) noexcept
	{
		if (!interface || interface > interfaceCount || !config || config > configsCount)
			return;
		controlHandlers[config - 1U][interface - 1U] = nullptr;
	}
} // namespace usb::device
