#pragma once

#ifndef MGMAKE_CLI_BACKEND_HXX
#define MGMAKE_CLI_BACKEND_HXX

#include "../detail/enum_string.hxx"

#include <optional>
#include <string_view>

// Backend names are parsed from CLI text and later mapped to concrete backend types in backend/registry.hxx.

namespace mgmake::cli {
	enum struct backend_kind {
		automatic,
		ninja,
		make,
		direct,

		count
	};

	using backend_kind_names = detail::enum_table<
		backend_kind,
		detail::enum_entry<backend_kind::automatic, "auto">,
		detail::enum_entry<backend_kind::ninja, "ninja">,
		detail::enum_entry<backend_kind::make, "make">,
		detail::enum_entry<backend_kind::direct, "direct">
	>;

	static_assert(
		backend_kind_names::is_zero_based_count_canonical(backend_kind::count),
		"backend_kind_names must cover every backend_kind value exactly once"
	);

	using backend_kind_parse_names = detail::enum_table<
		backend_kind,
		detail::enum_entry<backend_kind::automatic, "auto">,
		detail::enum_entry<backend_kind::automatic, "automatic">,
		detail::enum_entry<backend_kind::ninja, "ninja">,
		detail::enum_entry<backend_kind::make, "make">,
		detail::enum_entry<backend_kind::make, "makefile">,
		detail::enum_entry<backend_kind::make, "makefiles">,
		detail::enum_entry<backend_kind::direct, "direct">,
		detail::enum_entry<backend_kind::direct, "compiler">
	>;

	static_assert(
		backend_kind_parse_names::is_display_aliases(),
		"backend_kind_parse_names must not contain duplicate or empty names"
	);

	[[nodiscard]] inline constexpr std::string_view backend_name(backend_kind backend) noexcept {
		return backend_kind_names::to_string(backend);
	}

	[[nodiscard]] inline constexpr std::optional<backend_kind> backend_from_string(
		std::string_view text
	) noexcept {
		return backend_kind_parse_names::from_string(text);
	}

	[[nodiscard]] inline constexpr bool parse_backend(
		std::string_view text,
		backend_kind& out
	) noexcept {
		const auto parsed = backend_from_string(text);

		if (!parsed.has_value()) {
			return false;
		}

		out = *parsed;
		return true;
	}
}

#endif // MGMAKE_CLI_BACKEND_HXX
