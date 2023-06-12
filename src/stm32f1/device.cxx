// SPDX-License-Identifier: BSD-3-Clause
#include "usb/platform.hxx"
#include "usb/internal/core.hxx"
#include "usb/platforms/stm32f1/core.hxx"
#include "usb/internal/device.hxx"

using namespace usb::constants;
using namespace usb::types;
using namespace usb::core;
using namespace usb::core::internal;
using namespace usb::descriptors;
using namespace usb::device::internal;

namespace usb::device
{
	void setupEndpoint(const usbEndpointDescriptor_t &endpoint, uint16_t &startAddress)
	{
		if (endpoint.endpointType == usbEndpointType_t::control)
			return;

		usb::core::internal::setupEndpoint(endpoint.endpointAddress, endpoint.endpointType, startAddress,
			endpoint.maxPacketSize);
		startAddress += endpoint.maxPacketSize;
	}

	namespace internal
	{
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
			else
			{
				// EP0 consumes the first epBufferSize chunk of USB RAM after the endpoint table.
				uint16_t startAddress{epBufferSize};

				const auto descriptors{configDescriptors[activeConfig - 1U]};
				for (const auto &part : descriptors)
				{
					const auto *const descriptor{static_cast<const std::byte *>(part.descriptor)};
					usbDescriptor_t type{usbDescriptor_t::invalid};
					memcpy(&type, descriptor + 1, 1);
					if (type == usbDescriptor_t::endpoint)
					{
						const auto endpoint{*static_cast<const usbEndpointDescriptor_t *>(part.descriptor)};
						setupEndpoint(endpoint, startAddress);
					}
				}
				usb::core::initHandlers();
			}
			return true;
		}
	} // namespace internal

	void handleControlPacket() noexcept
	{
		// If we received a packet..
		if (usbPacket.dir() == endpointDir_t::controllerOut)
		{
			if (usbCtrl.epCtrlStat[0] & vals::usb::epStatusSetup)
				handleSetupPacket();
			else
				handleControllerOutPacket();
		}
		else
			handleControllerInPacket();
	}
} // namespace usb::device
