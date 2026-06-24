#pragma once

#ifndef MGMAKE_BUILD_ARTIFACT_NAMES_HXX
#define MGMAKE_BUILD_ARTIFACT_NAMES_HXX

#include "../detail/enum_string.hxx"
#include "../sys/platform.hxx"

#include <string_view>

namespace mgmake::build {
	using executable_extensions = detail::enum_table<
		sys::platform,
		detail::enum_entry<sys::platform::p_windows, ".exe">,
		detail::enum_entry<sys::platform::p_wasm, ".wasm">
	>;

	static_assert(executable_extensions::has_no_empty_names());
	static_assert(executable_extensions::has_no_duplicate_values());

	[[nodiscard]] inline constexpr std::string_view executable_extension(
		sys::platform platform
	) noexcept {
		return executable_extensions::to_string(platform, "");
	}

	using shared_library_extensions = detail::enum_table<
		sys::platform,
		detail::enum_entry<sys::platform::p_windows, ".dll">,
		detail::enum_entry<sys::platform::p_linux, ".so">,
		detail::enum_entry<sys::platform::p_macos, ".dylib">,
		detail::enum_entry<sys::platform::p_other_posix, ".so">
	>;

	static_assert(shared_library_extensions::has_no_empty_names());
	static_assert(shared_library_extensions::has_no_duplicate_values());

	[[nodiscard]] inline constexpr std::string_view shared_library_extension(
		sys::platform platform
	) noexcept {
		return shared_library_extensions::to_string(platform, "");
	}

	using static_library_prefixes = detail::enum_table<
		sys::platform,
		detail::enum_entry<sys::platform::p_linux, "lib">,
		detail::enum_entry<sys::platform::p_macos, "lib">,
		detail::enum_entry<sys::platform::p_other_posix, "lib">
	>;

	static_assert(static_library_prefixes::has_no_empty_names());
	static_assert(static_library_prefixes::has_no_duplicate_values());

	[[nodiscard]] inline constexpr std::string_view static_library_prefix(
		sys::platform platform
	) noexcept {
		return static_library_prefixes::to_string(platform, "");
	}

	using static_library_extensions = detail::enum_table<
		sys::platform,
		detail::enum_entry<sys::platform::p_windows, ".lib">
	>;

	static_assert(static_library_extensions::has_no_empty_names());
	static_assert(static_library_extensions::has_no_duplicate_values());

	[[nodiscard]] inline constexpr std::string_view static_library_extension(
		sys::platform platform
	) noexcept {
		return static_library_extensions::to_string(platform, ".a");
	}

	using shared_library_link_flags = detail::enum_table<
		sys::platform,
		detail::enum_entry<sys::platform::p_windows, "-shared">,
		detail::enum_entry<sys::platform::p_linux, "-shared">,
		detail::enum_entry<sys::platform::p_macos, "-dynamiclib">,
		detail::enum_entry<sys::platform::p_other_posix, "-shared">
	>;

	static_assert(shared_library_link_flags::has_no_empty_names());
	static_assert(shared_library_link_flags::has_no_duplicate_values());

	[[nodiscard]] inline constexpr std::string_view shared_library_link_flag(
		sys::platform platform
	) noexcept {
		return shared_library_link_flags::to_string(platform, "");
	}

	[[nodiscard]] inline constexpr std::string_view shared_library_prefix(
		sys::platform platform
	) noexcept {
		return (
			platform == sys::platform::p_linux
			|| platform == sys::platform::p_macos
			|| platform == sys::platform::p_other_posix
		) ? "lib" : "";
	}
}

#endif // MGMAKE_BUILD_ARTIFACT_NAMES_HXX
