#include "usb/types.hxx"
#include "usb/device.hxx"
#include "usb/drivers/dfu.hxx"
#include "usb/drivers/dfuTypes.hxx"

using namespace usb::types;
using namespace usb::dfu::types;

namespace usb::dfu
{
	config_t config{};
	static_assert(sizeof(config_t) == 6);

	static void init()
	{
		config.state = dfuState_t::applicationIdle;
		config.status = dfuStatus_t::ok;
	}

	static answer_t handleDFURequest(const std::size_t interface, const setupPacket_t &packet) noexcept
	{
		const auto &requestType{packet.requestType};
		if (requestType.recipient() != setupPacket::recipient_t::interface ||
			requestType.type() != setupPacket::request_t::typeClass ||
			packet.index != interface)
			return {response_t::unhandled, nullptr, 0};

		const auto request{static_cast<types::request_t>(packet.request)};
		switch (request)
		{
			case types::request_t::getStatus:
				return {response_t::data, &config, sizeof(config)};
			case types::request_t::clearStatus:
				if (config.state == dfuState_t::error)
				{
					config.state = dfuState_t::dfuIdle;
					config.status = dfuStatus_t::ok;
				}
				return {response_t::zeroLength, nullptr, 0};
			case types::request_t::getState:
				return {response_t::data, &config.state, sizeof(dfuState_t)};
			case types::request_t::abort:
				config.state = dfuState_t::dfuIdle;
				return {response_t::zeroLength, nullptr, 0};
		}

		return {response_t::unhandled, nullptr, 0};
	}

	void registerHandlers(const substrate::span<std::intptr_t> zones,
		const uint8_t interface, const uint8_t config) noexcept
	{
		usb::device::registerHandler(interface, config, handleDFURequest);
	}
} // namespace usb::dfu
