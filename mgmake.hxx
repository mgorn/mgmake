// This file is generated. Do not edit directly.
// Source: include/mgmake/mgmake.hxx

#ifndef MGMAKE_SINGLE_HEADER_HXX
#define MGMAKE_SINGLE_HEADER_HXX


// ===== begin include/mgmake/mgmake.hxx =====
#pragma once

#ifndef MGMAKE_MGMAKE_HXX
#define MGMAKE_MGMAKE_HXX


// ===== begin include/mgmake/detail/static_string.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_STATIC_STRING_HXX
#define MGMAKE_DETAIL_STATIC_STRING_HXX

#include <array>
#include <cstddef>
#include <string_view>

namespace mgmake::detail {
	template <std::size_t N>
	struct static_string {
		std::array<char, N> m_data{};

		constexpr static_string() = default;

		constexpr static_string(const char (&str)[N]) {
			for (std::size_t i = 0; i < N; ++i) {
				m_data[i] = str[i];
			}
		}

		[[nodiscard]] static consteval std::size_t size() noexcept {
			return N - 1;
		}

		[[nodiscard]] static consteval bool empty() noexcept {
			return size() == 0;
		}

		[[nodiscard]] constexpr std::string_view view() const noexcept {
			return std::string_view{ m_data.data(), size() };
		}

		constexpr operator const char*() const {
			return m_data.data();
		}

		constexpr operator std::string_view() const {
			return view();
		}
	};

	template <std::size_t N1, std::size_t N2>
	constexpr auto operator+(
		const static_string<N1>& a,
		const static_string<N2>& b
	) {
		static_string<N1 + N2 - 1> result;

		for (std::size_t i = 0; i < N1 - 1; ++i) {
			result.m_data[i] = a.m_data[i];
		}

		for (std::size_t i = 0; i < N2; ++i) {
			result.m_data[i + N1 - 1] = b.m_data[i];
		}

		return result;
	}
}

#endif
// ===== end include/mgmake/detail/static_string.hxx =====


// ===== begin include/mgmake/detail/enum_string.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_ENUM_STRING_HXX
#define MGMAKE_DETAIL_ENUM_STRING_HXX

// skipped duplicate include: include/mgmake/detail/static_string.hxx

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace mgmake::detail {
    template <typename E>
    concept enum_type = std::is_enum_v<E>;

    template <enum_type E>
    [[nodiscard]] inline constexpr auto enum_underlying(E value) noexcept {
        return static_cast<std::underlying_type_t<E>>(value);
    }

    template <enum_type E>
    [[nodiscard]] inline constexpr bool enum_underlying_is_negative(E value) noexcept {
        using U = std::underlying_type_t<E>;

        if constexpr (std::is_signed_v<U>) {
            return enum_underlying(value) < 0;
        } else {
            return false;
        }
    }

    template <enum_type E>
    struct enum_view_entry {
        E m_value{};
        std::string_view m_name{};
    };

    template <auto Value, static_string Name> requires enum_type<std::remove_cv_t<decltype(Value)>>
    struct enum_entry {
        using value_type = std::remove_cv_t<decltype(Value)>;

        static constexpr value_type m_value = Value;
        static constexpr auto m_name = Name;

        [[nodiscard]] static constexpr value_type value() noexcept {
            return m_value;
        }

        [[nodiscard]] static constexpr std::string_view name() noexcept {
            return m_name.view();
        }

        [[nodiscard]] static consteval std::size_t name_size() noexcept {
            return m_name.size();
        }

        [[nodiscard]] static consteval bool name_empty() noexcept {
            return m_name.empty();
        }
    };

    template <enum_type E, typename... Entries>
    struct enum_table {
        using value_type = E;

        static_assert(
            (std::same_as<typename Entries::value_type, E> && ...),
            "mgmake enum_table entries must all use the table enum type"
        );

        [[nodiscard]] static consteval std::size_t size() noexcept {
            return sizeof...(Entries);
        }

        [[nodiscard]] static constexpr auto values() noexcept {
            return std::array<E, sizeof...(Entries)>{
                Entries::value()...
            };
        }

        [[nodiscard]] static constexpr auto names() noexcept {
            return std::array<std::string_view, sizeof...(Entries)>{
                Entries::name()...
            };
        }

        [[nodiscard]] static constexpr auto entries() noexcept {
            return std::array<enum_view_entry<E>, sizeof...(Entries)>{
                enum_view_entry<E>{ Entries::value(), Entries::name() }...
            };
        }

        [[nodiscard]] static constexpr bool contains_value(E value) noexcept {
            return ((Entries::value() == value) || ...);
        }

        [[nodiscard]] static constexpr bool contains_name(std::string_view name) noexcept {
            return ((Entries::name() == name) || ...);
        }

        [[nodiscard]] static constexpr std::optional<std::string_view> to_string_opt(
            E value
        ) noexcept {
            std::optional<std::string_view> result{};

            (void)((Entries::value() == value
                ? (result = Entries::name(), true)
                : false) || ...);

            return result;
        }

        [[nodiscard]] static constexpr std::string_view to_string(
            E value,
            std::string_view fallback = "unknown"
        ) noexcept {
            const auto result = to_string_opt(value);

            if (result.has_value()) {
                return *result;
            }

            return fallback;
        }

        [[nodiscard]] static constexpr std::optional<E> from_string(
            std::string_view name
        ) noexcept {
            std::optional<E> result{};

            (void)((Entries::name() == name
                ? (result = Entries::value(), true)
                : false) || ...);

            return result;
        }

        [[nodiscard]] static constexpr E from_string_or(
            std::string_view name,
            E fallback
        ) noexcept {
            const auto result = from_string(name);

            if (result.has_value()) {
                return *result;
            }

            return fallback;
        }

        template <typename Fn>
        static constexpr void for_each_name(Fn&& fn) {
            (fn(Entries::name()), ...);
        }

        template <typename Fn>
        static constexpr void for_each_entry(Fn&& fn) {
            (fn(Entries::value(), Entries::name()), ...);
        }

        [[nodiscard]] static consteval bool has_no_empty_names() noexcept {
            return ((!Entries::name_empty()) && ...);
        }

        [[nodiscard]] static consteval bool has_no_duplicate_names() noexcept {
            constexpr auto table_names = names();

            for (std::size_t i = 0; i < table_names.size(); ++i) {
                for (std::size_t j = i + 1; j < table_names.size(); ++j) {
                    if (table_names[i] == table_names[j]) {
                        return false;
                    }
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool has_no_duplicate_values() noexcept {
            constexpr auto table_values = values();

            for (std::size_t i = 0; i < table_values.size(); ++i) {
                for (std::size_t j = i + 1; j < table_values.size(); ++j) {
                    if (table_values[i] == table_values[j]) {
                        return false;
                    }
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool covers_zero_based_count(E count) noexcept {
            if (enum_underlying_is_negative(count)) {
                return false;
            }

            const auto count_value = static_cast<std::size_t>(enum_underlying(count));

            if (sizeof...(Entries) != count_value) {
                return false;
            }

            constexpr auto table_values = values();

            for (std::size_t expected = 0; expected < count_value; ++expected) {
                bool found = false;

                for (const auto value : table_values) {
                    if (enum_underlying_is_negative(value)) {
                        return false;
                    }

                    if (static_cast<std::size_t>(enum_underlying(value)) == expected) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool is_canonical() noexcept {
            return has_no_empty_names()
                && has_no_duplicate_names()
                && has_no_duplicate_values();
        }

        [[nodiscard]] static consteval bool is_zero_based_count_canonical(
            E count
        ) noexcept {
            return is_canonical()
                && covers_zero_based_count(count);
        }

        [[nodiscard]] static consteval bool is_display_aliases() noexcept {
            return has_no_empty_names()
                && has_no_duplicate_names();
        }

        [[nodiscard]] static consteval bool is_parse_aliases() noexcept {
            return has_no_duplicate_names();
        }

        template <static_string Separator = ", ">
        [[nodiscard]] static consteval auto choices_static_string() noexcept {
            constexpr std::size_t entry_count = sizeof...(Entries);
            constexpr std::size_t separator_size = Separator.size();

            constexpr std::size_t names_size =
                (Entries::name_size() + ... + std::size_t{ 0 });

            constexpr std::size_t separators_size =
                entry_count == 0
                    ? 0
                    : (entry_count - 1) * separator_size;

            constexpr std::size_t total_size = names_size + separators_size;

            static_string<total_size + 1> result{};

            std::size_t out = 0;
            std::size_t index = 0;

            auto append = [&](std::string_view text) constexpr {
                for (const char ch : text) {
                    result.m_data[out] = ch;
                    ++out;
                }
            };

            auto append_entry = [&](std::string_view name) constexpr {
                if (index != 0) {
                    append(Separator.view());
                }

                append(name);
                ++index;
            };

            (append_entry(Entries::name()), ...);

            result.m_data[total_size] = '\0';

            return result;
        }

        template <static_string Separator = ", ">
        [[nodiscard]] static std::string choices_string() {
            constexpr auto result = choices_static_string<Separator>();

            return std::string{ result.view() };
        }
    };
}

#endif
// ===== end include/mgmake/detail/enum_string.hxx =====


// ===== begin include/mgmake/detail/convert.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_CONVERT_HXX
#define MGMAKE_DETAIL_CONVERT_HXX


// ===== begin include/mgmake/detail/assert.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_ASSERT_HXX
#define MGMAKE_DETAIL_ASSERT_HXX

#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string_view>

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

namespace mgmake::detail {

inline void debug_break() {
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__clang__) || defined(__GNUC__)
    __builtin_trap();
#else
    std::abort();
#endif
}

[[noreturn]] inline void assertion_failed(
    std::string_view condition,
    std::string_view message,
    std::source_location location = std::source_location::current()
) {
    std::fprintf(
        stderr,
        "\nmgmake assertion failed\n"
        "  condition: %.*s\n"
        "  message:   %.*s\n"
        "  location:  %s:%u:%u\n"
        "  function:  %s\n\n",
        static_cast<int>(condition.size()),
        condition.data(),
        static_cast<int>(message.size()),
        message.data(),
        location.file_name(),
        location.line(),
        location.column(),
        location.function_name()
    );

    debug_break();

    // In case the platform/debugger allows execution to continue.
    std::abort();
}

inline void mgmk_assert_impl(
    bool condition,
    std::string_view condition_text,
    std::string_view message,
    std::source_location location = std::source_location::current()
) {
    if (!condition) {
        assertion_failed(condition_text, message, location);
    }
}

} // namespace mgmake::detail

#ifndef MGMK_ENABLE_ASSERTS
    #ifndef NDEBUG
        #define MGMK_ENABLE_ASSERTS 1
    #else
        #define MGMK_ENABLE_ASSERTS 0
    #endif
#endif

#if MGMK_ENABLE_ASSERTS
    #define mgmkassert(condition, message)                                      \
        do {                                                                    \
            ::mgmake::detail::mgmk_assert_impl(                                 \
                static_cast<bool>(condition),                                   \
                #condition,                                                     \
                message,                                                        \
                std::source_location::current()                                 \
            );                                                                  \
        } while (false)
#else
    #define mgmkassert(condition, message)                                      \
        do {                                                                    \
            (void)sizeof(condition);                                            \
        } while (false)
#endif

#endif// ===== end include/mgmake/detail/assert.hxx =====


// ===== begin include/mgmake/sys/platform.hxx =====
#pragma once

#ifndef MGMAKE_SYS_PLATFORM_HXX
#define MGMAKE_SYS_PLATFORM_HXX

// skipped duplicate include: include/mgmake/detail/enum_string.hxx

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
// ===== end include/mgmake/sys/platform.hxx =====


#include <limits>
#include <string>
#include <string_view>

namespace mgmake::detail {
#ifdef MGMK_INCLUDED_WINDOWS
	inline constexpr std::string wide_to_utf8(std::wstring_view text) {
		if (text.empty()) {
			return {};
		}

		mgmkassert(text.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()), "Wide string is too large to convert to UTF-8");

		int wide_size = static_cast<int>(text.size());

		int utf8_size = WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			text.data(),
			wide_size,
			nullptr,
			0,
			nullptr,
			nullptr
		);

		mgmkassert(utf8_size > 0, "Failed to calculate UTF-8 argument size");

		std::string result;
		result.resize(static_cast<std::size_t>(utf8_size));

		int written = WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			text.data(),
			wide_size,
			result.data(),
			utf8_size,
			nullptr,
			nullptr
		);

		mgmkassert(written > 0, "Failed to convert command line argument to UTF-8");

		return result;
	}
#endif
}

#endif
// ===== end include/mgmake/detail/convert.hxx =====

// skipped duplicate include: include/mgmake/detail/assert.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

// ===== begin include/mgmake/sys/command_line.hxx =====
#pragma once

#ifndef MGMAKE_SYS_COMMAND_LINE_HXX
#define MGMAKE_SYS_COMMAND_LINE_HXX

// skipped duplicate include: include/mgmake/detail/convert.hxx

// ===== begin include/mgmake/sys/util.hxx =====
#pragma once

#ifndef MGMAKE_SYS_UTIL_HXX
#define MGMAKE_SYS_UTIL_HXX

// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <string>
#include <string_view>

namespace mgmake::sys {
#ifdef MGMK_PLATFORM_WINDOWS
	inline constexpr std::string shell_escape(std::string_view arg) {
		if (arg.empty()) {
			return "\"\"";
		}

		bool needs_quotes = false;

		for (const char ch : arg) {
			if (ch == ' ' || ch == '\t' || ch == '"' || ch == '\\') {
				needs_quotes = true;
				break;
			}
		}

		if (!needs_quotes) {
			return std::string(arg);
		}

		std::string result;
		result += '"';

		std::size_t backslashes = 0;

		for (const char ch : arg) {
			if (ch == '\\') {
				++backslashes;
				continue;
			}

			if (ch == '"') {
				result.append(backslashes * 2 + 1, '\\');
				result += '"';
				backslashes = 0;
				continue;
			}

			if (backslashes != 0) {
				result.append(backslashes, '\\');
				backslashes = 0;
			}

			result += ch;
		}

		if (backslashes != 0) {
			result.append(backslashes * 2, '\\');
		}

		result += '"';
		return result;
	}
#else
	inline constexpr std::string shell_escape(std::string_view arg) {
		if (arg.empty()) {
			return "''";
		}

		bool needs_quotes = false;

		for (const char ch : arg) {
			if (ch == ' ' || ch == '\t' || ch == '\'' || ch == '"' || ch == '$' || ch == '\\' || ch == '&' || ch == ';' || ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '|') {
				needs_quotes = true;
				break;
			}
		}

		if (!needs_quotes) {
			return std::string(arg);
		}

		std::string result;
		result += '\'';

		for (const char ch : arg) {
			if (ch == '\'') {
				result += "'\\''";
			} else {
				result += ch;
			}
		}

		result += '\'';
		return result;
	}
#endif
}

#endif// ===== end include/mgmake/sys/util.hxx =====


#include <cstdlib>
#include <span>
#include <string_view>
#include <vector>

namespace mgmake::sys {
	struct command_line {
		std::vector<std::string> m_args;

		inline constexpr std::string_view program_name() const {
			return m_args.empty() ? std::string_view{} : std::string_view(m_args[0]);
		}

		inline constexpr std::span<const std::string> user_args() const {
			if (m_args.size() <= 1) {
				return {};
			}

			return std::span<const std::string>(m_args).subspan(1);
		}

		inline constexpr std::string full_command() const {
			std::string result;

            for (std::size_t i = 0; i < m_args.size(); ++i) {
                if (i != 0) {
                    result += ' ';
                }

                result += sys::shell_escape(m_args[i]);
            }

            return result;
		}

		auto invoke() const {
			return std::system(full_command().c_str());
		}
	};

	inline constexpr command_line args_from_utf8(int argc, const char** argv) {
		command_line result;

		for (int i = 0; i < argc; ++i) {
			result.m_args.emplace_back(argv[i] ? argv[i] : "");
		}

		return result;
	}

#ifdef MGMK_INCLUDED_WINDOWS
	inline constexpr command_line args_from_wide(int argc, const wchar_t** argv) {
		command_line result;
		if (argc <= 0 || argv == nullptr) {
			return result;
		}
		result.m_args.reserve(static_cast<std::size_t>(argc));
		for (int i = 0; i < argc; ++i) {
			if (argv[i] == nullptr) {
				result.m_args.emplace_back();
				continue;
			}
			result.m_args.emplace_back(detail::wide_to_utf8(argv[i]));
		}
		return result;
	}
#endif
}

#endif// ===== end include/mgmake/sys/command_line.hxx =====

// skipped duplicate include: include/mgmake/sys/util.hxx

// ===== begin include/mgmake/build/toolchain.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_TOOLCHAIN_HXX
#define MGMAKE_BUILD_TOOLCHAIN_HXX

// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::build {
    struct toolchain {
        // Determines how MGMake should pass args to the compiler driver(s)
        enum struct dialect {
            gcc, // GCC-like command arguments (typically with a '-' dash)
            msvc // Windows-like arguments (typically with a '/' slash)
        };

        enum struct target_mode {
            implicit,
            clang_target,
            custom
        };

        std::string m_name; // The name of the toolchain
        dialect m_dialect = []{
#ifdef MGMK_PLATFORM_WINDOWS
            return dialect::msvc;
#else
            return dialect::gcc;
#endif
        }();
        target_mode m_target_mode = target_mode::implicit;

        std::string m_cc;
        std::string m_cxx;
        std::string m_ar;
        std::string m_linker;

        std::vector<std::string> m_compile_flags;
        std::vector<std::string> m_c_flags;
        std::vector<std::string> m_cxx_flags;
        std::vector<std::string> m_archive_flags;
        std::vector<std::string> m_link_flags;

        std::optional<std::string> m_target_triple;
        std::optional<std::string> m_sysroot;

       [[nodiscard]] inline constexpr std::string_view name() const {
            return m_name;
        }
        inline constexpr auto& name(std::string_view str) {
            m_name = str;
            return *this;
        }

        [[nodiscard]] inline constexpr enum dialect dialect() const noexcept {
            return m_dialect;
        }
        inline constexpr auto& dialect(enum dialect value) noexcept {
            m_dialect = value;
            return *this;
        }

        [[nodiscard]] inline constexpr target_mode target_selection() const noexcept {
            return m_target_mode;
        }
        inline constexpr auto& target_selection(target_mode mode) noexcept {
            m_target_mode = mode;
            return *this;
        }

        // cc
        [[nodiscard]] inline constexpr const std::string& cc() const noexcept {
            return m_cc;
        }
        inline constexpr auto& cc(std::string path) {
            m_cc = std::move(path);
            return *this;
        }

        // cxx
        [[nodiscard]] inline constexpr const std::string& cxx() const noexcept {
            return m_cxx;
        }
        inline constexpr auto& cxx(std::string path) {
            m_cxx = std::move(path);
            return *this;
        }

        // ar
        [[nodiscard]] inline constexpr const std::string& ar() const noexcept {
            return m_ar;
        }
        inline constexpr auto& ar(std::string path) {
            m_ar = std::move(path);
            return *this;
        }

        // linker
        [[nodiscard]] inline constexpr const std::string& linker() const noexcept {
            return m_linker;
        }
        inline constexpr auto& linker(std::string path) {
            m_linker = std::move(path);
            return *this;
        }

        // compile_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& compile_flags() const noexcept {
            return m_compile_flags;
        }
        inline constexpr auto& compile_flags(std::vector<std::string> flags) {
            m_compile_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& compile_flags(std::initializer_list<std::string_view> flags) {
            m_compile_flags.clear();

            for (std::string_view flag : flags) {
                m_compile_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_compile_flag(std::string_view flag) {
            m_compile_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_compile_flags() noexcept {
            m_compile_flags.clear();
            return *this;
        }

        // c_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& c_flags() const noexcept {
            return m_c_flags;
        }
        inline constexpr auto& c_flags(std::vector<std::string> flags) {
            m_c_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& c_flags(std::initializer_list<std::string_view> flags) {
            m_c_flags.clear();

            for (std::string_view flag : flags) {
                m_c_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_c_flag(std::string_view flag) {
            m_c_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_c_flags() noexcept {
            m_c_flags.clear();
            return *this;
        }

        // cxx_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& cxx_flags() const noexcept {
            return m_cxx_flags;
        }
        inline constexpr auto& cxx_flags(std::vector<std::string> flags) {
            m_cxx_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& cxx_flags(std::initializer_list<std::string_view> flags) {
            m_cxx_flags.clear();

            for (std::string_view flag : flags) {
                m_cxx_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_cxx_flag(std::string_view flag) {
            m_cxx_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_cxx_flags() noexcept {
            m_cxx_flags.clear();
            return *this;
        }

        // archive_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& archive_flags() const noexcept {
            return m_archive_flags;
        }
        inline constexpr auto& archive_flags(std::vector<std::string> flags) {
            m_archive_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& archive_flags(std::initializer_list<std::string_view> flags) {
            m_archive_flags.clear();

            for (std::string_view flag : flags) {
                m_archive_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_archive_flag(std::string_view flag) {
            m_archive_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_archive_flags() noexcept {
            m_archive_flags.clear();
            return *this;
        }

        // link_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& link_flags() const noexcept {
            return m_link_flags;
        }
        inline constexpr auto& link_flags(std::vector<std::string> flags) {
            m_link_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& link_flags(std::initializer_list<std::string_view> flags) {
            m_link_flags.clear();

            for (std::string_view flag : flags) {
                m_link_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_link_flag(std::string_view flag) {
            m_link_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_link_flags() noexcept {
            m_link_flags.clear();
            return *this;
        }

        // target_triple
        [[nodiscard]] inline constexpr const std::optional<std::string>& target_triple() const noexcept {
            return m_target_triple;
        }
        inline constexpr auto& target_triple(std::string triple) {
            m_target_triple = std::move(triple);
            return *this;
        }
        inline constexpr auto& clear_target_triple() noexcept {
            m_target_triple.reset();
            return *this;
        }

        // sysroot
        [[nodiscard]] inline constexpr const std::optional<std::string>& sysroot() const noexcept {
            return m_sysroot;
        }
        inline constexpr auto& sysroot(std::string path) {
            m_sysroot = std::move(path);
            return *this;
        }
        inline constexpr auto& sysroot(std::optional<std::string> path) {
            m_sysroot = std::move(path);
            return *this;
        }
        inline constexpr auto& clear_sysroot() noexcept {
            m_sysroot.reset();
            return *this;
        }
    };

    static constexpr auto tc_clang_mg = build::toolchain{"clang-mg"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("clang-mg")
        .cxx("clang-mg++")
        .ar("llvm-ar")
        .linker("clang-mg++")
        .target_selection(build::toolchain::target_mode::clang_target);

    static constexpr auto tc_clang = build::toolchain{"clang"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("clang")
        .cxx("clang++")
        .ar("llvm-ar")
        .linker("clang++")
        .target_selection(build::toolchain::target_mode::clang_target);

    static constexpr auto tc_gcc = build::toolchain{"gcc"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("gcc")
        .cxx("g++")
        .ar("ar")
        .linker("g++");
    
    static constexpr auto tc_msvc = build::toolchain{"msvc"}
        .dialect(build::toolchain::dialect::msvc)
        .cc("cl")
        .cxx("cl")
        .ar("lib")
        .linker("link");
}

#endif
// ===== end include/mgmake/build/toolchain.hxx =====


// ===== begin include/mgmake/build/request.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_REQUEST_HXX
#define MGMAKE_BUILD_REQUEST_HXX

// skipped duplicate include: include/mgmake/build/toolchain.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace mgmake::build {
    struct request {
        toolchain m_tc;
        std::filesystem::path m_build_dir;
        std::vector<std::string> m_targets; // Which targets to build, empty = build all
        sys::target m_target = sys::g_host_target;

        [[nodiscard]] inline constexpr const toolchain& toolchain() const {
            return m_tc;
        }
        inline constexpr auto& toolchain(struct toolchain& tc) {
            m_tc = tc;
            return *this;
        }

        [[nodiscard]] inline constexpr const std::filesystem::path& build_dir() const {
            return m_build_dir;
        }
        inline constexpr auto& build_dir(const std::filesystem::path dir) {
            m_build_dir = dir;
            return *this;
        }

        [[nodiscard]] inline const sys::target& target() const noexcept {
            return m_target;
        }

        inline auto& target(sys::target value) {
            m_target = std::move(value);
            return *this;
        }

        [[nodiscard]] inline constexpr sys::platform target_platform() const noexcept {
            return m_target.m_platform;
        }

        inline constexpr auto& target_platform(sys::platform value) noexcept {
            m_target.m_platform = value;
            return *this;
        }
    };
}

#endif
// ===== end include/mgmake/build/request.hxx =====


// ===== begin include/mgmake/build/artifact_names.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_ARTIFACT_NAMES_HXX
#define MGMAKE_BUILD_ARTIFACT_NAMES_HXX

// skipped duplicate include: include/mgmake/detail/enum_string.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

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
// ===== end include/mgmake/build/artifact_names.hxx =====


// ===== begin include/mgmake/build/target.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_TARGET_HXX
#define MGMAKE_BUILD_TARGET_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/build/toolchain.hxx
// skipped duplicate include: include/mgmake/detail/assert.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <string>

namespace mgmake::build {
	[[nodiscard]] inline std::string effective_target_triple(
		const toolchain& tc,
		const request& req
	) {
		if (tc.target_triple().has_value()) {
			return *tc.target_triple();
		}

		return sys::triple(req.target());
	}

	inline void append_target_args(
		sys::command_line& command,
		const toolchain& tc,
		const request& req
	) {
		switch (tc.target_selection()) {
			case toolchain::target_mode::implicit:
			case toolchain::target_mode::custom:
				return;

			case toolchain::target_mode::clang_target: {
				const auto triple = effective_target_triple(tc, req);

				mgmkassert(
					!triple.empty(),
					"mgmake build: clang target mode requires a non-empty target triple"
				);

				command.m_args.emplace_back("-target");
				command.m_args.emplace_back(triple);
				return;
			}
		}

		mgmkassert(false, "mgmake build: unknown toolchain target mode");
	}
}

#endif
// ===== end include/mgmake/build/target.hxx =====


// ===== begin include/mgmake/cli/action.hxx =====
#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

// skipped duplicate include: include/mgmake/detail/enum_string.hxx

#include <optional>
#include <string_view>
#include <utility>

namespace mgmake::cli {
	enum struct action_kind {
		build,
		generate,
		clean,
		run,
		help,
		version,

		count
	};

	using action_kind_names = detail::enum_table<
		action_kind,
		detail::enum_entry<action_kind::build, "build">,
		detail::enum_entry<action_kind::generate, "generate">,
		detail::enum_entry<action_kind::clean, "clean">,
		detail::enum_entry<action_kind::run, "run">,
		detail::enum_entry<action_kind::help, "help">,
		detail::enum_entry<action_kind::version, "version">
	>;

	static_assert(
		action_kind_names::is_zero_based_count_canonical(action_kind::count),
		"action_kind_names must cover every action_kind value exactly once"
	);

	using action_kind_parse_names = detail::enum_table<
		action_kind,
		detail::enum_entry<action_kind::build, "build">,
		detail::enum_entry<action_kind::generate, "generate">,
		detail::enum_entry<action_kind::generate, "gen">,
		detail::enum_entry<action_kind::clean, "clean">,
		detail::enum_entry<action_kind::run, "run">,
		detail::enum_entry<action_kind::help, "help">,
		detail::enum_entry<action_kind::version, "version">
	>;

	static_assert(
		action_kind_parse_names::is_display_aliases(),
		"action_kind_parse_names must not contain duplicate or empty names"
	);

	using action_help_entries = detail::enum_table<
		action_kind,
		detail::enum_entry<
			action_kind::build,
			"Build the project. This is the default command."
		>,
		detail::enum_entry<
			action_kind::generate,
			"Generate backend build files."
		>,
		detail::enum_entry<
			action_kind::clean,
			"Remove generated build output."
		>,
		detail::enum_entry<
			action_kind::run,
			"Build and run a target."
		>,
		detail::enum_entry<
			action_kind::help,
			"Show this help text."
		>,
		detail::enum_entry<
			action_kind::version,
			"Show version information."
		>
	>;

	static_assert(
		action_help_entries::has_no_empty_names()
			&& action_help_entries::has_no_duplicate_values()
			&& action_help_entries::covers_zero_based_count(action_kind::count),
		"action_help_entries must describe every action_kind value exactly once"
	);

	[[nodiscard]] inline constexpr std::string_view action_name(action_kind action) noexcept {
		return action_kind_names::to_string(action);
	}

	template <typename Fn>
	inline constexpr void for_each_action_help(Fn&& fn) {
		action_kind_names::for_each_entry([&](action_kind action, std::string_view name) {
			fn(action, name, action_help_entries::to_string(action, ""));
		});
	}

	[[nodiscard]] inline constexpr std::optional<action_kind> action_from_string(
		std::string_view text
	) noexcept {
		return action_kind_parse_names::from_string(text);
	}

	[[nodiscard]] inline constexpr bool parse_action(
		std::string_view text,
		action_kind& out
	) noexcept {
		const auto parsed = action_from_string(text);

		if (!parsed.has_value()) {
			return false;
		}

		out = *parsed;
		return true;
	}
}

#endif
// ===== end include/mgmake/cli/action.hxx =====


// ===== begin include/mgmake/cli/backend.hxx =====
#pragma once

#ifndef MGMAKE_CLI_BACKEND_HXX
#define MGMAKE_CLI_BACKEND_HXX

// skipped duplicate include: include/mgmake/detail/enum_string.hxx

#include <optional>
#include <string_view>

namespace mgmake::cli {
	enum struct backend_kind {
		automatic,
		graphviz,
		ninja,
		make,
		direct,

		count
	};

	using backend_kind_names = detail::enum_table<
		backend_kind,
		detail::enum_entry<backend_kind::automatic, "auto">,
		detail::enum_entry<backend_kind::graphviz, "graphviz">,
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
		detail::enum_entry<backend_kind::graphviz, "graphviz">,
		detail::enum_entry<backend_kind::graphviz, "dot">,
		detail::enum_entry<backend_kind::graphviz, "graph">,
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

#endif
// ===== end include/mgmake/cli/backend.hxx =====


// ===== begin include/mgmake/cli/options.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

// skipped duplicate include: include/mgmake/cli/action.hxx
// skipped duplicate include: include/mgmake/cli/backend.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <string>
#include <vector>

namespace mgmake::cli {
	struct options {
		action_kind m_action = action_kind::build;
		backend_kind m_backend = backend_kind::automatic;

		std::string m_build_dir = ".build";

		std::vector<std::string> m_targets;
		std::vector<std::string> m_passthrough_args;

		int m_jobs = 0;

		sys::arch m_target_arch = sys::g_host_arch;
		sys::platform m_target_platform = sys::g_host_platform;
		sys::abi m_target_abi = sys::g_host_abi;
		std::string m_target_triple{};

		bool m_verbose = false;
		bool m_dry_run = false;
		bool m_show_help = false;
		bool m_show_version = false;

		[[nodiscard]] inline sys::target target() const {
			return sys::target{
				m_target_arch,
				m_target_platform,
				m_target_abi,
				m_target_triple
			};
		}
	};
}

#endif
// ===== end include/mgmake/cli/options.hxx =====


// ===== begin include/mgmake/cli/parse_result.hxx =====
#pragma once

#ifndef MGMAKE_CLI_PARSE_RESULT_HXX
#define MGMAKE_CLI_PARSE_RESULT_HXX

// skipped duplicate include: include/mgmake/cli/options.hxx

#include <string>
#include <utility>

namespace mgmake::cli {
	struct parse_result {
		bool m_ok = false;
		options m_value{};
		std::string m_error{};

		[[nodiscard]] operator bool() const {
			return m_ok;
		}

		[[nodiscard]] operator options() const {
			return m_value;
		}

		static parse_result success(options opts) {
			parse_result result;
			result.m_ok = true;
			result.m_value = std::move(opts);
			return result;
		}

		static parse_result failure(std::string message) {
			parse_result result;
			result.m_ok = false;
			result.m_error = std::move(message);
			return result;
		}
	};
}

#endif
// ===== end include/mgmake/cli/parse_result.hxx =====


// ===== begin include/mgmake/cli/option_parse_result.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTION_PARSE_RESULT_HXX
#define MGMAKE_CLI_OPTION_PARSE_RESULT_HXX

#include <string>
#include <utility>

namespace mgmake::cli {
	struct option_parse_result {
		bool m_matched = false;
		bool m_ok = true;
		std::string m_error{};

		[[nodiscard]] static option_parse_result no_match() {
			return {};
		}

		[[nodiscard]] static option_parse_result success() {
			option_parse_result result;
			result.m_matched = true;
			result.m_ok = true;
			return result;
		}

		[[nodiscard]] static option_parse_result failure(std::string error) {
			option_parse_result result;
			result.m_matched = true;
			result.m_ok = false;
			result.m_error = std::move(error);
			return result;
		}
	};
}

#endif
// ===== end include/mgmake/cli/option_parse_result.hxx =====


// ===== begin include/mgmake/cli/value_parser.hxx =====
#pragma once

#ifndef MGMAKE_CLI_VALUE_PARSER_HXX
#define MGMAKE_CLI_VALUE_PARSER_HXX

// skipped duplicate include: include/mgmake/cli/backend.hxx

// ===== begin include/mgmake/cli/util.hxx =====
#pragma once

#ifndef MGMAKE_CLI_UTIL_HXX
#define MGMAKE_CLI_UTIL_HXX

#include <charconv>
#include <string_view>

namespace mgmake::cli {
	[[nodiscard]] inline constexpr bool parse_positive_int(std::string_view text, int& out) {
		if (text.empty()) {
			return false;
		}

		int value = 0;

		const char* first = text.data();
		const char* last = text.data() + text.size();

		auto [ptr, ec] = std::from_chars(first, last, value);

		if (ec != std::errc{} || ptr != last || value <= 0) {
			return false;
		}

		out = value;
		return true;
	}

	[[nodiscard]] inline constexpr bool is_option(std::string_view arg) {
		return arg.size() >= 2 && arg[0] == '-';
	}

}

#endif
// ===== end include/mgmake/cli/util.hxx =====


#include <format>
#include <string>
#include <string_view>

namespace mgmake::cli {
	template <typename T>
	struct value_parser;

	template <>
	struct value_parser<std::string> {
		[[nodiscard]] static bool parse(std::string_view text, std::string& out) {
			out = std::string{ text };
			return true;
		}

		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format("invalid string value '{}'", text);
		}
	};

	template <>
	struct value_parser<int> {
		[[nodiscard]] static bool parse(std::string_view text, int& out) {
			return parse_positive_int(text, out);
		}

		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format("invalid integer value '{}'", text);
		}
	};
}

#endif
// ===== end include/mgmake/cli/value_parser.hxx =====


// ===== begin include/mgmake/cli/enum_value_parser.hxx =====
#pragma once

#ifndef MGMAKE_CLI_ENUM_VALUE_PARSER_HXX
#define MGMAKE_CLI_ENUM_VALUE_PARSER_HXX

// skipped duplicate include: include/mgmake/cli/action.hxx
// skipped duplicate include: include/mgmake/cli/backend.hxx
// skipped duplicate include: include/mgmake/cli/value_parser.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace mgmake::cli {
	template <typename ParseTable, typename ChoiceTable = ParseTable>
	struct enum_value_parser {
		using value_type = typename ParseTable::value_type;

		[[nodiscard]] static constexpr bool parse(
			std::string_view text,
			value_type& out
		) noexcept {
			const auto parsed = ParseTable::from_string(text);

			if (!parsed.has_value()) {
				return false;
			}

			out = *parsed;
			return true;
		}

		template <typename Fn>
		static constexpr void for_each_choice(Fn&& fn) {
			ChoiceTable::for_each_name(std::forward<Fn>(fn));
		}

		[[nodiscard]] static std::string choices_string() {
			return ChoiceTable::choices_string();
		}

		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown value '{}'; expected one of: {}",
				text,
				ChoiceTable::choices_string()
			);
		}
	};

	template <>
	struct value_parser<backend_kind> :
		enum_value_parser<backend_kind_parse_names, backend_kind_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown backend '{}'; expected one of: {}",
				text,
				backend_kind_names::choices_string()
			);
		}
	};

	template <>
	struct value_parser<action_kind> :
		enum_value_parser<action_kind_parse_names, action_kind_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown action '{}'; expected one of: {}",
				text,
				action_kind_names::choices_string()
			);
		}
	};

	template <>
	struct value_parser<sys::platform> :
		enum_value_parser<sys::platform_parse_names, sys::platform_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown platform '{}'; expected one of: {}",
				text,
				sys::platform_names::choices_string()
			);
		}
	};

	template <>
	struct value_parser<sys::arch> :
		enum_value_parser<sys::arch_parse_names, sys::arch_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown architecture '{}'; expected one of: {}",
				text,
				sys::arch_names::choices_string()
			);
		}
	};

	template <>
	struct value_parser<sys::abi> :
		enum_value_parser<sys::abi_parse_names, sys::abi_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown ABI '{}'; expected one of: {}",
				text,
				sys::abi_names::choices_string()
			);
		}
	};
}

#endif
// ===== end include/mgmake/cli/enum_value_parser.hxx =====


// ===== begin include/mgmake/cli/option_builder.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTION_BUILDER_HXX
#define MGMAKE_CLI_OPTION_BUILDER_HXX

// skipped duplicate include: include/mgmake/cli/option_parse_result.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx
// skipped duplicate include: include/mgmake/cli/value_parser.hxx
// skipped duplicate include: include/mgmake/cli/enum_value_parser.hxx
// skipped duplicate include: include/mgmake/cli/util.hxx
// skipped duplicate include: include/mgmake/detail/static_string.hxx

#include <concepts>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace mgmake::cli {
	template <detail::static_string... Values>
	struct choice_list {
		[[nodiscard]] static consteval bool empty() noexcept {
			return sizeof...(Values) == 0;
		}

		[[nodiscard]] static constexpr bool contains(std::string_view value) noexcept {
			return ((value == Values.view()) || ...);
		}

		template <typename Fn>
		static constexpr void for_each(Fn&& fn) {
			(fn(Values.view()), ...);
		}
	};
}

namespace mgmake::cli::detail {
	template <typename T>
	struct member_pointer_traits;

	template <typename Object, typename Value>
	struct member_pointer_traits<Value Object::*> {
		using object_type = Object;
		using value_type = Value;
	};

	template <auto Member>
	using member_value_t = typename member_pointer_traits<decltype(Member)>::value_type;

	template <typename T>
	struct vector_traits {
		static constexpr bool is_vector = false;
	};

	template <typename T, typename Alloc>
	struct vector_traits<std::vector<T, Alloc>> {
		static constexpr bool is_vector = true;
		using value_type = T;
	};

	template <typename T>
	inline constexpr bool is_vector_v = vector_traits<T>::is_vector;

	template <typename T>
	struct option_value_traits {
		using value_type = T;
	};

	template <typename T, typename Alloc>
	struct option_value_traits<std::vector<T, Alloc>> {
		using value_type = T;
	};

	template <typename T>
	using option_value_t = typename option_value_traits<T>::value_type;

	template <typename Parser>
	concept has_for_each_choice = requires {
		Parser::for_each_choice([](std::string_view) {});
	};

	template <typename Parser>
	concept has_choices_string = requires {
		{ Parser::choices_string() } -> std::same_as<std::string>;
	};

	template <mgmake::detail::static_string Long>
	[[nodiscard]] constexpr bool matches_long(std::string_view arg) {
		constexpr auto name = Long.view();

		return arg.size() == name.size() + 2
			&& arg[0] == '-'
			&& arg[1] == '-'
			&& arg.substr(2) == name;
	}

	template <mgmake::detail::static_string Long>
	[[nodiscard]] constexpr bool starts_with_long_equals(std::string_view arg) {
		constexpr auto name = Long.view();

		return arg.size() >= name.size() + 3
			&& arg[0] == '-'
			&& arg[1] == '-'
			&& arg.substr(2, name.size()) == name
			&& arg[2 + name.size()] == '=';
	}

	template <char Short>
	[[nodiscard]] constexpr bool matches_short(std::string_view arg) {
		if constexpr (Short == '\0') {
			return false;
		} else {
			return arg.size() == 2 && arg[0] == '-' && arg[1] == Short;
		}
	}

	template <char Short>
	[[nodiscard]] constexpr bool starts_with_short_value(std::string_view arg) {
		if constexpr (Short == '\0') {
			return false;
		} else {
			return arg.size() > 2 && arg[0] == '-' && arg[1] == Short;
		}
	}

	struct no_field_t {};
	inline constexpr no_field_t no_field{};

	struct no_callback_t {};
	inline constexpr no_callback_t no_callback{};

	template <typename T>
	inline constexpr bool is_no_field_v =
		std::same_as<std::remove_cvref_t<T>, no_field_t>;

	template <typename T>
	inline constexpr bool is_no_callback_v =
		std::same_as<std::remove_cvref_t<T>, no_callback_t>;
}

namespace mgmake::cli {
	enum struct option_mode {
		deduce,
		flag,
		value,
		append,
		callback
	};

	template <typename Option>
	struct option_impl;

	template <
		auto Field = detail::no_field,
		mgmake::detail::static_string LongName = "",
		char ShortName = '\0',
		option_mode Mode = option_mode::deduce,
		mgmake::detail::static_string ValueName = "",
		mgmake::detail::static_string Description = "",
		typename Choices = choice_list<>,
		auto Callback = detail::no_callback
	>
	struct option_builder {
		static constexpr auto field_value = Field;
		static constexpr auto long_name_value = LongName;
		static constexpr char short_name_value = ShortName;
		static constexpr option_mode mode_value = Mode;
		static constexpr auto value_name_value = ValueName;
		static constexpr auto description_value = Description;
		using choices_type = Choices;
		static constexpr auto callback_value = Callback;

		[[nodiscard]] static constexpr std::string_view long_name_view() noexcept {
			return LongName.view();
		}

		[[nodiscard]] static constexpr std::string_view value_name_view() noexcept {
			return ValueName.view();
		}

		[[nodiscard]] static constexpr std::string_view description_view() noexcept {
			return Description.view();
		}

		template <auto NewField>
		using field = option_builder<NewField, LongName, ShortName, Mode, ValueName, Description, Choices, Callback>;

		template <mgmake::detail::static_string NewName>
		using long_name = option_builder<Field, NewName, ShortName, Mode, ValueName, Description, Choices, Callback>;

		template <mgmake::detail::static_string NewName>
		using name = long_name<NewName>;

		template <char NewShortName>
		using short_name = option_builder<Field, LongName, NewShortName, Mode, ValueName, Description, Choices, Callback>;

		using flag = option_builder<Field, LongName, ShortName, option_mode::flag, ValueName, Description, Choices, Callback>;
		using value = option_builder<Field, LongName, ShortName, option_mode::value, ValueName, Description, Choices, Callback>;
		using append = option_builder<Field, LongName, ShortName, option_mode::append, ValueName, Description, Choices, Callback>;

		template <mgmake::detail::static_string NewValueName>
		using value_name = option_builder<Field, LongName, ShortName, Mode, NewValueName, Description, Choices, Callback>;

		template <mgmake::detail::static_string NewDescription>
		using description = option_builder<Field, LongName, ShortName, Mode, ValueName, NewDescription, Choices, Callback>;

		template <mgmake::detail::static_string... NewChoices>
		using choices = option_builder<Field, LongName, ShortName, Mode, ValueName, Description, choice_list<NewChoices...>, Callback>;

		template <auto NewCallback>
		using callback = option_builder<Field, LongName, ShortName, option_mode::callback, ValueName, Description, Choices, NewCallback>;

		static option_parse_result try_parse(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			return option_impl<option_builder>::try_parse(opts, args, index, arg);
		}
	};

	template <auto Field, mgmake::detail::static_string LongName, char ShortName = '\0'>
	using value_option =
		typename option_builder<>::template field<Field>
			::template long_name<LongName>
			::template short_name<ShortName>
			::value;

	template <auto Field, mgmake::detail::static_string LongName, char ShortName = '\0'>
	using flag_option =
		typename option_builder<>::template field<Field>
			::template long_name<LongName>
			::template short_name<ShortName>
			::flag;

	template <auto Field, mgmake::detail::static_string LongName, char ShortName = '\0'>
	using append_option =
		typename option_builder<>::template field<Field>
			::template long_name<LongName>
			::template short_name<ShortName>
			::append;

	template <mgmake::detail::static_string LongName, char ShortName, auto Callback>
	using callback_option =
		typename option_builder<>::template long_name<LongName>
			::template short_name<ShortName>
			::template callback<Callback>;
}

namespace mgmake::cli::detail {
	template <auto Field, option_mode Mode>
	struct actual_option_mode {
		static constexpr option_mode value = Mode;
	};

	template <auto Field>
	struct actual_option_mode<Field, option_mode::deduce> {
		static_assert(
			!is_no_field_v<decltype(Field)>,
			"option_mode::deduce requires a field; use .flag, .value, .append, or .callback explicitly"
		);

		using field_type = member_value_t<Field>;

		static constexpr option_mode value = [] {
			if constexpr (std::same_as<field_type, bool>) {
				return option_mode::flag;
			} else if constexpr (is_vector_v<field_type>) {
				return option_mode::append;
			} else {
				return option_mode::value;
			}
		}();
	};
}

namespace mgmake::cli {
	template <
		auto Field,
		mgmake::detail::static_string LongName,
		char ShortName,
		option_mode Mode,
		mgmake::detail::static_string ValueName,
		mgmake::detail::static_string Description,
		typename Choices,
		auto Callback
	>
	struct option_impl<
		option_builder<Field, LongName, ShortName, Mode, ValueName, Description, Choices, Callback>
	> {
		static_assert(
			Mode == option_mode::callback || !detail::is_no_field_v<decltype(Field)>,
			"non-callback CLI options require a field"
		);

		static_assert(
			Mode != option_mode::callback || !detail::is_no_callback_v<decltype(Callback)>,
			"callback CLI options require a callback"
		);

		static option_parse_result try_parse(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			constexpr option_mode actual_mode = detail::actual_option_mode<Field, Mode>::value;

			if constexpr (actual_mode == option_mode::callback) {
				return try_parse_callback(opts, arg);
			} else if constexpr (actual_mode == option_mode::flag) {
				return try_parse_flag(opts, arg);
			} else if constexpr (actual_mode == option_mode::value) {
				return try_parse_value(opts, args, index, arg);
			} else if constexpr (actual_mode == option_mode::append) {
				return try_parse_append(opts, args, index, arg);
			} else {
				static_assert(actual_mode != option_mode::deduce);
			}
		}

		static option_parse_result try_parse_callback(options& opts, std::string_view arg) {
			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				Callback(opts);
				return option_parse_result::success();
			}

			return option_parse_result::no_match();
		}

		static option_parse_result try_parse_flag(options& opts, std::string_view arg) {
			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				opts.*Field = true;
				return option_parse_result::success();
			}

			return option_parse_result::no_match();
		}

		static option_parse_result try_parse_value(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			using value_type = detail::member_value_t<Field>;

			if (detail::starts_with_long_equals<LongName>(arg)) {
				constexpr auto name = LongName.view();
				std::string_view value = arg.substr(3 + name.size());

				if (value.empty()) {
					return option_parse_result::failure(
						std::format("missing value after '--{}='", name)
					);
				}

				return parse_and_assign<value_type>(opts, value);
			}

			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				if (index + 1 >= args.size()) {
					return option_parse_result::failure(
						std::format("missing value after '{}'", arg)
					);
				}

				++index;
				return parse_and_assign<value_type>(opts, args[index]);
			}

			if (detail::starts_with_short_value<ShortName>(arg)) {
				return parse_and_assign<value_type>(opts, arg.substr(2));
			}

			return option_parse_result::no_match();
		}

		static option_parse_result try_parse_append(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			using vector_type = detail::member_value_t<Field>;
			using value_type = typename detail::vector_traits<vector_type>::value_type;

			if (detail::starts_with_long_equals<LongName>(arg)) {
				constexpr auto name = LongName.view();
				std::string_view value = arg.substr(3 + name.size());

				if (value.empty()) {
					return option_parse_result::failure(
						std::format("missing value after '--{}='", name)
					);
				}

				return parse_and_append<value_type>(opts, value);
			}

			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				if (index + 1 >= args.size()) {
					return option_parse_result::failure(
						std::format("missing value after '{}'", arg)
					);
				}

				++index;
				return parse_and_append<value_type>(opts, args[index]);
			}

			if (detail::starts_with_short_value<ShortName>(arg)) {
				return parse_and_append<value_type>(opts, arg.substr(2));
			}

			return option_parse_result::no_match();
		}

		template <typename Value>
		static option_parse_result parse_and_assign(options& opts, std::string_view text) {
			if constexpr (!Choices::empty()) {
				if (!Choices::contains(text)) {
					return option_parse_result::failure(
						std::format("invalid value '{}' for option '--{}'", text, LongName.view())
					);
				}
			}

			Value value{};

			if (!value_parser<Value>::parse(text, value)) {
				return option_parse_result::failure(value_parser<Value>::error(text));
			}

			opts.*Field = std::move(value);
			return option_parse_result::success();
		}

		template <typename Value>
		static option_parse_result parse_and_append(options& opts, std::string_view text) {
			if constexpr (!Choices::empty()) {
				if (!Choices::contains(text)) {
					return option_parse_result::failure(
						std::format("invalid value '{}' for option '--{}'", text, LongName.view())
					);
				}
			}

			Value value{};

			if (!value_parser<Value>::parse(text, value)) {
				return option_parse_result::failure(value_parser<Value>::error(text));
			}

			(opts.*Field).emplace_back(std::move(value));
			return option_parse_result::success();
		}
	};
}

#endif
// ===== end include/mgmake/cli/option_builder.hxx =====


// ===== begin include/mgmake/cli/option_parser.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTION_PARSER_HXX
#define MGMAKE_CLI_OPTION_PARSER_HXX

// skipped duplicate include: include/mgmake/cli/option_builder.hxx
// skipped duplicate include: include/mgmake/cli/parse_result.hxx
// skipped duplicate include: include/mgmake/cli/util.hxx

#include <format>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace mgmake::cli::detail {
	template <typename Option>
	[[nodiscard]] inline std::string option_usage_string() {
		std::string result;

		if constexpr (Option::short_name_value != '\0') {
			result += "-";
			result += Option::short_name_value;
			result += ", ";
		}

		result += "--";
		result += Option::long_name_view();

		constexpr option_mode mode = actual_option_mode<
			Option::field_value,
			Option::mode_value
		>::value;

		if constexpr (mode == option_mode::value || mode == option_mode::append) {
			result += " <";

			if constexpr (!Option::value_name_value.empty()) {
				result += Option::value_name_view();
			} else {
				result += "value";
			}

			result += ">";
		}

		return result;
	}

	template <typename Option>
	[[nodiscard]] inline std::string explicit_choices_string() {
		std::string result;
		bool first = true;

		Option::choices_type::for_each([&](std::string_view choice) {
			if (!first) {
				result += ", ";
			}

			result += choice;
			first = false;
		});

		return result;
	}

	template <typename Parser>
	[[nodiscard]] inline std::string parser_choices_string() {
		if constexpr (has_choices_string<Parser>) {
			return Parser::choices_string();
		} else if constexpr (has_for_each_choice<Parser>) {
			std::string result;
			bool first = true;

			Parser::for_each_choice([&](std::string_view choice) {
				if (!first) {
					result += ", ";
				}

				result += choice;
				first = false;
			});

			return result;
		} else {
			return {};
		}
	}

	template <typename Option>
	[[nodiscard]] inline std::string option_choices_string() {
		if constexpr (!Option::choices_type::empty()) {
			return explicit_choices_string<Option>();
		} else {
			constexpr option_mode mode = actual_option_mode<
				Option::field_value,
				Option::mode_value
			>::value;

			if constexpr (mode == option_mode::value || mode == option_mode::append) {
				using field_type = member_value_t<Option::field_value>;
				using value_type = option_value_t<field_type>;

				return parser_choices_string<value_parser<value_type>>();
			} else {
				return {};
			}
		}
	}

	template <typename Option>
	inline void print_option_help_row() {
		std::string usage = option_usage_string<Option>();
		std::string desc = std::string{ Option::description_view() };
		std::string choices = option_choices_string<Option>();

		if (!choices.empty()) {
			if (!desc.empty()) {
				desc += " ";
			}

			desc += "Choices: ";
			desc += choices;
			desc += ".";
		}

		std::println("  {:<24} {}", usage, desc);
	}

	inline void print_commands_help() {
		for_each_action_help([](
			[[maybe_unused]] action_kind action,
			std::string_view name,
			std::string_view description
		) {
			std::println("  {:<10} {}", name, description);
		});
	}
}

namespace mgmake::cli {
	template <typename... Options>
	struct option_parser {
		static void print_options_help() {
			(detail::print_option_help_row<Options>(), ...);
		}

		static void print_help(std::string_view program_name) {
			if (program_name.empty()) {
				program_name = "mgmake";
			}

			std::println("usage:");
			std::println("  {} [command] [options] [targets...] [-- passthrough...]", program_name);
			std::println("");
			std::println("commands:");
			detail::print_commands_help();
			std::println("");
			std::println("options:");
			print_options_help();
		}

		[[nodiscard]] static option_parse_result try_parse_option(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			option_parse_result result = option_parse_result::no_match();
			bool matched = false;

			([&] {
				if (!matched) {
					result = Options::try_parse(opts, args, index, arg);
					matched = result.m_matched;
				}
			}(), ...);

			return result;
		}

		[[nodiscard]] static parse_result parse(std::span<const std::string> args) {
			options opts;
			bool saw_first_positional = false;

			for (std::size_t i = 0; i < args.size(); ++i) {
				std::string_view arg = args[i];

				if (arg == "--") {
					for (++i; i < args.size(); ++i) {
						opts.m_passthrough_args.emplace_back(args[i]);
					}

					break;
				}

				option_parse_result option_result = try_parse_option(opts, args, i, arg);

				if (option_result.m_matched) {
					if (!option_result.m_ok) {
						return parse_result::failure(std::move(option_result.m_error));
					}

					continue;
				}

				if (is_option(arg)) {
					return parse_result::failure(std::format("unknown option '{}'", arg));
				}

				if (!saw_first_positional) {
					saw_first_positional = true;

					action_kind parsed_action{};

					if (parse_action(arg, parsed_action)) {
						opts.m_action = parsed_action;

						if (parsed_action == action_kind::help) {
							opts.m_show_help = true;
						}

						if (parsed_action == action_kind::version) {
							opts.m_show_version = true;
						}

						continue;
					}
				}

				opts.m_targets.emplace_back(arg);
			}

			return parse_result::success(std::move(opts));
		}
	};
}

#endif
// ===== end include/mgmake/cli/option_parser.hxx =====


// ===== begin include/mgmake/cli/parse.hxx =====
#pragma once

#ifndef MGMAKE_CLI_PARSE_HXX
#define MGMAKE_CLI_PARSE_HXX

// skipped duplicate include: include/mgmake/cli/option_parser.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx

namespace mgmake::cli {
	inline void apply_help(options& opts) {
		opts.m_action = action_kind::help;
		opts.m_show_help = true;
	}

	inline void apply_version(options& opts) {
		opts.m_action = action_kind::version;
		opts.m_show_version = true;
	}

	using help_option =
		callback_option<"help", 'h', apply_help>
			::description<"Show help.">;

	using version_option =
		callback_option<"version", '\0', apply_version>
			::description<"Show version information.">;

	using verbose_option =
		flag_option<&options::m_verbose, "verbose", 'v'>
			::description<"Print commands before executing them.">;

	using dry_run_option =
		flag_option<&options::m_dry_run, "dry-run">
			::description<"Print commands without executing them.">;

	using backend_option =
		value_option<&options::m_backend, "backend">
			::value_name<"name">
			::description<"Select a build backend to use.">;

	using build_dir_option =
		value_option<&options::m_build_dir, "build-dir">
			::value_name<"path">
			::description<"Set the build directory.">;

	using jobs_option =
		value_option<&options::m_jobs, "jobs", 'j'>
			::value_name<"count">
			::description<"Set the maximum number of parallel jobs.">;

	using target_option =
		append_option<&options::m_targets, "target">
			::value_name<"name">
			::description<"Build a specific target. May be passed multiple times.">;

	using platform_option =
		value_option<&options::m_target_platform, "platform">
			::value_name<"platform">
			::description<"Set the target platform used for artifact naming.">;

	using arch_option =
		value_option<&options::m_target_arch, "arch">
			::value_name<"arch">
			::description<"Set the target architecture.">;

	using abi_option =
		value_option<&options::m_target_abi, "abi">
			::value_name<"abi">
			::description<"Set the target ABI.">;

	using target_triple_option =
		value_option<&options::m_target_triple, "target-triple">
			::value_name<"triple">
			::description<"Set an explicit compiler target triple for target-aware toolchains.">;

	using default_parser = option_parser<
		help_option,
		version_option,
		verbose_option,
		dry_run_option,
		backend_option,
		build_dir_option,
		jobs_option,
		target_option,
		platform_option,
		arch_option,
		abi_option,
		target_triple_option
	>;

	[[nodiscard]] inline parse_result parse(std::span<const std::string> args) {
		return default_parser::parse(args);
	}

	inline auto parse(const sys::command_line& cmd) {
		return parse(cmd.user_args());
	}

	inline void print_help(std::string_view program_name) {
		default_parser::print_help(program_name);
	}
}

#endif
// ===== end include/mgmake/cli/parse.hxx =====

// skipped duplicate include: include/mgmake/cli/util.hxx

// ===== begin include/mgmake/dag/action.hxx =====
#pragma once

#ifndef MGMAKE_DAG_ACTION_HXX
#define MGMAKE_DAG_ACTION_HXX


// ===== begin include/mgmake/dag/artifact.hxx =====
#pragma once

#ifndef MGMAKE_DAG_ARTIFACT_HXX
#define MGMAKE_DAG_ARTIFACT_HXX

#include <filesystem>
#include <vector>

namespace mgmake::dag {
    struct artifact {
        using id = std::vector<artifact>::size_type;

        enum struct kind {
            source, // A source code (.cxx, .c, etc)
            generated, // Generated by another program
            phony // Fake placeholder
        } m_kind = kind::source;
        std::filesystem::path m_path;
    };
}

#endif// ===== end include/mgmake/dag/artifact.hxx =====


// skipped duplicate include: include/mgmake/sys/command_line.hxx

#include <filesystem>
#include <string>
#include <vector>

namespace mgmake::dag {
    struct action {
        using id = std::vector<action>::size_type;

        std::string m_name;
        std::string m_description;

        std::vector<artifact::id> m_inputs;
        std::vector<artifact::id> m_outputs;

        bool m_always_run = false; // always_fun = false :(

        sys::command_line m_command;
        std::filesystem::path m_working_directory;
    };
}

#endif// ===== end include/mgmake/dag/action.hxx =====

// skipped duplicate include: include/mgmake/dag/artifact.hxx

// ===== begin include/mgmake/dag/graph.hxx =====
#pragma once

#ifndef MGMAKE_DAG_GRAPH_HXX
#define MGMAKE_DAG_GRAPH_HXX

// skipped duplicate include: include/mgmake/detail/assert.hxx
// skipped duplicate include: include/mgmake/dag/action.hxx
// skipped duplicate include: include/mgmake/dag/artifact.hxx

// ===== begin include/mgmake/dag/target.hxx =====
#pragma once

#ifndef MGMAKE_DAG_TARGET_HXX
#define MGMAKE_DAG_TARGET_HXX

// skipped duplicate include: include/mgmake/dag/artifact.hxx

#include <set>
#include <string>
#include <vector>

namespace mgmake::dag {
    struct target {
        using id = std::vector<target>::size_type;

        std::string m_name;

        // Empty is valid for interface/no-op/metadata targets
        std::set<artifact::id> m_outputs;

        // Other DAG targets this target conceptually depends on.
        // For now this mostly exists so graphviz/ninja can show target-level deps.
        std::set<target::id> m_dependencies;

		void add_dependency(const target::id dep) {
			m_dependencies.emplace(dep);
		}
    };
}

#endif// ===== end include/mgmake/dag/target.hxx =====


#include <utility>
#include <vector>

namespace mgmake::dag {
    struct graph {
        std::vector<artifact> m_artifacts;
        std::vector<action> m_actions;
        std::vector<target> m_targets;

        inline constexpr artifact::id create_artifact(auto&&... args) {
            m_artifacts.emplace_back(std::forward<decltype(args)>(args)...);
            return { m_artifacts.size() - 1 };
        }
        inline constexpr action::id create_action(auto&&... args) {
            m_actions.emplace_back(std::forward<decltype(args)>(args)...);
            return { m_actions.size() - 1 };
        }
        inline constexpr target::id create_target(auto&&... args) {
            m_targets.emplace_back(std::forward<decltype(args)>(args)...);
            return { m_targets.size() - 1 };
        }

		inline constexpr struct artifact& artifact(const artifact::id id) {
			mgmkassert(not m_artifacts.empty(), "Invalid artifact ID: there are no artifacts.");
			mgmkassert(id < m_artifacts.size(), "Invalid artifact ID");
			return m_artifacts.at(id);
		}
		inline constexpr const struct artifact& artifact(const artifact::id id) const {
			mgmkassert(not m_artifacts.empty(), "Invalid artifact ID: there are no artifacts.");
			mgmkassert(id < m_artifacts.size(), "Invalid artifact ID");
			return m_artifacts.at(id);
		}
		inline constexpr struct action& action(const action::id id) {
			mgmkassert(not m_actions.empty(), "Invalid action ID: there are no actions.");
			mgmkassert(id < m_actions.size(), "Invalid action ID");
			return m_actions.at(id);
		}
		inline constexpr const struct action& action(const action::id id) const {
			mgmkassert(not m_actions.empty(), "Invalid action ID: there are no actions.");
			mgmkassert(id < m_actions.size(), "Invalid action ID");
			return m_actions.at(id);
		}
		inline constexpr struct target& target(const target::id id) {
			mgmkassert(not m_targets.empty(), "Invalid target ID: there are no targets.");
			mgmkassert(id < m_targets.size(), "Invalid target ID");
			return m_targets.at(id);
		}
		inline constexpr const struct target& target(const target::id id) const {
			mgmkassert(not m_targets.empty(), "Invalid target ID: there are no targets.");
			mgmkassert(id < m_targets.size(), "Invalid target ID");
			return m_targets.at(id);
		}
    };
}

#endif// ===== end include/mgmake/dag/graph.hxx =====

// skipped duplicate include: include/mgmake/dag/target.hxx

// ===== begin include/mgmake/backend/traits.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_TRAITS_HXX
#define MGMAKE_BACKEND_TRAITS_HXX

// skipped duplicate include: include/mgmake/dag/graph.hxx

namespace mgmake::backend {
    // Generates graph output (graph.dot, build.ninja)
    trait generator {
        void generate(const dag::graph& graph, const build::request& req) const;
    };
    // Actually builds the program from the graph (invokes compiler, runs ninja)
    trait builder {
        void build(const dag::graph& graph, const build::request& req) const;
    };
}

#endif// ===== end include/mgmake/backend/traits.hxx =====


// ===== begin include/mgmake/backend/graphviz.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_GRAPHVIZ_HXX
#define MGMAKE_BACKEND_GRAPHVIZ_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/dag/artifact.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx

#include <filesystem>
#include <fstream>

namespace mgmake::backend {
    namespace detail {
        inline std::string dot_escape(std::string_view text) {
            std::string result;

            for (const char ch : text) {
                switch (ch) {
                    case '\\':
                        result += "\\\\";
                        break;

                    case '"':
                        result += "\\\"";
                        break;

                    case '\n':
                        result += "\\n";
                        break;

                    case '\r':
                        break;

                    default:
                        result += ch;
                        break;
                }
            }

            return result;
        }

        inline std::string dot_label(const std::filesystem::path& path) {
            return dot_escape(path.generic_string());
        }

        inline std::string artifact_shape(dag::artifact::kind kind) {
            switch (kind) {
                case dag::artifact::kind::source:
                    return "box";

                case dag::artifact::kind::generated:
                    return "component";

                case dag::artifact::kind::phony:
                    return "note";
            }

            return "box";
        }

        inline std::string artifact_kind_name(dag::artifact::kind kind) {
            switch (kind) {
                case dag::artifact::kind::source:
                    return "source";

                case dag::artifact::kind::generated:
                    return "generated";

                case dag::artifact::kind::phony:
                    return "phony";
            }

            return "artifact";
        }
    }

    template<bool show_commands = true, bool show_action_ids = true, bool show_artifact_ids = true, bool show_targets = true>
    struct graphviz {
        std::filesystem::path m_output_file = "graph.dot";

        void generate(const dag::graph& graph, const build::request& req) const {
            auto output_path = req.build_dir() / m_output_file;
            if (output_path.has_parent_path()) {
                std::filesystem::create_directories(output_path.parent_path());
            }
            std::ofstream out(output_path);

            out << "digraph mgmake {\n";
            out << "    rankdir=LR;\n";
            out << "    graph [fontname=\"monospace\"];\n";
            out << "    node [fontname=\"monospace\"];\n";
            out << "    edge [fontname=\"monospace\"];\n\n";

            for (auto i = 0; i < graph.m_artifacts.size(); ++i) {
                const auto& artifact = graph.artifact(i);

                std::string label;

                if constexpr (show_artifact_ids) {
                    label += "artifact ";
                    label += std::to_string(i);
                    label += "\\n";
                }

                label += detail::artifact_kind_name(artifact.m_kind);
                label += "\\n";
                label += detail::dot_label(artifact.m_path);

                out << "    artifact_" << i
                    << " [shape=" << detail::artifact_shape(artifact.m_kind)
                    << ", label=\"" << label << "\"];\n";
            }

            out << "\n";

            for (auto i = 0; i < graph.m_actions.size(); ++i) {
                const auto& action = graph.action(i);

                std::string label;

                if (show_action_ids) {
                    label += "action ";
                    label += std::to_string(i);
                    label += "\\n";
                }

                if (!action.m_name.empty()) {
                    label += detail::dot_escape(action.m_name);
                } else {
                    label += "unnamed action";
                }

                if (!action.m_description.empty()) {
                    label += "\\n";
                    label += detail::dot_escape(action.m_description);
                }

                if (show_commands && !action.m_command.m_args.empty()) {
                    label += "\\n";

                    for (std::size_t arg_i = 0; arg_i < action.m_command.m_args.size(); ++arg_i) {
                        if (arg_i != 0) {
                            label += " ";
                        }

                        label += detail::dot_escape(action.m_command.m_args[arg_i]);
                    }
                }

                out << "    action_" << i
                    << " [shape=ellipse, label=\"" << label << "\"];\n";
            }

            out << "\n";

            if constexpr (show_targets) {
                for (auto i = 0; i < graph.m_targets.size(); ++i) {
                    const auto& target = graph.target(i);

                    std::string label = "target";
                    label += "\\n";
                    label += detail::dot_escape(target.m_name);

                    out << "    target_" << i
                        << " [shape=house, label=\"" << label << "\"];\n";
                }

                out << "\n";
            }

            for (auto i = 0; i < graph.m_actions.size(); ++i) {
                const auto& action = graph.action(i);

                for (const auto input : action.m_inputs) {
                    out << "    artifact_" << input << " -> action_" << i << ";\n";
                }

                for (const auto output : action.m_outputs) {
                    out << "    action_" << i << " -> artifact_" << output << ";\n";
                }
            }

            if constexpr (show_targets) {
                out << "\n";

                for (auto i = 0; i < graph.m_targets.size(); ++i) {
                    const auto& target = graph.target(i);

                    for (const auto output : target.m_outputs) {
                        out << "    target_" << i << " -> artifact_" << output << ";\n";
                    }
                }
            }

            out << "}\n";
        }
    };
}

#endif// ===== end include/mgmake/backend/graphviz.hxx =====


// ===== begin include/mgmake/backend/ninja.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_NINJA_HXX
#define MGMAKE_BACKEND_NINJA_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx
// skipped duplicate include: include/mgmake/sys/util.hxx

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>

namespace mgmake::backend {
    namespace detail {
        inline std::string ninja_escape_build_text(std::string_view text) {
            std::string result;

            for (const char ch : text) {
                switch (ch) {
                    case '$':
                        result += "$$";
                        break;

                    case ' ':
                        result += "$ ";
                        break;

                    case ':':
                        result += "$:";
                        break;

                    case '|':
                        result += "$|";
                        break;

                    case '\n':
                        result += "$\n";
                        break;

                    default:
                        result += ch;
                        break;
                }
            }

            return result;
        }

        inline std::string ninja_escape_path(const std::filesystem::path& path) {
            return ninja_escape_build_text(path.generic_string());
        }

        inline std::string ninja_escape_variable_text(std::string_view text) {
            std::string result;

            for (const char ch : text) {
                switch (ch) {
                    case '$':
                        result += "$$";
                        break;

                    case '\n':
                        result += "$\n";
                        break;

                    default:
                        result += ch;
                        break;
                }
            }

            return result;
        }

        inline void write_artifact_list(std::ofstream& out, const dag::graph& graph, const std::vector<dag::artifact::id>& artifacts) {
            bool first = true;

            for (const auto id : artifacts) {
                const auto& artifact = graph.artifact(id);

                if (!first) {
                    out << ' ';
                }

                out << ninja_escape_path(artifact.m_path);
                first = false;
            }
        }
		inline void write_artifact_list(std::ofstream& out, const dag::graph& graph, const std::set<dag::artifact::id>& artifacts) {
			return write_artifact_list(out, graph, artifacts | std::ranges::to<std::vector<dag::artifact::id>>());
		}

        inline void create_output_directories(const dag::graph& graph) {
            for (const auto& action : graph.m_actions) {
                for (const auto output_id : action.m_outputs) {
                    const auto& output = graph.artifact(output_id);

                    if (output.m_kind == dag::artifact::kind::phony) {
                        continue;
                    }

                    const auto parent = output.m_path.parent_path();

                    if (!parent.empty()) {
                        std::filesystem::create_directories(parent);
                    }
                }
            }
        }

        inline void write_target_defaults(std::ofstream& out, const dag::graph& graph) {
            if (graph.m_targets.empty()) {
                return;
            }

            out << "\ndefault";

            for (const auto& target : graph.m_targets) {
                out << ' ' << ninja_escape_build_text(target.m_name);
            }

            out << "\n";
        }
    }

    struct ninja {
        std::filesystem::path m_output_file = "build.ninja";

        void generate(const dag::graph& graph, const build::request& req) const {
            auto output_path = req.build_dir() / m_output_file;
            if (output_path.has_parent_path()) {
                std::filesystem::create_directories(output_path.parent_path());
            }
            detail::create_output_directories(graph);

            std::ofstream out(output_path);
			mgmkassert(out.is_open(), "ninja backend: failed to open " + output_path.string());

            out << "# generated by mgmake\n\n";

            out << "builddir = "
                << detail::ninja_escape_variable_text(req.build_dir().generic_string())
                << "\n\n";

            bool needs_always = false;

            for (const auto& action : graph.m_actions) {
                if (action.m_always_run) {
                    needs_always = true;
                    break;
                }
            }

            if (needs_always) {
                out << "build __mgmake_always: phony\n\n";
            }

            for (auto i = 0; i < graph.m_actions.size(); ++i) {
                const auto& action = graph.action(i);

                mgmkassert(not action.m_outputs.empty(), "ninja backend: action '" + action.m_name + "' has no outputs");
				mgmkassert(not action.m_command.m_args.empty(), "ninja backend: action '" + action.m_name + "' has no command");

                out << "rule action_" << i << "\n";
                out << "  command = " << action.m_command.full_command() << "\n";

                if (!action.m_description.empty()) {
                    out << "  description = " << detail::ninja_escape_variable_text(action.m_description) << "\n";
                } else if (!action.m_name.empty()) {
                    out << "  description = " << detail::ninja_escape_variable_text(action.m_name) << "\n";
                }

                if (!action.m_working_directory.empty()) {
#if defined(_WIN32) // bruh
                    out << "  command = cd /d "
                        << detail::ninja_escape_variable_text(sys::shell_escape(action.m_working_directory.string()))
                        << " && "
                        << action.m_command.full_command()
                        << "\n";
#else
                    out << "  command = cd "
                        << detail::ninja_escape_variable_text(sys::shell_escape(action.m_working_directory.string()))
                        << " && "
                        << action.m_command.full_command()
                        << "\n";
#endif
                }

                out << "\n";

                out << "build ";
                detail::write_artifact_list(out, graph, action.m_outputs);
                out << ": action_" << i;

                if (!action.m_inputs.empty()) {
                    out << ' ';
                    detail::write_artifact_list(out, graph, action.m_inputs);
                }

                if (action.m_always_run) {
                    out << " | __mgmake_always";
                }

                out << "\n\n";
            }

            for (const auto& target : graph.m_targets) {
                out << "build " << detail::ninja_escape_build_text(target.m_name) << ": phony ";
                if (not target.m_outputs.empty()) {
                    out << ' ';
                    detail::write_artifact_list(out, graph, target.m_outputs);
                }
                for (const auto dep_id : target.m_dependencies) {
                    const auto& dep = graph.target(dep_id);
                    out << ' ' << detail::ninja_escape_build_text(dep.m_name);
                }
                out << "\n";
            }

            detail::write_target_defaults(out, graph);
        }

		std::expected<void, std::string> build(const dag::graph& graph, const build::request& req) const {
            auto output_path = req.build_dir() / m_output_file;
            if (output_path.has_parent_path()) {
                std::filesystem::create_directories(output_path.parent_path());
            }
			generate(graph, req);

			sys::command_line command;
			command.m_args.emplace_back("ninja");
			command.m_args.emplace_back("-f");
			command.m_args.emplace_back(output_path.string());

			const auto exit_code = command.invoke();

			if (exit_code != 0) {
				return std::unexpected(
					"ninja backend: ninja failed with exit code " + std::to_string(exit_code)
				);
			}

			return {};
		}
    };
}

#endif// ===== end include/mgmake/backend/ninja.hxx =====


// ===== begin include/mgmake/spec/executable.hxx =====
#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX


// ===== begin include/mgmake/spec/target.hxx =====
#pragma once

#ifndef MGMK_SPEC_TARGET_HXX
#define MGMK_SPEC_TARGET_HXX

#include <filesystem>
#include <set>
#include <string>
#include <string_view>

namespace mgmake::spec {
	template<typename target_t>
	struct target {
		std::string m_name;
		std::set<std::filesystem::path> m_sources;
		std::set<std::filesystem::path> m_include_dirs;
		std::set<std::string> m_linked_libraries;

		inline constexpr auto& name() const {
			return m_name;
		}

		inline constexpr auto& add_source(const std::filesystem::path& file) {
			m_sources.emplace(file);
			return self();
		}
		inline constexpr auto& sources() const {
			return m_sources;
		}

		inline constexpr auto& add_include_dir(const std::filesystem::path& file) {
			m_include_dirs.emplace(file);
			return self();
		}
		inline constexpr auto& include_dirs() const {
			return m_include_dirs;
		}

		inline constexpr auto& link(std::string_view lib) {
			m_linked_libraries.emplace(lib);
			return self();
		}
		inline constexpr auto& link(const std::string& lib) {
			return link(std::string_view{ lib });
		}
		inline constexpr auto& linked_libraries() const {
			return m_linked_libraries;
		}

		// Implicit cast to std::string_view for when the target needs to be identified by name
		operator std::string_view() const {
			return m_name;
		}
	private:
		inline constexpr target_t& self() {
			return *static_cast<target_t*>(this);
		}
	};
}

#endif
// ===== end include/mgmake/spec/target.hxx =====


#include <vector>

namespace mgmake::spec {
	struct executable : public target<executable> {
		using id = std::vector<executable>::size_type;
	};
}

#endif
// ===== end include/mgmake/spec/executable.hxx =====


// ===== begin include/mgmake/spec/executable_impl.hxx =====
#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_IMPL_HXX
#define MGMK_SPEC_EXECUTABLE_IMPL_HXX

// skipped duplicate include: include/mgmake/spec/executable.hxx

#endif
// ===== end include/mgmake/spec/executable_impl.hxx =====


// ===== begin include/mgmake/spec/library.hxx =====
#pragma once

#ifndef MGMK_SPEC_LIBRARY_HXX
#define MGMK_SPEC_LIBRARY_HXX

// skipped duplicate include: include/mgmake/spec/target.hxx

#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project;

	struct library : public target<library> {
		using id = std::vector<library>::size_type;

		enum struct kind {
			static_lib, // k prefix bc static is a keyword
			shared_lib,
			interface
		} m_kind;

		library(std::string_view name, kind k) : target<library>{ std::string{ name } }, m_kind{k} {}
	};
}

#endif
// ===== end include/mgmake/spec/library.hxx =====


// ===== begin include/mgmake/spec/library_impl.hxx =====
#pragma once

#ifndef MGMK_SPEC_LIBRARY_IMPL_HXX
#define MGMK_SPEC_LIBRARY_IMPL_HXX

// skipped duplicate include: include/mgmake/spec/library.hxx

#endif
// ===== end include/mgmake/spec/library_impl.hxx =====


// ===== begin include/mgmake/spec/project.hxx =====
#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx
// skipped duplicate include: include/mgmake/detail/assert.hxx
// skipped duplicate include: include/mgmake/spec/executable.hxx
// skipped duplicate include: include/mgmake/spec/library.hxx

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project {
		std::string m_name;
		std::vector<spec::executable> m_executables;
		std::vector<spec::library> m_libraries;

		inline constexpr project& add_target(const spec::executable& exe) {
			mgmkassert(not exe.m_name.empty(), "mgmake spec: executable target has no name");
            mgmkassert(not find_library(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");
            mgmkassert(not find_executable(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");

			m_executables.emplace_back(exe);
			return *this;
		}
		inline constexpr project& add_target(const spec::library& lib) {
			mgmkassert(not lib.m_name.empty(), "mgmake spec: library target has no name");
            mgmkassert(not find_executable(lib.m_name).has_value(), "mgmake spec: target name conflict '" + lib.m_name + "'");

			// Skip if the library was already added
			if (find_library(lib.m_name).has_value()) {
                return *this;
            }

			m_libraries.emplace_back(lib);
			return *this;
		}

		const std::optional<spec::library::id> find_library(std::string_view name) const {
            for (spec::library::id idx = 0; idx < m_libraries.size(); idx++) {
				const auto& lib = m_libraries.at(idx);
                if (lib.m_name == name) {
                    return idx;
                }
            }
            return std::nullopt;
		}
		const spec::library* get_library(const spec::library::id idx) const {
			if (idx >= m_libraries.size())
				return nullptr;
			return &m_libraries.at(idx);
		}

		const std::optional<spec::executable::id> find_executable(std::string_view name) const {
            for (spec::executable::id idx = 0; idx < m_executables.size(); idx++) {
				const auto& exe = m_executables.at(idx);
                if (exe.m_name == name) {
                    return idx;
                }
            }
            return std::nullopt;
        }
		const spec::executable* get_executable(const spec::executable::id idx) const {
			if (idx >= m_executables.size())
				return nullptr;
			return &m_executables.at(idx);
		}

		dag::graph graph(const build::request& req) const;
	};
}

#endif
// ===== end include/mgmake/spec/project.hxx =====


// ===== begin include/mgmake/lower/target.hxx =====
#pragma once

#ifndef MGMK_LOWER_TARGET_HXX
#define MGMK_LOWER_TARGET_HXX

// skipped duplicate include: include/mgmake/dag/artifact.hxx
// skipped duplicate include: include/mgmake/dag/target.hxx

#include <filesystem>
#include <optional>
#include <set>
#include <vector>

namespace mgmake::lower {
	struct target {
		std::optional<dag::target::id> m_dag_target;
		std::vector<dag::artifact::id> m_linkable_artifacts;
		// Include dirs inherited by consumers of this lowered target.
		std::set<std::filesystem::path> m_include_dirs;
	};
}

#endif
// ===== end include/mgmake/lower/target.hxx =====


// ===== begin include/mgmake/lower/usage.hxx =====
#pragma once

#ifndef MGMK_LOWER_USAGE_HXX
#define MGMK_LOWER_USAGE_HXX

// skipped duplicate include: include/mgmake/dag/artifact.hxx
// skipped duplicate include: include/mgmake/dag/target.hxx

#include <filesystem>
#include <set>
#include <vector>

namespace mgmake::lower {
	struct usage {
		std::set<std::filesystem::path> m_include_dirs;
		std::vector<dag::artifact::id> m_link_inputs;
		std::set<dag::target::id> m_dag_dependencies;
	};
}

#endif
// ===== end include/mgmake/lower/usage.hxx =====


// ===== begin include/mgmake/lower/emitter.hxx =====
#pragma once

#ifndef MGMK_LOWER_EMITTER_HXX
#define MGMK_LOWER_EMITTER_HXX

// skipped duplicate include: include/mgmake/dag/action.hxx
// skipped duplicate include: include/mgmake/dag/artifact.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx
// skipped duplicate include: include/mgmake/dag/target.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace mgmake::lower {
	struct emitter {
		dag::graph& m_graph;

		emitter(dag::graph& graph)
			: m_graph{graph} {}

		dag::artifact::id source(const std::filesystem::path& path) {
			return m_graph.create_artifact(dag::artifact::kind::source, path);
		}

		dag::artifact::id generated(const std::filesystem::path& path) {
			return m_graph.create_artifact(dag::artifact::kind::generated, path);
		}

		const std::filesystem::path& path(dag::artifact::id id) const {
			return m_graph.artifact(id).m_path;
		}

		dag::action::id action(
			std::string name,
			std::string description,
			std::vector<dag::artifact::id> inputs,
			std::vector<dag::artifact::id> outputs,
			sys::command_line command
		) {
			return m_graph.create_action(
				std::move(name),
				std::move(description),
				std::move(inputs),
				std::move(outputs),
				false,
				std::move(command),
				std::filesystem::path{}
			);
		}

		dag::target::id target(dag::target target) {
			return m_graph.create_target(std::move(target));
		}
	};
}

#endif
// ===== end include/mgmake/lower/emitter.hxx =====


// ===== begin include/mgmake/lower/context.hxx =====
#pragma once

#ifndef MGMK_LOWER_CONTEXT_HXX
#define MGMK_LOWER_CONTEXT_HXX

// skipped duplicate include: include/mgmake/lower/emitter.hxx
// skipped duplicate include: include/mgmake/lower/target.hxx
// skipped duplicate include: include/mgmake/lower/usage.hxx
// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/spec/executable.hxx
// skipped duplicate include: include/mgmake/spec/library.hxx

#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project;
}

namespace mgmake::lower {
	struct context {
		const build::request& m_req;
		const spec::project& m_project;
		lower::emitter m_emit;

		context(
			dag::graph& graph,
			const build::request& req,
			const spec::project& project
		);

		lower::emitter& emit() {
			return m_emit;
		}

		const lower::emitter& emit() const {
			return m_emit;
		}

		const build::request& request() const {
			return m_req;
		}

		const build::toolchain& toolchain() const {
			return m_req.toolchain();
		}

		const lower::target& lower_library(spec::library::id id);
		void lower_executable(spec::executable::id id);

		lower::usage use_libraries(
			const std::set<std::string>& libraries,
			std::string_view owner_name
		);

		template<typename target_t>
		std::vector<dag::artifact::id> lower_objects(
			const target_t& target,
			const std::set<std::filesystem::path>& include_dirs
		);

	private:
		lower::target lower_interface_library(
			const spec::library& lib,
			lower::usage usage
		);

		lower::target lower_static_library(
			const spec::library& lib,
			lower::usage usage
		);

		lower::target lower_shared_library(
			const spec::library& lib,
			lower::usage usage
		);

		std::vector<std::optional<lower::target>> m_libraries;
		std::set<spec::library::id> m_active_libraries;
	};
}

#endif
// ===== end include/mgmake/lower/context.hxx =====


// ===== begin include/mgmake/lower/objects.hxx =====
#pragma once

#ifndef MGMK_LOWER_OBJECTS_HXX
#define MGMK_LOWER_OBJECTS_HXX

// skipped duplicate include: include/mgmake/lower/context.hxx
// skipped duplicate include: include/mgmake/build/target.hxx
// skipped duplicate include: include/mgmake/build/toolchain.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx

#include <cstddef>
#include <filesystem>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace mgmake::lower {
	template<typename target_t>
	inline std::vector<dag::artifact::id> context::lower_objects(
		const target_t& target,
		const std::set<std::filesystem::path>& include_dirs
	) {
		const auto& tc = toolchain();

		std::vector<dag::artifact::id> object_ids{};
		object_ids.reserve(target.m_sources.size());

		std::size_t source_index = 0;

		for (const auto& source : target.m_sources) {
			auto source_id = m_emit.source(source);
			const std::string_view object_extension =
				tc.dialect() == build::toolchain::dialect::msvc ? ".obj" : ".o";

			std::filesystem::path object_path =
				request().build_dir() /
				"obj" /
				target.m_name /
				(std::to_string(source_index++) + std::string{ object_extension });

			auto object_id = m_emit.generated(object_path);

			sys::command_line command{};
			command.m_args.emplace_back(tc.cxx());

			build::append_target_args(command, tc, request());

			for (const auto& flag : tc.compile_flags()) {
				command.m_args.emplace_back(flag);
			}

			for (const auto& flag : tc.cxx_flags()) {
				command.m_args.emplace_back(flag);
			}

			for (const auto& include_dir : include_dirs) {
				switch (tc.dialect()) {
					case build::toolchain::dialect::gcc:
						command.m_args.emplace_back(std::string{"-I"} + include_dir.string());
						break;

					case build::toolchain::dialect::msvc:
						command.m_args.emplace_back(std::string{"/I"} + include_dir.string());
						break;
				}
			}

			switch (tc.dialect()) {
				case build::toolchain::dialect::gcc:
					command.m_args.emplace_back("-c");
					command.m_args.emplace_back(source.string());
					command.m_args.emplace_back("-o");
					command.m_args.emplace_back(object_path.string());
					break;

				case build::toolchain::dialect::msvc:
					command.m_args.emplace_back("/c");
					command.m_args.emplace_back(source.string());
					command.m_args.emplace_back(std::string{"/Fo"} + object_path.string());
					break;
			}

			m_emit.action(
				std::string{"Compile "} + source.string(),
				std::string{"Compiles source file '"} + source.string() + "' for target '" + target.m_name + "'.",
				{ source_id },
				{ object_id },
				std::move(command)
			);

			object_ids.emplace_back(object_id);
		}

		return object_ids;
	}
}

#endif
// ===== end include/mgmake/lower/objects.hxx =====


// ===== begin include/mgmake/lower/context_impl.hxx =====
#pragma once

#ifndef MGMK_LOWER_CONTEXT_IMPL_HXX
#define MGMK_LOWER_CONTEXT_IMPL_HXX

// skipped duplicate include: include/mgmake/lower/context.hxx
// skipped duplicate include: include/mgmake/lower/objects.hxx
// skipped duplicate include: include/mgmake/build/artifact_names.hxx
// skipped duplicate include: include/mgmake/build/target.hxx
// skipped duplicate include: include/mgmake/detail/assert.hxx
// skipped duplicate include: include/mgmake/spec/project.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx

#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::lower {
	inline context::context(
		dag::graph& graph,
		const build::request& req,
		const spec::project& project
	)
		: m_req{req}
		, m_project{project}
		, m_emit{graph}
		, m_libraries(project.m_libraries.size()) {}

	inline lower::usage context::use_libraries(
		const std::set<std::string>& libraries,
		std::string_view owner_name
	) {
		lower::usage result{};

		for (const auto& library_name : libraries) {
			const auto linked_id = m_project.find_library(library_name);

			mgmkassert(
				linked_id.has_value(),
				"mgmake lower: target '" + std::string{owner_name} +
					"' links unknown library '" + library_name + "'"
			);

			const lower::target& dep = lower_library(linked_id.value());

			if (dep.m_dag_target.has_value()) {
				result.m_dag_dependencies.emplace(dep.m_dag_target.value());
			}

			result.m_link_inputs.insert(
				result.m_link_inputs.end(),
				dep.m_linkable_artifacts.begin(),
				dep.m_linkable_artifacts.end()
			);

			result.m_include_dirs.insert_range(dep.m_include_dirs);
		}

		return result;
	}

	inline const lower::target& context::lower_library(spec::library::id id) {
		mgmkassert(
			id < m_libraries.size(),
			"mgmake lower: invalid library id"
		);

		if (m_libraries.at(id).has_value()) {
			return m_libraries.at(id).value();
		}

		const auto& lib = m_project.m_libraries.at(id);

		mgmkassert(not lib.m_name.empty(), "mgmake lower: library target has no name");

		mgmkassert(
			not m_active_libraries.contains(id),
			"mgmake lower: cyclic library dependency involving '" + lib.m_name + "'"
		);

		m_active_libraries.emplace(id);

		lower::usage usage = use_libraries(
			lib.linked_libraries(),
			lib.m_name
		);

		switch (lib.m_kind) {
			case spec::library::kind::interface:
				m_libraries.at(id) = lower_interface_library(lib, std::move(usage));
				break;

			case spec::library::kind::static_lib:
				m_libraries.at(id) = lower_static_library(lib, std::move(usage));
				break;

			case spec::library::kind::shared_lib:
				m_libraries.at(id) = lower_shared_library(lib, std::move(usage));
				break;
		}

		m_active_libraries.erase(id);

		return m_libraries.at(id).value();
	}

	inline lower::target context::lower_interface_library(
		const spec::library& lib,
		lower::usage usage
	) {
		mgmkassert(
			lib.m_sources.empty(),
			"mgmake lower: interface library '" + lib.m_name + "' cannot have sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);
		auto link_inputs = std::move(usage.m_link_inputs);

		dag::target dag_target{
			lib.m_name,
			{},
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts = std::move(link_inputs);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline lower::target context::lower_static_library(
		const spec::library& lib,
		lower::usage usage
	) {
		const auto& tc = toolchain();

		mgmkassert(
			not lib.m_sources.empty(),
			"mgmake lower: static library '" + lib.m_name + "' has no sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(lib, include_dirs);

		std::filesystem::path archive_path;

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc:
				archive_path = request().build_dir() / "lib" / ("lib" + lib.m_name + ".a");
				break;

			case build::toolchain::dialect::msvc:
				archive_path = request().build_dir() / "lib" / (lib.m_name + ".lib");
				break;
		}

		auto archive_id = m_emit.generated(archive_path);

		sys::command_line command{};
		command.m_args.emplace_back(tc.ar());

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc: {
				if (tc.archive_flags().empty()) {
					command.m_args.emplace_back("rcs");
				}
				else {
					for (const auto& flag : tc.archive_flags()) {
						command.m_args.emplace_back(flag);
					}
				}

				command.m_args.emplace_back(archive_path.string());

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(m_emit.path(object_id).string());
				}

				break;
			}

			case build::toolchain::dialect::msvc: {
				for (const auto& flag : tc.archive_flags()) {
					command.m_args.emplace_back(flag);
				}

				command.m_args.emplace_back(std::string{"/OUT:"} + archive_path.string());

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(m_emit.path(object_id).string());
				}

				break;
			}
		}

		m_emit.action(
			std::string{"Build static library "} + lib.m_name,
			std::string{"Builds static library target '"} + lib.m_name + "'.",
			object_ids,
			{ archive_id },
			std::move(command)
		);

		dag::target dag_target{
			lib.m_name,
			{ archive_id },
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts.emplace_back(archive_id);
		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline lower::target context::lower_shared_library(
		const spec::library& lib,
		lower::usage usage
	) {
		const auto& tc = toolchain();

		mgmkassert(
			not lib.m_sources.empty(),
			"mgmake lower: shared library '" + lib.m_name + "' has no sources"
		);

		mgmkassert(
			tc.dialect() == build::toolchain::dialect::gcc,
			"mgmake lower: shared library lowering is currently only implemented for GCC-like toolchains"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(lib, include_dirs);

		const auto platform = request().target_platform();
		std::filesystem::path shared_path =
			request().build_dir() /
			"lib" /
			(
				std::string{ build::shared_library_prefix(platform) } +
				lib.m_name +
				std::string{ build::shared_library_extension(platform) }
			);

		auto shared_id = m_emit.generated(shared_path);

		sys::command_line command{};
		command.m_args.emplace_back(tc.linker());

		build::append_target_args(command, tc, request());

		const auto shared_flag = build::shared_library_link_flag(platform);

		mgmkassert(
			!shared_flag.empty(),
			"mgmake lower: shared library lowering is not supported for requested target platform"
		);

		command.m_args.emplace_back(shared_flag);

		for (auto object_id : object_ids) {
			command.m_args.emplace_back(m_emit.path(object_id).string());
		}

		for (auto link_input : usage.m_link_inputs) {
			command.m_args.emplace_back(m_emit.path(link_input).string());
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		command.m_args.emplace_back("-o");
		command.m_args.emplace_back(shared_path.string());

		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_link_inputs.begin(), usage.m_link_inputs.end());

		m_emit.action(
			std::string{"Build shared library "} + lib.m_name,
			std::string{"Builds shared library target '"} + lib.m_name + "'.",
			std::move(inputs),
			{ shared_id },
			std::move(command)
		);

		dag::target dag_target{
			lib.m_name,
			{ shared_id },
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts.emplace_back(shared_id);
		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline void context::lower_executable(spec::executable::id id) {
		mgmkassert(
			id < m_project.m_executables.size(),
			"mgmake lower: invalid executable id"
		);

		const auto& exe = m_project.m_executables.at(id);
		const auto& tc = toolchain();

		mgmkassert(not exe.m_name.empty(), "mgmake lower: executable target has no name");
		mgmkassert(
			not exe.m_sources.empty(),
			"mgmake lower: executable target '" + exe.m_name + "' has no sources"
		);

		lower::usage usage = use_libraries(
			exe.linked_libraries(),
			exe.m_name
		);

		auto include_dirs = exe.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(exe, include_dirs);
		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_link_inputs.begin(), usage.m_link_inputs.end());

		std::filesystem::path output =
			request().build_dir() /
			(
				exe.m_name +
				std::string{ build::executable_extension(request().target_platform()) }
			);

		auto output_id = m_emit.generated(output);

		sys::command_line command{};
		command.m_args.emplace_back(tc.cxx());

		build::append_target_args(command, tc, request());

		for (auto object_id : object_ids) {
			command.m_args.emplace_back(m_emit.path(object_id).string());
		}

		for (auto link_input : usage.m_link_inputs) {
			command.m_args.emplace_back(m_emit.path(link_input).string());
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc:
				command.m_args.emplace_back("-o");
				command.m_args.emplace_back(output.string());
				break;

			case build::toolchain::dialect::msvc:
				command.m_args.emplace_back(std::string{"/Fe"} + output.string());
				break;
		}

		m_emit.action(
			std::string{"Build executable "} + exe.m_name,
			std::string{"Builds executable target '"} + exe.m_name + "'.",
			std::move(inputs),
			{ output_id },
			std::move(command)
		);

		dag::target dag_target{
			exe.m_name,
			{ output_id },
			std::move(usage.m_dag_dependencies)
		};

		m_emit.target(std::move(dag_target));
	}
}

#endif
// ===== end include/mgmake/lower/context_impl.hxx =====


// ===== begin include/mgmake/spec/project_impl.hxx =====
#pragma once

#ifndef MGMK_SPEC_PROJECT_IMPL_HXX
#define MGMK_SPEC_PROJECT_IMPL_HXX

// skipped duplicate include: include/mgmake/spec/project.hxx
// skipped duplicate include: include/mgmake/lower/context_impl.hxx

namespace mgmake::spec {
	inline dag::graph project::graph(const build::request& req) const {
		dag::graph result{};
		lower::context ctx{result, req, *this};

		for (spec::library::id id = 0; id < m_libraries.size(); ++id) {
			ctx.lower_library(id);
		}

		for (spec::executable::id id = 0; id < m_executables.size(); ++id) {
			ctx.lower_executable(id);
		}

		return result;
	}
}

#endif
// ===== end include/mgmake/spec/project_impl.hxx =====


namespace mgmake {
	int entry(const sys::command_line& command_line) {
		auto parsed = cli::parse(command_line.user_args());

		if (!parsed) {
			std::println(stderr, "mgmake: error: {}", parsed.m_error);
			std::println(stderr, "try '{} help'", command_line.program_name());
			return 2;
		}

		const cli::options& opts = parsed.m_value;

		if (opts.m_show_help) {
			cli::print_help(command_line.program_name());
			return 0;
		}

		std::println("action: {}", cli::action_name(opts.m_action));
		std::println("backend: {}", cli::backend_name(opts.m_backend));
		std::println("build dir: {}", opts.m_build_dir);

		for (const auto& target : opts.m_targets) {
			std::println("target: {}", target);
		}

		return 0;
	}
}
namespace mgmk = mgmake;

#if defined(MGMK_PLATFORM_WINDOWS) and defined(WIN32_LEAN_AND_MEAN)
#define MGMAKE_BUILD_ENTRY(ProjectType) \
int wmain(int argc, wchar_t** argv) { \
    auto args = ::mgmk::sys::args_from_wide(argc, argv); \
    return ::mgmk::entry<ProjectType>(args); \
}
#else
#define MGMAKE_BUILD_ENTRY(ProjectType) \
int main(int argc, char** argv) { \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv); \
    return ::mgmk::entry<ProjectType>(args); \
}
#endif

// Short-hand
#define MGMK_BUILD_ENTRY MGMAKE_BUILD_ENTRY
#define MGMK_ENTRY MGMK_BUILD_ENTRY

#endif
// ===== end include/mgmake/mgmake.hxx =====


#endif // MGMAKE_SINGLE_HEADER_HXX
