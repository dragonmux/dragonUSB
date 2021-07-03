// SPDX-License-Identifier: BSD-3-Clause
#include "usb/internal/core.hxx"
#include "usb/internal/device.hxx"
#include <substrate/indexed_iterator>

using namespace usb::constants;
using namespace usb::types;
using namespace usb::core;
using namespace usb::core::internal;
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

	static const usbStringLangDesc_t stringLangIDDescriptor{u'\x0904'};

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
				return {response_t::data, &deviceDescriptor, sizeof(usbDeviceDescriptor_t), memory_t::flash};
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
				epStatusControllerIn[0].partsData = configDescriptor;
				return {response_t::data, nullptr, configDescriptor.totalLength(), memory_t::flash};
			}
			// Handle interface descriptor requests
			case usbDescriptor_t::interface:
			{
				if (descriptor.index >= interfaceDescriptorCount)
					break;
				const auto &interfaceDescriptor{interfaceDescriptors[descriptor.index]};
				return {response_t::data, &interfaceDescriptor, sizeof(usbInterfaceDescriptor_t), memory_t::flash};
			}
			// Handle endpoint descriptor requests
			case usbDescriptor_t::endpoint:
			{
				if (descriptor.index >= endpointDescriptorCount)
					break;
				const auto &endpointDescriptor{endpointDescriptors[descriptor.index]};
				return {response_t::data, &endpointDescriptor, sizeof(usbEndpointDescriptor_t), memory_t::flash};
			}
			// Handle string requests
			case usbDescriptor_t::string:
			{
				if (descriptor.index > stringCount)
					break;
				else if (descriptor.index == 0)
					return {response_t::data, &stringLangIDDescriptor, sizeof(usbStringLangDesc_t), memory_t::flash};
				const auto &string{strings[descriptor.index - 1U]};
				epStatusControllerIn[0].isMultiPart(true);
				epStatusControllerIn[0].partNumber = 0;
				epStatusControllerIn[0].partsData = string;
				return {response_t::data, nullptr, string.totalLength(), memory_t::flash};
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

	void completeSetupPacket() noexcept
	{
		// If we have no response
		if (!epStatusControllerIn[0].needsArming())
		{
			// But rather need more data
			if (epStatusControllerOut[0].needsArming())
			{
				// <SETUP[0]><OUT[1]><OUT[0]>...<IN[1]>
				usbCtrlState = ctrlState_t::dataRX;
			}
			// We need to stall in answer
			else if (epStatusControllerIn[0].stall())
			{
				// <SETUP[0]><STALL>
				usb::core::stallEP(0);
				usbCtrlState = ctrlState_t::idle;
			}
		}
		// We have a valid response
		else
		{
			// Is this as part of a multi-part transaction?
			if (packet.requestType.dir() == endpointDir_t::controllerIn)
				// <SETUP[0]><IN[1]><IN[0]>...<OUT[1]>
				usbCtrlState = ctrlState_t::dataTX;
			// Or just a quick answer?
			else
				//  <SETUP[0]><IN[1]>
				usbCtrlState = ctrlState_t::statusTX;
			if (writeEP(0))
			{
				if (usbCtrlState == ctrlState_t::dataTX)
					usbCtrlState = ctrlState_t::statusRX;
				else
					usbCtrlState = ctrlState_t::idle;
			}
		}
	}

	namespace internal
	{
	void handleSetupPacket() noexcept
	{
		// Read in the new setup packet
		static_assert(sizeof(setupPacket_t) == 8); // Setup packets must be 8 bytes.
		epStatusControllerOut[0].memBuffer = &packet;
		epStatusControllerOut[0].transferCount = sizeof(setupPacket_t);
		if (!readEP(0))
		{
			// Truncated transfer.. WTF.
			usb::core::stallEP(0);
			return;
		}

		// Set up EP0 state for a reply of some kind
		//usbDeferalFlags = 0;
		usbCtrlState = ctrlState_t::wait;
		epStatusControllerIn[0].needsArming(false);
		epStatusControllerIn[0].stall(false);
		epStatusControllerIn[0].transferCount = 0;
		epStatusControllerOut[0].needsArming(false);
		epStatusControllerOut[0].stall(false);
		epStatusControllerOut[0].transferCount = 0;

		response_t response{response_t::unhandled};
		const void *data{nullptr};
		std::uint16_t size{0};
		memory_t memoryType{memory_t::sram};

		std::tie(response, data, size, memoryType) = handleStandardRequest();
		if (response == response_t::unhandled && activeConfig)
		{
			for (const auto &[i, handler] : substrate::indexedIterator_t{controlHandlers[activeConfig - 1U]})
			{
				if (handler)
					std::tie(response, data, size, memoryType) = handler(i + 1U, packet);
			}
		}

		epStatusControllerIn[0].stall(response == response_t::stall || response == response_t::unhandled);
		epStatusControllerIn[0].needsArming(response == response_t::data || response == response_t::zeroLength);
		epStatusControllerIn[0].memBuffer = data;
		epStatusControllerIn[0].memoryType(memoryType);
		const auto transferCount{response == response_t::zeroLength ? uint16_t(0U) : size};
		epStatusControllerIn[0].transferCount = std::min(transferCount, packet.length);
		// If the response is whacko, don't do the stupid thing
		if (response == response_t::data && !data && !epStatusControllerIn[0].isMultiPart())
			epStatusControllerIn[0].needsArming(false);
		completeSetupPacket();
	}

	void handleControllerOutPacket() noexcept
	{
		// If we're in the data phase
		if (usbCtrlState == ctrlState_t::dataRX)
		{
			if (readEP(0))
			{
				// If we now have all the data for the transaction..
				usbCtrlState = ctrlState_t::statusTX;
				// TODO: Handle data and generate status response.
			}
		}
		// If we're in the status phase
		else
			usbCtrlState = ctrlState_t::idle;
	}

	void handleControllerInPacket() noexcept
	{
		if (usbState == deviceState_t::addressing)
		{
			// We just handled an addressing request, and prepared our answer. Before we get a chance
			// to return from the interrupt that caused this chain of events, lets set the device address.
			const auto address{packet.value.asAddress()};

			// Check that the last setup packet was actually a set address request
			if (packet.requestType.type() != setupPacket::request_t::typeStandard ||
				packet.request != request_t::setAddress || address.addrH != 0)
			{
				usb::core::address(0);
				usbState = deviceState_t::waiting;
			}
			else
			{
				usb::core::address(address.addrL);
				usbState = deviceState_t::addressed;
			}
		}

		// If we're in the data phase
		if (usbCtrlState == ctrlState_t::dataTX)
		{
			if (writeEP(0))
			{
				// If we now have all the data for the transaction..
				//usbCtrlState = ctrlState_t::statusRX;
				usbCtrlState = ctrlState_t::idle;
			}
		}
		// Otherwise this was a status phase TX-complete interrupt
		else
			usbCtrlState = ctrlState_t::idle;
	}
	} // namespace internal
} // namespace usb::device
