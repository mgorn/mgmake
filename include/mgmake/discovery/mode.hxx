#pragma once

#ifndef MGMAKE_DISCOVERY_MODE_HXX
#define MGMAKE_DISCOVERY_MODE_HXX

#include "../detail/enum_string.hxx"

#include <string_view>

namespace mgmake::discovery {
	enum struct mode {
		automatic,
		exact,
		family_fallback,
		auto_fallback,
		disabled,
		count
	};

	using mode_names = detail::enum_table<
		mode,
		detail::enum_entry<mode::automatic, "automatic">,
		detail::enum_entry<mode::exact, "exact">,
		detail::enum_entry<mode::family_fallback, "family-fallback">,
		detail::enum_entry<mode::auto_fallback, "auto-fallback">,
		detail::enum_entry<mode::disabled, "disabled">
	>;

	using mode_parse_names = detail::enum_table<
		mode,
		detail::enum_entry<mode::automatic, "automatic">,
		detail::enum_entry<mode::automatic, "default">,
		detail::enum_entry<mode::exact, "exact">,
		detail::enum_entry<mode::family_fallback, "family-fallback">,
		detail::enum_entry<mode::family_fallback, "family">,
		detail::enum_entry<mode::auto_fallback, "auto-fallback">,
		detail::enum_entry<mode::auto_fallback, "auto">,
		detail::enum_entry<mode::disabled, "disabled">,
		detail::enum_entry<mode::disabled, "off">
	>;

	static_assert(mode_names::is_zero_based_count_canonical(mode::count));
	static_assert(mode_parse_names::is_display_aliases());

	[[nodiscard]] inline constexpr std::string_view name(mode value) noexcept {
		return mode_names::to_string(value);
	}
}

#endif
