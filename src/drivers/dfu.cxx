#include "usb/types.hxx"
#include "usb/core.hxx"
#include "usb/device.hxx"
#include "usb/drivers/dfu.hxx"
#include "usb/drivers/dfuTypes.hxx"

using namespace usb::core;
using namespace usb::device;
using namespace usb::types;
using namespace usb::dfu::types;
using usb::device::packet;

namespace usb::dfu
{
	static config_t config{};
	static std::array<uint8_t, flashPageSize> buffer{};

	static substrate::span<const zone_t> zones{};
	static flashState_t flashState{};

	static_assert(sizeof(config_t) == 6);

	static void tick() noexcept;

	static void init() noexcept
	{
		config.state = dfuState_t::applicationIdle;
		config.status = dfuStatus_t::ok;
	}

	static bool handleSetInterface()
	{
		unregsiterSOFHandler(packet.index);
		if (packet.value >= zones.size())
			return false;

		config.state = dfuState_t::dfuIdle;
		config.status = dfuStatus_t::ok;

		const auto &zone{zones[packet.value]};
		flashState.op = flashOp_t::none;
		flashState.readAddr = zone.start;
		flashState.eraseAddr = zone.start;
		flashState.writeAddr = zone.start;
		flashState.endAddr = zone.end;

		registerSOFHandler(packet.index, tick);
		return true;
	}

	void detached(const bool state) noexcept
	{
		if (state)
			config.state = dfuState_t::dfuIdle;
		else
			config.state = dfuState_t::applicationIdle;
	}

	[[noreturn]] static void detach()
	{
		usb::core::detach();
		config.state = dfuState_t::applicationDetach;
		reboot();
	}

	void downloadStepDone() noexcept { config.state = dfuState_t::downloadBusy; }

	static answer_t handleDownload() noexcept
	{
		if (packet.length)
		{
			if (packet.length > buffer.size() ||
				flashState.eraseAddr + packet.length > flashState.endAddr)
				return {response_t::stall, nullptr, 0};

			flashState.op = flashOp_t::erase;
			flashState.offset = 0;
			flashState.byteCount = packet.length;

			auto &epStatus{epStatusControllerOut[0]};
			epStatus.memBuffer = buffer.data();
			epStatus.transferCount = packet.length;
			epStatus.needsArming(true);
			config.state = dfuState_t::downloadIdle;
			setupCallback = downloadStepDone;
		}
		else
			config.state = dfuState_t::manifestSync;
		return {response_t::zeroLength, nullptr, 0};
	}

	static answer_t handleDFURequest(const std::size_t interface) noexcept
	{
		const auto &requestType{packet.requestType};
		if (requestType.recipient() != setupPacket::recipient_t::interface ||
			requestType.type() != setupPacket::request_t::typeClass ||
			packet.index != interface)
			return {response_t::unhandled, nullptr, 0};

		const auto request{static_cast<types::request_t>(packet.request)};
		switch (request)
		{
			case types::request_t::detach:
				if (packet.requestType.dir() == endpointDir_t::controllerIn)
					return {response_t::stall, nullptr, 0};
				usb::device::setupCallback = detach;
				return {response_t::zeroLength, nullptr, 0};
			case types::request_t::download:
				if (packet.requestType.dir() == endpointDir_t::controllerIn)
					return {response_t::stall, nullptr, 0};
				return handleDownload();
			case types::request_t::upload:
				if (packet.requestType.dir() == endpointDir_t::controllerOut ||
					packet.length > buffer.size() ||
					flashState.readAddr + packet.length > flashState.endAddr)
					return {response_t::stall, nullptr, 0};
				std::memcpy(buffer.data(), reinterpret_cast<void *>(flashState.readAddr), packet.length);
				flashState.readAddr += packet.length;
				return {response_t::data, buffer.data(), packet.length};
			case types::request_t::getStatus:
				if (packet.requestType.dir() == endpointDir_t::controllerOut)
					return {response_t::stall, nullptr, 0};
				if (config.state == dfuState_t::downloadSync)
					config.state = dfuState_t::downloadIdle;
				else if (config.state == dfuState_t::manifestSync)
					config.state = dfuState_t::manifest;
				else if (config.state == dfuState_t::manifest)
					config.state = dfuState_t::dfuIdle;
				return {response_t::data, &config, sizeof(config)};
			case types::request_t::clearStatus:
				if (packet.requestType.dir() == endpointDir_t::controllerIn)
					return {response_t::stall, nullptr, 0};
				if (config.state == dfuState_t::error)
				{
					config.state = dfuState_t::dfuIdle;
					config.status = dfuStatus_t::ok;
				}
				return {response_t::zeroLength, nullptr, 0};
			case types::request_t::getState:
				if (packet.requestType.dir() == endpointDir_t::controllerOut)
					return {response_t::stall, nullptr, 0};
				return {response_t::data, &config.state, sizeof(dfuState_t)};
			case types::request_t::abort:
				if (packet.requestType.dir() == endpointDir_t::controllerIn)
					return {response_t::stall, nullptr, 0};
				config.state = dfuState_t::dfuIdle;
				return {response_t::zeroLength, nullptr, 0};
		}

		return {response_t::stall, nullptr, 0};
	}

	void tick() noexcept
	{
		if (flashState.op == flashOp_t::none || flashBusy())
			return;
		else if (flashState.op == flashOp_t::erase)
		{
			if (flashState.eraseAddr < flashState.writeAddr + flashState.byteCount)
			{
				erase(flashState.eraseAddr);
				flashState.eraseAddr += flashEraseSize;
			}
			else if (flashState.endAddr >= flashState.writeAddr + flashState.byteCount)
				flashState.op = flashOp_t::write;
		}
		if (flashState.op == flashOp_t::write && config.state == dfuState_t::downloadBusy)
		{
			if (flashState.offset == flashState.byteCount)
			{
				flashState.op = flashOp_t::none;
				flashState.offset = 0;
				flashState.byteCount = 0;
				config.state = dfuState_t::downloadSync;
			}
			else
			{
				const auto amount{std::min(flashState.byteCount - flashState.offset, flashBufferSize)};
				write(flashState.writeAddr, amount, buffer.data() + flashState.offset);
				flashState.offset += amount;
				flashState.writeAddr += amount;
			}
		}
	}

	void registerHandlers(const substrate::span<const zone_t> flashZones,
		const uint8_t interface, const uint8_t config) noexcept
	{
		init();
		zones = flashZones;
		usb::device::registerAltModeHandler(interface, config, handleSetInterface);
		usb::device::registerHandler(interface, config, handleDFURequest);
	}
} // namespace usb::dfu
