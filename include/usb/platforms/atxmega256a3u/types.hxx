// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_PLATFORMS_ATXMEGA256A3U_TYPES___HXX
#define USB_PLATFORMS_ATXMEGA256A3U_TYPES___HXX

#include <flash.hxx>
#include "usb/descriptors.hxx"

template<> struct flash_t<usb::descriptors::usbMultiPartDesc_t> final
{
private:
	using T = usb::descriptors::usbMultiPartDesc_t;
	const T *value_;

public:
	constexpr flash_t(const T *const value) noexcept : value_{value} { }
	constexpr flash_t(const flash_t &) noexcept = default;
	constexpr flash_t(flash_t &&) noexcept = default;
	constexpr flash_t &operator =(const flash_t &) noexcept = default;
	constexpr flash_t &operator =(flash_t &&) noexcept = default;
	~flash_t() noexcept = default;

	T operator *() const noexcept
	{
		T result{};
		const auto resultAddr{reinterpret_cast<uint32_t>(&result)};
		const auto valueAddr{reinterpret_cast<uint32_t>(value_)};
		const uint8_t x{RAMPX};
		const uint8_t z{RAMPZ};

		static_assert(sizeof(T) == 3);

		__asm__(R"(
			movw r26, %[result]
			out 0x39, %C[result]
			movw r30, %[value]
			out 0x3B, %C[value]
			elpm r16, Z+
			st X+, r16
			elpm r16, Z+
			st X+, r16
			elpm r16, Z
			st X+, r16
			)" : : [result] "g" (resultAddr), [value] "g" (valueAddr) :
				"r16", "r26", "r27", "r30", "r31"
		);

		RAMPZ = z;
		RAMPX = x;
		return result;
	}

	constexpr std::ptrdiff_t operator -(const flash_t &other) const noexcept
		{ return value_ - other.value_; }

	constexpr flash_t operator +(const size_t offset) const noexcept
		{ return {value_ + offset}; }

	constexpr flash_t &operator ++() noexcept
	{
		++value_;
		return *this;
	}

	T operator[](const size_t offset) const noexcept
		{ return *(*this + offset); }

	constexpr bool operator ==(const flash_t &other) const noexcept
		{ return value_ == other.value_; }
	constexpr bool operator !=(const flash_t &other) const noexcept
		{ return value_ != other.value_; }

	constexpr bool operator >=(const flash_t &other) const noexcept
		{ return value_ >= other.value_; }

	[[nodiscard]] constexpr const T *pointer() const noexcept { return value_; }
};

namespace usb::descriptors
{
	struct usbMultiPartTable_t
	{
	private:
		flash_t<usbMultiPartDesc_t> _begin{nullptr};
		flash_t<usbMultiPartDesc_t> _end{nullptr};

	public:
		constexpr usbMultiPartTable_t() noexcept = default;
		constexpr usbMultiPartTable_t(const usbMultiPartTable_t &) noexcept = default;
		constexpr usbMultiPartTable_t(usbMultiPartTable_t &&) noexcept = default;
		constexpr usbMultiPartTable_t(const usbMultiPartDesc_t *const begin,
			const usbMultiPartDesc_t *const end) noexcept : _begin{begin}, _end{end} { }
		[[nodiscard]] constexpr auto begin() const noexcept { return _begin; }
		[[nodiscard]] constexpr auto end() const noexcept { return _end; }
		[[nodiscard]] constexpr auto count() const noexcept { return _end - _begin; }

		[[nodiscard]] auto part(const std::size_t index) const noexcept
		{
			if (_begin + index >= _end)
				return _end;
			return _begin + index;
		}
		auto operator [](const std::size_t index) const noexcept { return part(index); }

		[[nodiscard]] auto totalLength() const noexcept
		{
			// TODO: Convert to std::accumulate() later.
			std::size_t count{};
			for (const auto &descriptor : *this)
				count += descriptor.length;
			return count;
		}

		constexpr usbMultiPartTable_t &operator =(const usbMultiPartTable_t &) noexcept = default;
		constexpr usbMultiPartTable_t &operator =(usbMultiPartTable_t &&) noexcept = default;
		usbMultiPartTable_t &operator =(const flash_t<usbMultiPartTable_t> &data) noexcept;
	};
} // namespace usb::descriptors

template<> struct flash_t<usb::descriptors::usbMultiPartTable_t> final
{
private:
	using T = usb::descriptors::usbMultiPartTable_t;
	T value_;

public:
	constexpr flash_t(const T value) noexcept : value_{value} { }

	T operator *() const noexcept
	{
		T result{};
		const auto resultAddr{reinterpret_cast<uint32_t>(&result)};
		const auto valueAddr{reinterpret_cast<uint32_t>(&value_)};
		const uint8_t x{RAMPX};
		const uint8_t z{RAMPZ};

		static_assert(sizeof(T) == 4);

		__asm__(R"(
			movw r26, %[result]
			out 0x39, %C[result]
			movw r30, %[value]
			out 0x3B, %C[value]
			elpm r16, Z+
			st X+, r16
			elpm r16, Z+
			st X+, r16
			elpm r16, Z+
			st X+, r16
			elpm r16, Z
			st X+, r16
			)" : : [result] "g" (resultAddr), [value] "g" (valueAddr) :
				"r16", "r26", "r27", "r30", "r31"
		);

		RAMPZ = z;
		RAMPX = x;
		return result;
	}

	[[nodiscard]] auto totalLength() const noexcept
	{
		const auto table{**this};
		return table.totalLength();
	}
};

namespace usb::descriptors
{
	static inline usbMultiPartTable_t &
		usbMultiPartTable_t::operator =(const flash_t<usbMultiPartTable_t> &data) noexcept
	{
		*this = *data;
		return *this;
	}

	inline namespace constants { using namespace usb::constants; }

	extern const std::array<flash_t<usbMultiPartTable_t>, configsCount> configDescriptors;
	extern const std::array<flash_t<usbMultiPartTable_t>, stringCount> strings;
} // namespace usb::descriptors

#endif /*USB_PLATFORMS_ATXMEGA256A3U_TYPES___HXX*/
