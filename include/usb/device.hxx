// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_DEVICE_HXX
#define USB_DEVICE_HXX

#include <cstdint>
#include <cstring>
#include <type_traits>
#include "usb/types.hxx"
#include "usb/descriptors.hxx"

namespace usb::types
{
	enum class request_t : uint8_t
	{
		getStatus = 0,
		clearFeature = 1,
		setFeature = 3,
		setAddress = 5,
		getDescriptor = 6,
		setDescriptor = 7,
		getConfiguration = 8,
		setConfiguration = 9,
		getInterface = 10,
		setInterface = 11,
		syncFrame = 12
	};

	namespace setupPacket
	{
		using usb::descriptors::usbDescriptor_t;

		enum class request_t : uint8_t
		{
			typeStandard = 0x00,
			typeClass = 0x20U,
			typeVendor = 0x40U
		};

		enum class recipient_t : uint8_t
		{
			device = 0,
			interface = 1,
			endpoint = 2,
			other = 3
		};

		enum class reportType_t : uint8_t
		{
			input = 1,
			output = 2,
			feature = 3
		};

		struct requestType_t final
		{
		private:
			uint8_t value{};

		public:
			requestType_t() = default;

			[[nodiscard]] recipient_t recipient() const noexcept
				{ return static_cast<recipient_t>(value & 0x1FU); }
			[[nodiscard]] request_t type() const noexcept
				{ return static_cast<request_t>(value & 0x60U); }
			[[nodiscard]] endpointDir_t dir() const noexcept
				{ return static_cast<endpointDir_t>(value & 0x80U); }
		};

		struct descriptor_t final
		{
			uint8_t index;
			usbDescriptor_t type;
		};

		struct address_t final
		{
			uint8_t addrL;
			uint8_t addrH;
		};

		struct report_t final
		{
			uint8_t index;
			reportType_t type;
		};

		struct value_t final
		{
		private:
			uint16_t value{};

		public:
			value_t() = default;
			[[nodiscard]] operator uint16_t() const noexcept { return value; }

			[[nodiscard]] descriptor_t asDescriptor() const noexcept
			{
				descriptor_t result{};
				std::memcpy(&result, &value, sizeof(value));
				return result;
			}

			[[nodiscard]] address_t asAddress() const noexcept
			{
				address_t result{};
				std::memcpy(&result, &value, sizeof(value));
				return result;
			}

			[[nodiscard]] uint8_t asConfiguration() const noexcept
			{
				uint8_t result{};
				static_assert(sizeof(uint8_t) < sizeof(uint16_t));
				static_assert(sizeof(uint8_t) == 1);
				std::memcpy(&result, &value, sizeof(result));
				return result;
			}

			[[nodiscard]] report_t asReport() const noexcept
			{
				report_t result{};
				std::memcpy(&result, &value, sizeof(value));
				return result;
			}
		};

		struct index_t final
		{
		private:
			uint8_t valueL{};
			uint8_t valueH{};

		public:
			index_t() = default;
			[[nodiscard]] operator uint16_t() const noexcept { return uint16_t((valueH << 8U) | valueL); }

			[[nodiscard]] endpointDir_t dir() const noexcept { return static_cast<endpointDir_t>(valueL & 0x80U); }
			[[nodiscard]] uint8_t endpoint() const noexcept { return valueL & 0x0FU; }
		};
	} // namespace setupPacket

	struct setupPacket_t final
	{
		setupPacket::requestType_t requestType{};
		request_t request{};
		setupPacket::value_t value{};
		setupPacket::index_t index{};
		uint16_t length{};

		setupPacket_t() = default;
	};
} // namespace usb::types

namespace usb::device
{
	using callback_t = void (*)();

	extern usb::types::setupPacket_t packet;
	extern uint8_t activeConfig;
	extern callback_t setupCallback;

	extern void handleControlPacket() noexcept;

	using controlHandler_t = usb::types::answer_t (*)(std::size_t interface);
	using altModeHandler_t = bool (*)();

	extern void registerHandler(uint8_t interface, uint8_t config, controlHandler_t handler) noexcept;
	extern void unregisterHandler(uint8_t interface, uint8_t config) noexcept;
	extern void registerAltModeHandler(uint8_t interface, uint8_t config, altModeHandler_t handler) noexcept;
	extern void unregisterAltModeHandler(uint8_t interface, uint8_t config) noexcept;
} // namespace usb::device

#endif /*USB_DEVICE_HXX*/
