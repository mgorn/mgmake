#pragma once

#ifndef MGMAKE_SYS_PLATFORM_HXX
#define MGMAKE_SYS_PLATFORM_HXX

#if defined(_WIN32)
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
	// If you wish to have your platform supported, please open an issue so I can look into it.
	#pragma message("MGMake doesn't officially support your platform; expect to run into issues.")
#endif // Platform detection

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
		p_unknown,

		p_windows,
		p_linux,
		p_macos,
		p_wasm,
		p_freestanding,
		p_other_posix,

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
#endif // Architecture detection
	}();

    static constexpr platform g_host_platform = [] constexpr {
#if defined(MGMK_PLATFORM_WINDOWS)
		return platform::p_windows;
#elif defined(MGMK_PLATFORM_WASM)
		return platform::p_wasm;
#elif defined(MGMK_PLATFORM_MACOS)
		return platform::p_macos;
#elif defined(MGMK_PLATFORM_LINUX)
		return platform::p_linux;
#elif defined(MGMK_PLATFORM_OTHER_POSIX)
		return platform::p_other_posix;
#else
		return platform::p_unknown;
#endif // Platform detection
	}();

    static constexpr abi g_host_abi = [] constexpr {
#if defined(_MSC_VER)
		return abi::msvc;
#elif defined(__ANDROID__)
		return abi::android;
#elif defined(__APPLE__)
		return abi::none;
#elif defined(__EMSCRIPTEN__)
		return abi::none;
#elif defined(__MUSL__)
		return abi::musl;
#elif defined(__GNUC__) || defined(__clang__)
		return abi::gnu;
#else
		return abi::unknown;
#endif // ABI detection
	}();

    struct target {
		arch m_arch = arch::unknown;
		platform m_platform = platform::p_unknown;
		abi m_abi = abi::unknown;
		std::string m_triple{};
	};

    static inline constexpr target g_host_target{
		g_host_arch,
		g_host_platform,
		g_host_abi,
		{}
	};
}

#endif // MGMAKE_SYS_PLATFORM_HXX