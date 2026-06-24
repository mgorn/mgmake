#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_FAMILY_HXX
#define MGMAKE_DISCOVERY_TOOL_FAMILY_HXX

#include "../detail/enum_string.hxx"

#include <string_view>

namespace mgmake::discovery {
	enum struct tool_family {
		unknown,
		gcc,
		clang,
		apple_clang,
		msvc,
		clang_cl,
		llvm_binutils,
		gnu_binutils,
		msvc_binutils,
		mingw,
		cygwin,
		android_clang,
		emscripten,
		cuda,
		hip,
		count
	};

	enum struct linker_flavor {
		unknown,
		compiler_driver,
		gnu_ld,
		lld,
		lld_link,
		msvc_link,
		apple_ld,
		count
	};

	enum struct object_format {
		unknown,
		coff,
		elf,
		mach_o,
		wasm,
		count
	};

	using tool_family_names = detail::enum_table<
		tool_family,
		detail::enum_entry<tool_family::unknown, "unknown">,
		detail::enum_entry<tool_family::gcc, "gcc">,
		detail::enum_entry<tool_family::clang, "clang">,
		detail::enum_entry<tool_family::apple_clang, "apple-clang">,
		detail::enum_entry<tool_family::msvc, "msvc">,
		detail::enum_entry<tool_family::clang_cl, "clang-cl">,
		detail::enum_entry<tool_family::llvm_binutils, "llvm-binutils">,
		detail::enum_entry<tool_family::gnu_binutils, "gnu-binutils">,
		detail::enum_entry<tool_family::msvc_binutils, "msvc-binutils">,
		detail::enum_entry<tool_family::mingw, "mingw">,
		detail::enum_entry<tool_family::cygwin, "cygwin">,
		detail::enum_entry<tool_family::android_clang, "android-clang">,
		detail::enum_entry<tool_family::emscripten, "emscripten">,
		detail::enum_entry<tool_family::cuda, "cuda">,
		detail::enum_entry<tool_family::hip, "hip">
	>;

	using linker_flavor_names = detail::enum_table<
		linker_flavor,
		detail::enum_entry<linker_flavor::unknown, "unknown">,
		detail::enum_entry<linker_flavor::compiler_driver, "compiler-driver">,
		detail::enum_entry<linker_flavor::gnu_ld, "gnu-ld">,
		detail::enum_entry<linker_flavor::lld, "lld">,
		detail::enum_entry<linker_flavor::lld_link, "lld-link">,
		detail::enum_entry<linker_flavor::msvc_link, "msvc-link">,
		detail::enum_entry<linker_flavor::apple_ld, "apple-ld">
	>;

	using object_format_names = detail::enum_table<
		object_format,
		detail::enum_entry<object_format::unknown, "unknown">,
		detail::enum_entry<object_format::coff, "coff">,
		detail::enum_entry<object_format::elf, "elf">,
		detail::enum_entry<object_format::mach_o, "mach-o">,
		detail::enum_entry<object_format::wasm, "wasm">
	>;

	static_assert(tool_family_names::is_zero_based_count_canonical(tool_family::count));
	static_assert(linker_flavor_names::is_zero_based_count_canonical(linker_flavor::count));
	static_assert(object_format_names::is_zero_based_count_canonical(object_format::count));

	[[nodiscard]] inline constexpr std::string_view name(tool_family family) noexcept {
		return tool_family_names::to_string(family);
	}

	[[nodiscard]] inline constexpr std::string_view name(linker_flavor flavor) noexcept {
		return linker_flavor_names::to_string(flavor);
	}

	[[nodiscard]] inline constexpr std::string_view name(object_format format) noexcept {
		return object_format_names::to_string(format);
	}
}

#endif // MGMAKE_DISCOVERY_TOOL_FAMILY_HXX
