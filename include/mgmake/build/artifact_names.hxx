#pragma once

#ifndef MGMAKE_BUILD_ARTIFACT_NAMES_HXX
#define MGMAKE_BUILD_ARTIFACT_NAMES_HXX

#include "../detail/enum_string.hxx"
#include "../sys/platform.hxx"

#include <string_view>

namespace mgmake::build {
	using executable_extensions = detail::enum_table<
		sys::platform,
		detail::enum_entry<sys::platform::windows, ".exe">,
		detail::enum_entry<sys::platform::wasm, ".wasm">
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
		detail::enum_entry<sys::platform::windows, ".dll">,
		detail::enum_entry<sys::platform::linux, ".so">,
		detail::enum_entry<sys::platform::macos, ".dylib">,
		detail::enum_entry<sys::platform::wasm, ".wasm">,
		detail::enum_entry<sys::platform::other_posix, ".so">
	>;

	static_assert(shared_library_extensions::has_no_empty_names());
	static_assert(shared_library_extensions::has_no_duplicate_values());

	[[nodiscard]] inline constexpr std::string_view shared_library_extension(
		sys::platform platform
	) noexcept {
		return shared_library_extensions::to_string(platform, "");
	}

	using shared_library_link_flags = detail::enum_table<
		sys::platform,
		detail::enum_entry<sys::platform::windows, "-shared">,
		detail::enum_entry<sys::platform::linux, "-shared">,
		detail::enum_entry<sys::platform::macos, "-dynamiclib">,
		detail::enum_entry<sys::platform::other_posix, "-shared">
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
			platform == sys::platform::linux
			|| platform == sys::platform::macos
			|| platform == sys::platform::other_posix
		) ? "lib" : "";
	}
}

#endif
