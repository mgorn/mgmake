#pragma once

#ifndef MGMAKE_SYS_PLATFORM_HXX
#define MGMAKE_SYS_PLATFORM_HXX

#include "../detail/enum_string.hxx"

#include <optional>
#include <string>
#include <string_view>

#if defined(_WIN32)
	#ifndef MGMK_NO_WINDOWS
		#ifndef NOMINMAX
			#define NOMINMAX
		#endif

		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN
		#endif

		#include <windows.h>
		#pragma message("Windows is included here. This is probably the source of your pain.")
		#define MGMK_INCLUDED_WINDOWS
	#endif

	#define MGMK_PLATFORM_WINDOWS 1
#elif defined(__EMSCRIPTEN__)
	#define MGMK_PLATFORM_WASM 1
	#define MGMK_PLATFORM_POSIX 1
#elif defined(__APPLE__)
	#define MGMK_PLATFORM_MACOS 1
	#define MGMK_PLATFORM_POSIX 1
#elif defined(__linux__)
	#define MGMK_PLATFORM_LINUX 1
	#define MGMK_PLATFORM_POSIX 1
#elif defined(__unix__)
	#define MGMK_PLATFORM_OTHER_POSIX 1
	#define MGMK_PLATFORM_POSIX 1
#else
	#define MGMK_PLATFORM_UNSUPPORTED 1
#endif

namespace mgmake::sys {
	enum struct arch {
		unknown,

		x86,
		x86_64,

		arm,
		aarch64,

		wasm32,
		wasm64,

		riscv32,
		riscv64,

		count
	};

	enum struct platform {
		unknown,

		windows,
		linux,
		macos,
		wasm,
		freestanding,
		other_posix,

		count
	};

	enum struct abi {
		unknown,

		msvc,
		gnu,
		musl,
		android,
		eabi,
		none,

		count
	};

	struct target {
		arch m_arch = arch::unknown;
		platform m_platform = platform::unknown;
		abi m_abi = abi::unknown;
		std::string m_triple{};
	};

	static constexpr arch g_host_arch = [] constexpr {
#if defined(_M_X64) || defined(__x86_64__)
		return arch::x86_64;
#elif defined(_M_IX86) || defined(__i386__)
		return arch::x86;
#elif defined(_M_ARM64) || defined(__aarch64__)
		return arch::aarch64;
#elif defined(_M_ARM) || defined(__arm__)
		return arch::arm;
#elif defined(__wasm64__)
		return arch::wasm64;
#elif defined(__wasm32__) || defined(__EMSCRIPTEN__)
		return arch::wasm32;
#elif defined(__riscv) && (__riscv_xlen == 64)
		return arch::riscv64;
#elif defined(__riscv) && (__riscv_xlen == 32)
		return arch::riscv32;
#else
		return arch::unknown;
#endif
	}();

	static constexpr platform g_host_platform = [] constexpr {
#if defined(MGMK_PLATFORM_WINDOWS)
		return platform::windows;
#elif defined(MGMK_PLATFORM_WASM)
		return platform::wasm;
#elif defined(MGMK_PLATFORM_MACOS)
		return platform::macos;
#elif defined(MGMK_PLATFORM_LINUX)
		return platform::linux;
#elif defined(MGMK_PLATFORM_OTHER_POSIX)
		return platform::other_posix;
#else
		return platform::unknown;
#endif
	}();

	static constexpr abi g_host_abi = [] constexpr {
#if defined(_MSC_VER)
		return abi::msvc;
#elif defined(__ANDROID__)
		return abi::android;
#elif defined(__MUSL__)
		return abi::musl;
#elif defined(__GNUC__) || defined(__clang__)
		return abi::gnu;
#else
		return abi::unknown;
#endif
	}();

	inline const target g_host_target{
		g_host_arch,
		g_host_platform,
		g_host_abi,
		{}
	};

	using arch_names = detail::enum_table<
		arch,
		detail::enum_entry<arch::unknown, "unknown">,
		detail::enum_entry<arch::x86, "x86">,
		detail::enum_entry<arch::x86_64, "x86_64">,
		detail::enum_entry<arch::arm, "arm">,
		detail::enum_entry<arch::aarch64, "aarch64">,
		detail::enum_entry<arch::wasm32, "wasm32">,
		detail::enum_entry<arch::wasm64, "wasm64">,
		detail::enum_entry<arch::riscv32, "riscv32">,
		detail::enum_entry<arch::riscv64, "riscv64">
	>;

	using platform_names = detail::enum_table<
		platform,
		detail::enum_entry<platform::unknown, "unknown">,
		detail::enum_entry<platform::windows, "windows">,
		detail::enum_entry<platform::linux, "linux">,
		detail::enum_entry<platform::macos, "macos">,
		detail::enum_entry<platform::wasm, "wasm">,
		detail::enum_entry<platform::freestanding, "freestanding">,
		detail::enum_entry<platform::other_posix, "other-posix">
	>;

	using abi_names = detail::enum_table<
		abi,
		detail::enum_entry<abi::unknown, "unknown">,
		detail::enum_entry<abi::msvc, "msvc">,
		detail::enum_entry<abi::gnu, "gnu">,
		detail::enum_entry<abi::musl, "musl">,
		detail::enum_entry<abi::android, "android">,
		detail::enum_entry<abi::eabi, "eabi">,
		detail::enum_entry<abi::none, "none">
	>;

	static_assert(arch_names::is_zero_based_count_canonical(arch::count));
	static_assert(platform_names::is_zero_based_count_canonical(platform::count));
	static_assert(abi_names::is_zero_based_count_canonical(abi::count));

	[[nodiscard]] inline constexpr std::string_view name(arch value) noexcept {
		return arch_names::to_string(value);
	}

	[[nodiscard]] inline constexpr std::string_view name(platform value) noexcept {
		return platform_names::to_string(value);
	}

	[[nodiscard]] inline constexpr std::string_view name(abi value) noexcept {
		return abi_names::to_string(value);
	}

	using arch_parse_names = detail::enum_table<
		arch,
		detail::enum_entry<g_host_arch, "host">,
		detail::enum_entry<g_host_arch, "native">,
		detail::enum_entry<arch::unknown, "unknown">,
		detail::enum_entry<arch::x86, "x86">,
		detail::enum_entry<arch::x86, "i386">,
		detail::enum_entry<arch::x86, "i686">,
		detail::enum_entry<arch::x86_64, "x86_64">,
		detail::enum_entry<arch::x86_64, "x64">,
		detail::enum_entry<arch::x86_64, "amd64">,
		detail::enum_entry<arch::arm, "arm">,
		detail::enum_entry<arch::aarch64, "aarch64">,
		detail::enum_entry<arch::aarch64, "arm64">,
		detail::enum_entry<arch::wasm32, "wasm32">,
		detail::enum_entry<arch::wasm64, "wasm64">,
		detail::enum_entry<arch::riscv32, "riscv32">,
		detail::enum_entry<arch::riscv64, "riscv64">
	>;

	using platform_parse_names = detail::enum_table<
		platform,
		detail::enum_entry<g_host_platform, "host">,
		detail::enum_entry<g_host_platform, "native">,
		detail::enum_entry<platform::unknown, "unknown">,
		detail::enum_entry<platform::windows, "windows">,
		detail::enum_entry<platform::windows, "win">,
		detail::enum_entry<platform::windows, "win32">,
		detail::enum_entry<platform::windows, "win64">,
		detail::enum_entry<platform::linux, "linux">,
		detail::enum_entry<platform::macos, "macos">,
		detail::enum_entry<platform::macos, "mac">,
		detail::enum_entry<platform::macos, "darwin">,
		detail::enum_entry<platform::macos, "osx">,
		detail::enum_entry<platform::wasm, "wasm">,
		detail::enum_entry<platform::wasm, "webassembly">,
		detail::enum_entry<platform::wasm, "emscripten">,
		detail::enum_entry<platform::freestanding, "freestanding">,
		detail::enum_entry<platform::freestanding, "none">,
		detail::enum_entry<platform::other_posix, "posix">,
		detail::enum_entry<platform::other_posix, "unix">,
		detail::enum_entry<platform::other_posix, "other-posix">
	>;

	using abi_parse_names = detail::enum_table<
		abi,
		detail::enum_entry<g_host_abi, "host">,
		detail::enum_entry<g_host_abi, "native">,
		detail::enum_entry<abi::unknown, "unknown">,
		detail::enum_entry<abi::msvc, "msvc">,
		detail::enum_entry<abi::gnu, "gnu">,
		detail::enum_entry<abi::musl, "musl">,
		detail::enum_entry<abi::android, "android">,
		detail::enum_entry<abi::eabi, "eabi">,
		detail::enum_entry<abi::none, "none">
	>;

	static_assert(arch_parse_names::is_display_aliases());
	static_assert(platform_parse_names::is_display_aliases());
	static_assert(abi_parse_names::is_display_aliases());

	[[nodiscard]] inline constexpr std::optional<arch> arch_from_string(
		std::string_view text
	) noexcept {
		return arch_parse_names::from_string(text);
	}

	[[nodiscard]] inline constexpr std::optional<platform> platform_from_string(
		std::string_view text
	) noexcept {
		return platform_parse_names::from_string(text);
	}

	[[nodiscard]] inline constexpr std::optional<abi> abi_from_string(
		std::string_view text
	) noexcept {
		return abi_parse_names::from_string(text);
	}

	using arch_triple_names = detail::enum_table<
		arch,
		detail::enum_entry<arch::unknown, "unknown">,
		detail::enum_entry<arch::x86, "i686">,
		detail::enum_entry<arch::x86_64, "x86_64">,
		detail::enum_entry<arch::arm, "arm">,
		detail::enum_entry<arch::aarch64, "aarch64">,
		detail::enum_entry<arch::wasm32, "wasm32">,
		detail::enum_entry<arch::wasm64, "wasm64">,
		detail::enum_entry<arch::riscv32, "riscv32">,
		detail::enum_entry<arch::riscv64, "riscv64">
	>;

	using platform_triple_names = detail::enum_table<
		platform,
		detail::enum_entry<platform::unknown, "unknown">,
		detail::enum_entry<platform::windows, "windows">,
		detail::enum_entry<platform::linux, "linux">,
		detail::enum_entry<platform::macos, "darwin">,
		detail::enum_entry<platform::wasm, "unknown">,
		detail::enum_entry<platform::freestanding, "none">,
		detail::enum_entry<platform::other_posix, "unknown">
	>;

	using abi_triple_names = detail::enum_table<
		abi,
		detail::enum_entry<abi::unknown, "">,
		detail::enum_entry<abi::msvc, "msvc">,
		detail::enum_entry<abi::gnu, "gnu">,
		detail::enum_entry<abi::musl, "musl">,
		detail::enum_entry<abi::android, "android">,
		detail::enum_entry<abi::eabi, "eabi">,
		detail::enum_entry<abi::none, "">
	>;

	static_assert(arch_triple_names::covers_zero_based_count(arch::count));
	static_assert(platform_triple_names::covers_zero_based_count(platform::count));
	static_assert(abi_triple_names::covers_zero_based_count(abi::count));

	[[nodiscard]] inline constexpr std::string_view triple_name(arch value) noexcept {
		return arch_triple_names::to_string(value);
	}

	[[nodiscard]] inline constexpr std::string_view triple_name(platform value) noexcept {
		return platform_triple_names::to_string(value);
	}

	[[nodiscard]] inline constexpr std::string_view triple_name(abi value) noexcept {
		return abi_triple_names::to_string(value, "");
	}

	[[nodiscard]] inline std::string triple(const target& value) {
		if (!value.m_triple.empty()) {
			return value.m_triple;
		}

		std::string result{ triple_name(value.m_arch) };
		result += "-unknown-";
		result += triple_name(value.m_platform);

		const auto abi_name = triple_name(value.m_abi);

		if (!abi_name.empty()) {
			result += "-";
			result += abi_name;
		}

		return result;
	}
}

#endif
