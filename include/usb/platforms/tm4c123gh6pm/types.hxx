// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_PLATFORMS_TM4C123GH6PM_TYPES___HXX
#define USB_PLATFORMS_TM4C123GH6PM_TYPES___HXX

#include "usb/descriptors.hxx"

namespace usb::descriptors
{
	struct usbMultiPartTable_t
	{
	private:
		const usbMultiPartDesc_t *_begin{nullptr};
		const usbMultiPartDesc_t *_end{nullptr};

	public:
		constexpr usbMultiPartTable_t() noexcept = default;
		constexpr usbMultiPartTable_t(const usbMultiPartDesc_t *const begin,
			const usbMultiPartDesc_t *const end) noexcept : _begin{begin}, _end{end} { }
		[[nodiscard]] constexpr auto begin() const noexcept { return _begin; }
		[[nodiscard]] constexpr auto end() const noexcept { return _end; }
		[[nodiscard]] constexpr auto count() const noexcept { return _end - _begin; }

		[[nodiscard]] constexpr auto &part(const std::size_t index) const noexcept
		{
			if (_begin + index >= _end)
				return *_end;
			return _begin[index];
		}
		constexpr auto &operator [](const std::size_t index) const noexcept { return part(index); }

		[[nodiscard]] constexpr auto totalLength() const noexcept
		{
			// TODO: Convert to std::accumulate() later.
			std::size_t count{};
			for (const auto &descriptor : *this)
				count += descriptor.length;
			return count;
		}

		constexpr usbMultiPartTable_t &operator =(const usbMultiPartTable_t &) noexcept = default;
	};

	inline namespace constants { using namespace usb::constants; }

	extern const std::array<usbMultiPartTable_t, configsCount> configDescriptors;
	extern const std::array<usbMultiPartTable_t, stringCount> strings;
} // namespace usb::descriptors

#endif /*USB_PLATFORMS_TM4C123GH6PM_TYPES___HXX*/
