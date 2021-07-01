// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_TYPES___HXX
#define USB_TYPES___HXX

#include <cstdint>
#include <tuple>
#include "usb/platform.hxx"
#include "usb/platforms/types.hxx"

namespace usb::types
{
	struct handler_t final
	{
		void (*init)(uint8_t endpoint);
		void (*deinit)(uint8_t endpoint);
		void (*handlePacket)(uint8_t endpoint);
	};

	enum class ctrlState_t
	{
		idle,
		wait,
		dataTX,
		dataRX,
		statusTX,
		statusRX
	};

	enum class deviceState_t
	{
		detached,
		attached,
		powered,
		waiting,
		addressing,
		addressed,
		configured
	};

	enum class response_t
	{
		data,
		zeroLength,
		unhandled,
		stall
	};

	struct usbEP_t final
	{
	private:
		uint8_t value{};

	public:
		usbEP_t() = default;
		usbEP_t(const uint8_t num, const endpointDir_t dir) : value(uint8_t(dir) | num) { }

		void endpoint(const uint8_t num) noexcept
		{
			value &= 0xF0U;
			value |= uint8_t(num & 0x0FU);
		}

		[[nodiscard]] uint8_t endpoint() const noexcept { return value & 0x0FU; }

		void dir(const endpointDir_t dir) noexcept
		{
			value &= 0x7FU;
			value |= uint8_t(dir);
		}

		[[nodiscard]] endpointDir_t dir() const noexcept { return static_cast<endpointDir_t>(value & 0x80U); }
	};

	enum class memory_t
	{
		sram,
		flash
	};

	template<typename buffer_t> struct usbEPStatus_t final
	{
	private:
		uint8_t value{};

	public:
		buffer_t *memBuffer{nullptr};
		usbEP_t ctrl{};
		uint16_t transferCount{};
		// Multi-part fields
		uint8_t partNumber{};
		usb::descriptors::usbMultiPartTable_t partsData{};

		usbEPStatus_t() = default;

		void transferTerminated(const bool terminated) noexcept
		{
			value &= 0xFEU;
			value |= terminated ? 0x01U : 0x00U;
		}

		[[nodiscard]] bool transferTerminated() const noexcept { return value & 0x01U; }

		void needsArming(const bool needed) noexcept
		{
			value &= 0xFDU;
			value |= uint8_t(needed ? 0x02U : 0x00U);
		}

		[[nodiscard]] bool needsArming() const noexcept { return value & 0x02U; }

		void stall(const bool needed) noexcept
		{
			value &= 0xFBU;
			value |= uint8_t(needed ? 0x04U : 0x00U);
		}

		[[nodiscard]] bool stall() const noexcept { return value & 0x04U; }

		void isMultiPart(const bool multiPart) noexcept
		{
			value &= 0xF7U;
			value |= uint8_t(multiPart ? 0x08U : 0x00U);
		}

		[[nodiscard]] bool isMultiPart() const noexcept { return value & 0x08U; }

		void memoryType(memory_t type) noexcept
		{
#ifdef USB_MEM_SEGMENTED
			value &= 0xEFU;
			value |= uint8_t(type == memory_t::flash ? 0x10U : 0x00U);
#endif
		}

		memory_t memoryType() const noexcept { return (value & 0x10U) ? memory_t::flash : memory_t::sram; }
		void resetStatus() noexcept { value = 0; }
	};

	struct answer_t : public std::tuple<response_t, const void *, std::uint16_t, memory_t>
	{
		using tuple_t = std::tuple<response_t, const void *, std::uint16_t, memory_t>;
		answer_t(response_t response, const void *data, std::uint16_t length) :
			tuple_t{response, data, length, memory_t::sram} { }
		answer_t(response_t response, const void *data, std::uint16_t length, memory_t memoryType) :
			tuple_t{response, data, length, memoryType} { }
	};
} // namespace usb::types

#endif /*USB_TYPES___HXX*/
