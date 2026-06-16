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

#include <cstdlib>
#include <iostream>
#include <source_location>
#include <string_view>

namespace mgmake::detail {

[[noreturn]] inline void assertion_failed(
    std::string_view condition,
    std::string_view message,
    std::source_location location = std::source_location::current()
) {
    std::cerr
        << "mgmake assertion failed\n"
        << "  condition: " << condition << "\n"
        << "  message: " << message << "\n"
        << "  file: " << location.file_name() << "\n"
        << "  line: " << location.line() << "\n"
        << "  function: " << location.function_name() << "\n";

    std::abort();
}

inline void mgmk_assert_impl(
    const bool condition,
    const std::string_view condition_text,
    const std::string_view message,
    const std::source_location location = std::source_location::current()
) {
    if (!condition) {
        assertion_failed(condition_text, message, location);
    }
}

struct constexpr_assertion_failure {};

template<typename message_t>
[[noreturn]] consteval void constexpr_assert_failed(
    const char*,
    const message_t&
) {
    throw constexpr_assertion_failure{};
}

} // namespace mgmake::detail

#define mgmkstaticassert(condition, message) \
    static_assert(static_cast<bool>(condition), message)

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
            if (!(condition)) {                                                 \
                if consteval {                                                  \
                    ::mgmake::detail::constexpr_assert_failed(                  \
                        #condition,                                             \
                        message                                                 \
                    );                                                          \
                } else {                                                        \
                    ::mgmake::detail::mgmk_assert_impl(                         \
                        false,                                                  \
                        #condition,                                             \
                        message,                                                \
                        std::source_location::current()                         \
                    );                                                          \
                }                                                               \
            }                                                                   \
        } while (false)
#else
    #define mgmkassert(condition, message)                                      \
        do {                                                                    \
            (void)sizeof(condition);                                            \
        } while (false)
#endif

#endif
// ===== end include/mgmake/detail/assert.hxx =====


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

	struct target {
		arch m_arch = arch::unknown;
		platform m_platform = platform::p_unknown;
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
		detail::enum_entry<platform::p_unknown, "unknown">,
		detail::enum_entry<platform::p_windows, "windows">,
		detail::enum_entry<platform::p_linux, "linux">,
		detail::enum_entry<platform::p_macos, "macos">,
		detail::enum_entry<platform::p_wasm, "wasm">,
		detail::enum_entry<platform::p_freestanding, "freestanding">,
		detail::enum_entry<platform::p_other_posix, "other-posix">
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
		detail::enum_entry<platform::p_unknown, "unknown">,
		detail::enum_entry<platform::p_windows, "windows">,
		detail::enum_entry<platform::p_windows, "win">,
		detail::enum_entry<platform::p_windows, "win32">,
		detail::enum_entry<platform::p_windows, "win64">,
		detail::enum_entry<platform::p_linux, "linux">,
		detail::enum_entry<platform::p_macos, "macos">,
		detail::enum_entry<platform::p_macos, "mac">,
		detail::enum_entry<platform::p_macos, "darwin">,
		detail::enum_entry<platform::p_macos, "osx">,
		detail::enum_entry<platform::p_wasm, "wasm">,
		detail::enum_entry<platform::p_wasm, "webassembly">,
		detail::enum_entry<platform::p_wasm, "emscripten">,
		detail::enum_entry<platform::p_freestanding, "freestanding">,
		detail::enum_entry<platform::p_freestanding, "none">,
		detail::enum_entry<platform::p_other_posix, "posix">,
		detail::enum_entry<platform::p_other_posix, "unix">,
		detail::enum_entry<platform::p_other_posix, "other-posix">
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
		detail::enum_entry<platform::p_unknown, "unknown">,
		detail::enum_entry<platform::p_windows, "windows">,
		detail::enum_entry<platform::p_linux, "linux">,
		detail::enum_entry<platform::p_macos, "darwin">,
		detail::enum_entry<platform::p_wasm, "unknown">,
		detail::enum_entry<platform::p_freestanding, "none">,
		detail::enum_entry<platform::p_other_posix, "unknown">
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
			if (
				ch == ' '
				|| ch == '\t'
				|| ch == '"'
				|| ch == '&'
				|| ch == '|'
				|| ch == '<'
				|| ch == '>'
				|| ch == '^'
				|| ch == '%'
				|| ch == '!'
				|| ch == '('
				|| ch == ')'
			) {
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

#endif
// ===== end include/mgmake/sys/util.hxx =====


// ===== begin include/mgmake/sys/command_line.hxx =====
#pragma once

#ifndef MGMAKE_SYS_COMMAND_LINE_HXX
#define MGMAKE_SYS_COMMAND_LINE_HXX

// skipped duplicate include: include/mgmake/detail/convert.hxx
// skipped duplicate include: include/mgmake/sys/util.hxx

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
			const auto command = full_command();

#if defined(MGMK_PLATFORM_WINDOWS)
			std::string shell_command;
			shell_command.reserve(command.size() + 2);
			shell_command += '"';
			shell_command += command;
			shell_command += '"';
			return std::system(shell_command.c_str());
#else
			return std::system(command.c_str());
#endif
		}
	};

	inline constexpr command_line args_from_utf8(int argc, const char* const* argv) {
		command_line result;

		if (argc <= 0 || argv == nullptr) {
			return result;
		}

		result.m_args.reserve(static_cast<std::size_t>(argc));

		for (int i = 0; i < argc; ++i) {
			result.m_args.emplace_back(argv[i] ? argv[i] : "");
		}

		return result;
	}

#ifdef MGMK_INCLUDED_WINDOWS
	inline constexpr command_line args_from_wide(int argc, const wchar_t* const* argv) {
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

#endif
// ===== end include/mgmake/sys/command_line.hxx =====


// ===== begin include/mgmake/discovery/tool_role.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_ROLE_HXX
#define MGMAKE_DISCOVERY_TOOL_ROLE_HXX

// skipped duplicate include: include/mgmake/detail/enum_string.hxx

#include <string_view>

namespace mgmake::discovery {
	enum struct tool_role {
		c_compiler,
		cxx_compiler,
		objc_compiler,
		objcxx_compiler,
		assembler,
		cuda_compiler,
		hip_compiler,
		resource_compiler,
		midl_compiler,
		archiver,
		ranlib,
		librarian,
		linker,
		shared_linker,
		dll_tool,
		manifest_tool,
		strip,
		objcopy,
		objdump,
		nm,
		readelf,
		debug_symbol_tool,
		lipo,
		install_name_tool,
		codesign,
		generator_ninja,
		generator_make,
		generator_msbuild,
		generator_xcode,
		cmake,
		pkg_config,
		exe_wrapper,
		emulator,
		count
	};

	using tool_role_names = detail::enum_table<
		tool_role,
		detail::enum_entry<tool_role::c_compiler, "c compiler">,
		detail::enum_entry<tool_role::cxx_compiler, "c++ compiler">,
		detail::enum_entry<tool_role::objc_compiler, "objective-c compiler">,
		detail::enum_entry<tool_role::objcxx_compiler, "objective-c++ compiler">,
		detail::enum_entry<tool_role::assembler, "assembler">,
		detail::enum_entry<tool_role::cuda_compiler, "cuda compiler">,
		detail::enum_entry<tool_role::hip_compiler, "hip compiler">,
		detail::enum_entry<tool_role::resource_compiler, "resource compiler">,
		detail::enum_entry<tool_role::midl_compiler, "midl compiler">,
		detail::enum_entry<tool_role::archiver, "archiver">,
		detail::enum_entry<tool_role::ranlib, "ranlib">,
		detail::enum_entry<tool_role::librarian, "librarian">,
		detail::enum_entry<tool_role::linker, "linker">,
		detail::enum_entry<tool_role::shared_linker, "shared linker">,
		detail::enum_entry<tool_role::dll_tool, "dll tool">,
		detail::enum_entry<tool_role::manifest_tool, "manifest tool">,
		detail::enum_entry<tool_role::strip, "strip">,
		detail::enum_entry<tool_role::objcopy, "objcopy">,
		detail::enum_entry<tool_role::objdump, "objdump">,
		detail::enum_entry<tool_role::nm, "nm">,
		detail::enum_entry<tool_role::readelf, "readelf">,
		detail::enum_entry<tool_role::debug_symbol_tool, "debug symbol tool">,
		detail::enum_entry<tool_role::lipo, "lipo">,
		detail::enum_entry<tool_role::install_name_tool, "install name tool">,
		detail::enum_entry<tool_role::codesign, "codesign">,
		detail::enum_entry<tool_role::generator_ninja, "ninja">,
		detail::enum_entry<tool_role::generator_make, "make">,
		detail::enum_entry<tool_role::generator_msbuild, "msbuild">,
		detail::enum_entry<tool_role::generator_xcode, "xcodebuild">,
		detail::enum_entry<tool_role::cmake, "cmake">,
		detail::enum_entry<tool_role::pkg_config, "pkg-config">,
		detail::enum_entry<tool_role::exe_wrapper, "exe wrapper">,
		detail::enum_entry<tool_role::emulator, "emulator">
	>;

	static_assert(tool_role_names::is_zero_based_count_canonical(tool_role::count));

	[[nodiscard]] inline constexpr std::string_view name(tool_role role) noexcept {
		return tool_role_names::to_string(role);
	}
}

#endif
// ===== end include/mgmake/discovery/tool_role.hxx =====


// ===== begin include/mgmake/discovery/tool_provider.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_PROVIDER_HXX
#define MGMAKE_DISCOVERY_TOOL_PROVIDER_HXX

// skipped duplicate include: include/mgmake/detail/enum_string.hxx

#include <string_view>

namespace mgmake::discovery {
	enum struct tool_provider {
		explicit_path,
		cli_override,
		environment_override,
		cache,
		sibling,
		target_prefix,
		path,
		known_install_root,
		toolchain_root,
		sdk_root,
		sysroot_root,
		project_toolchain_root,
		package_toolchain,
		windows_registry,
		vswhere,
		visual_studio,
		visual_studio_llvm,
		visual_studio_environment,
		windows_sdk,
		standalone_llvm,
		msys2,
		mingw,
		cygwin,
		xcrun,
		xcode,
		homebrew,
		macports,
		unix_system,
		distro_llvm,
		distro_gcc,
		android_ndk,
		emscripten_sdk,
		embedded_sdk,
		compatible_fallback,
		auto_fallback,
		count
	};

	using tool_provider_names = detail::enum_table<
		tool_provider,
		detail::enum_entry<tool_provider::explicit_path, "explicit path">,
		detail::enum_entry<tool_provider::cli_override, "CLI override">,
		detail::enum_entry<tool_provider::environment_override, "environment override">,
		detail::enum_entry<tool_provider::cache, "cache">,
		detail::enum_entry<tool_provider::sibling, "sibling">,
		detail::enum_entry<tool_provider::target_prefix, "target prefix">,
		detail::enum_entry<tool_provider::path, "PATH">,
		detail::enum_entry<tool_provider::known_install_root, "known install root">,
		detail::enum_entry<tool_provider::toolchain_root, "toolchain root">,
		detail::enum_entry<tool_provider::sdk_root, "SDK root">,
		detail::enum_entry<tool_provider::sysroot_root, "sysroot root">,
		detail::enum_entry<tool_provider::project_toolchain_root, "project toolchain root">,
		detail::enum_entry<tool_provider::package_toolchain, "package toolchain">,
		detail::enum_entry<tool_provider::windows_registry, "Windows registry">,
		detail::enum_entry<tool_provider::vswhere, "vswhere">,
		detail::enum_entry<tool_provider::visual_studio, "Visual Studio">,
		detail::enum_entry<tool_provider::visual_studio_llvm, "Visual Studio LLVM">,
		detail::enum_entry<tool_provider::visual_studio_environment, "Visual Studio environment">,
		detail::enum_entry<tool_provider::windows_sdk, "Windows SDK">,
		detail::enum_entry<tool_provider::standalone_llvm, "standalone LLVM">,
		detail::enum_entry<tool_provider::msys2, "MSYS2">,
		detail::enum_entry<tool_provider::mingw, "MinGW">,
		detail::enum_entry<tool_provider::cygwin, "Cygwin">,
		detail::enum_entry<tool_provider::xcrun, "xcrun">,
		detail::enum_entry<tool_provider::xcode, "Xcode">,
		detail::enum_entry<tool_provider::homebrew, "Homebrew">,
		detail::enum_entry<tool_provider::macports, "MacPorts">,
		detail::enum_entry<tool_provider::unix_system, "Unix system">,
		detail::enum_entry<tool_provider::distro_llvm, "distro LLVM">,
		detail::enum_entry<tool_provider::distro_gcc, "distro GCC">,
		detail::enum_entry<tool_provider::android_ndk, "Android NDK">,
		detail::enum_entry<tool_provider::emscripten_sdk, "Emscripten SDK">,
		detail::enum_entry<tool_provider::embedded_sdk, "embedded SDK">,
		detail::enum_entry<tool_provider::compatible_fallback, "compatible fallback">,
		detail::enum_entry<tool_provider::auto_fallback, "auto fallback">
	>;

	static_assert(tool_provider_names::is_zero_based_count_canonical(tool_provider::count));

	[[nodiscard]] inline constexpr std::string_view name(tool_provider provider) noexcept {
		return tool_provider_names::to_string(provider);
	}
}

#endif
// ===== end include/mgmake/discovery/tool_provider.hxx =====


// ===== begin include/mgmake/discovery/mode.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_MODE_HXX
#define MGMAKE_DISCOVERY_MODE_HXX

// skipped duplicate include: include/mgmake/detail/enum_string.hxx

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
// ===== end include/mgmake/discovery/mode.hxx =====


// ===== begin include/mgmake/discovery/tool_family.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_FAMILY_HXX
#define MGMAKE_DISCOVERY_TOOL_FAMILY_HXX

// skipped duplicate include: include/mgmake/detail/enum_string.hxx

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

#endif
// ===== end include/mgmake/discovery/tool_family.hxx =====


// ===== begin include/mgmake/discovery/tool_binding.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_BINDING_HXX
#define MGMAKE_DISCOVERY_TOOL_BINDING_HXX

// skipped duplicate include: include/mgmake/discovery/tool_role.hxx

#include <string>

namespace mgmake::discovery {
	struct tool_binding {
		tool_role m_role{};
		std::string m_name{};
	};
}

#endif
// ===== end include/mgmake/discovery/tool_binding.hxx =====


// ===== begin include/mgmake/discovery/resolved_tool.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_RESOLVED_TOOL_HXX
#define MGMAKE_DISCOVERY_RESOLVED_TOOL_HXX

// skipped duplicate include: include/mgmake/discovery/tool_family.hxx
// skipped duplicate include: include/mgmake/discovery/tool_provider.hxx
// skipped duplicate include: include/mgmake/discovery/tool_role.hxx

#include <filesystem>
#include <optional>
#include <string>

namespace mgmake::discovery {
	struct tool_candidate {
		tool_role m_role{};
		std::string m_logical_name{};
		std::filesystem::path m_path{};
		tool_provider m_provider{};
		std::string m_reason{};
		int m_priority = 0;
		bool m_authoritative = false;
		std::optional<std::filesystem::path> m_provider_root{};
	};

	struct rejected_tool_candidate {
		tool_candidate m_candidate{};
		std::string m_reason{};
	};

	struct resolved_tool {
		tool_role m_role{};
		std::string m_logical_name{};
		std::filesystem::path m_path{};
		tool_provider m_provider{};
		tool_family m_family = tool_family::unknown;
		linker_flavor m_linker_flavor = linker_flavor::unknown;
		object_format m_object_format = object_format::unknown;
		std::string m_version{};
		std::string m_target_triple{};
		std::string m_reason{};
		std::optional<std::filesystem::path> m_provider_root{};

		[[nodiscard]] inline const std::filesystem::path& path() const noexcept {
			return m_path;
		}

		[[nodiscard]] inline std::string path_string() const {
#if defined(MGMK_PLATFORM_WINDOWS)
			return m_path.generic_string();
#else
			return m_path.string();
#endif
		}
	};
}

#endif
// ===== end include/mgmake/discovery/resolved_tool.hxx =====


// ===== begin include/mgmake/discovery/tool_environment.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_ENVIRONMENT_HXX
#define MGMAKE_DISCOVERY_TOOL_ENVIRONMENT_HXX

// skipped duplicate include: include/mgmake/sys/util.hxx

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mgmake::discovery {
#if defined(_WIN32)
	[[nodiscard]] inline std::string cmd_call_quote(std::string_view text) {
		std::string result;
		result += "\"\"";

		for (const char ch : text) {
			if (ch == '"') {
				result += "\"\"";
			} else {
				result += ch;
			}
		}

		result += "\"\"";
		return result;
	}
#endif

	struct environment_variable {
		std::string m_name{};
		std::string m_value{};
	};

	struct tool_environment {
		std::vector<std::filesystem::path> m_path_prepend{};
		std::vector<std::filesystem::path> m_path_append{};
		std::vector<environment_variable> m_variables{};
		std::optional<std::filesystem::path> m_setup_script{};
		std::vector<std::string> m_setup_args{};
		std::string m_fingerprint{};

		[[nodiscard]] inline bool empty() const noexcept {
			return m_path_prepend.empty()
				&& m_path_append.empty()
				&& m_variables.empty()
				&& !m_setup_script.has_value();
		}
	};

	[[nodiscard]] inline std::string wrap_command_for_environment(
		const tool_environment& env,
		std::string command
	) {
		if (env.empty()) {
			return command;
		}

		if (env.m_setup_script.has_value()) {
			std::string result;
#if defined(_WIN32)
			result += "cmd /s /c \"call ";
			result += cmd_call_quote(env.m_setup_script->string());
#else
			result += ". ";
			result += sys::shell_escape(env.m_setup_script->string());
#endif
			for (const auto& arg : env.m_setup_args) {
				result += ' ';
				result += sys::shell_escape(arg);
			}

#if defined(_WIN32)
			result += " >nul && ";
			result += command;
			result += "\"";
#else
			result += " && ";
			result += command;
#endif
			return result;
		}

		std::string prefix;
#if !defined(_WIN32)
		for (const auto& variable : env.m_variables) {
			prefix += variable.m_name;
			prefix += '=';
			prefix += sys::shell_escape(variable.m_value);
			prefix += ' ';
		}
#endif
		return prefix + command;
	}
}

#endif
// ===== end include/mgmake/discovery/tool_environment.hxx =====


// ===== begin include/mgmake/build/toolchain.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_TOOLCHAIN_HXX
#define MGMAKE_BUILD_TOOLCHAIN_HXX

// skipped duplicate include: include/mgmake/discovery/mode.hxx
// skipped duplicate include: include/mgmake/discovery/tool_binding.hxx
// skipped duplicate include: include/mgmake/discovery/tool_role.hxx
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
        std::vector<discovery::tool_binding> m_tools{};

        std::optional<std::string> m_version{};
        std::optional<std::string> m_sdk_root{};
        std::optional<std::string> m_toolchain_root{};
        std::optional<std::string> m_package_root{};
        std::vector<std::string> m_search_roots{};
        discovery::mode m_discovery_mode = discovery::mode::exact;

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

        inline constexpr auto& tool(discovery::tool_role role, std::string name) {
            for (auto& binding : m_tools) {
                if (binding.m_role == role) {
                    binding.m_name = std::move(name);
                    return *this;
                }
            }

            m_tools.push_back(discovery::tool_binding{
                .m_role = role,
                .m_name = std::move(name)
            });

            return *this;
        }

        [[nodiscard]] inline constexpr std::string_view tool(
            discovery::tool_role role
        ) const noexcept {
            for (const auto& binding : m_tools) {
                if (binding.m_role == role) {
                    return binding.m_name;
                }
            }

            switch (role) {
                case discovery::tool_role::c_compiler:
                    return m_cc;

                case discovery::tool_role::cxx_compiler:
                    return m_cxx;

                case discovery::tool_role::archiver:
                    return m_ar;

                case discovery::tool_role::linker:
                    return m_linker;

                default:
                    return {};
            }
        }

        inline constexpr auto& assembler(std::string name) {
            return tool(discovery::tool_role::assembler, std::move(name));
        }

        inline constexpr auto& ranlib(std::string name) {
            return tool(discovery::tool_role::ranlib, std::move(name));
        }

        inline constexpr auto& librarian(std::string name) {
            return tool(discovery::tool_role::librarian, std::move(name));
        }

        inline constexpr auto& shared_linker(std::string name) {
            return tool(discovery::tool_role::shared_linker, std::move(name));
        }

        inline constexpr auto& resource_compiler(std::string name) {
            return tool(discovery::tool_role::resource_compiler, std::move(name));
        }

        inline constexpr auto& manifest_tool(std::string name) {
            return tool(discovery::tool_role::manifest_tool, std::move(name));
        }

        inline constexpr auto& dll_tool(std::string name) {
            return tool(discovery::tool_role::dll_tool, std::move(name));
        }

        inline constexpr auto& strip(std::string name) {
            return tool(discovery::tool_role::strip, std::move(name));
        }

        inline constexpr auto& objcopy(std::string name) {
            return tool(discovery::tool_role::objcopy, std::move(name));
        }

        inline constexpr auto& objdump(std::string name) {
            return tool(discovery::tool_role::objdump, std::move(name));
        }

        inline constexpr auto& nm(std::string name) {
            return tool(discovery::tool_role::nm, std::move(name));
        }

        inline constexpr auto& readelf(std::string name) {
            return tool(discovery::tool_role::readelf, std::move(name));
        }

        inline constexpr auto& cmake(std::string name) {
            return tool(discovery::tool_role::cmake, std::move(name));
        }

        inline constexpr auto& pkg_config(std::string name) {
            return tool(discovery::tool_role::pkg_config, std::move(name));
        }

        inline constexpr auto& discovery_mode(discovery::mode value) noexcept {
            m_discovery_mode = value;
            return *this;
        }

        [[nodiscard]] inline constexpr discovery::mode discovery_mode() const noexcept {
            return m_discovery_mode;
        }

        inline constexpr auto& sdk_root(std::string path) {
            m_sdk_root = std::move(path);
            return *this;
        }

        inline constexpr auto& toolchain_root(std::string path) {
            m_toolchain_root = std::move(path);
            return *this;
        }

        inline constexpr auto& package_root(std::string path) {
            m_package_root = std::move(path);
            return *this;
        }

        inline constexpr auto& add_search_root(std::string path) {
            m_search_roots.emplace_back(std::move(path));
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

    inline const auto tc_clang_mg = build::toolchain{"clang-mg"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("clang-mg")
        .cxx("clang-mg++")
        .ar("llvm-ar")
        .ranlib("llvm-ranlib")
        .linker("clang-mg++")
        .shared_linker("clang-mg++")
        .resource_compiler("llvm-rc")
        .strip("llvm-strip")
        .objcopy("llvm-objcopy")
        .objdump("llvm-objdump")
        .nm("llvm-nm")
        .readelf("llvm-readelf")
        .target_selection(build::toolchain::target_mode::clang_target)
        .discovery_mode(discovery::mode::family_fallback);

    inline const auto tc_clang = build::toolchain{"clang"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("clang")
        .cxx("clang++")
        .ar("llvm-ar")
        .ranlib("llvm-ranlib")
        .linker("clang++")
        .shared_linker("clang++")
        .resource_compiler("llvm-rc")
        .strip("llvm-strip")
        .objcopy("llvm-objcopy")
        .objdump("llvm-objdump")
        .nm("llvm-nm")
        .readelf("llvm-readelf")
        .target_selection(build::toolchain::target_mode::clang_target)
        .discovery_mode(discovery::mode::family_fallback);

    inline const auto tc_gcc = build::toolchain{"gcc"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("gcc")
        .cxx("g++")
        .ar("ar")
        .ranlib("ranlib")
        .linker("g++")
        .shared_linker("g++")
        .resource_compiler("windres")
        .strip("strip")
        .objcopy("objcopy")
        .objdump("objdump")
        .nm("nm")
        .readelf("readelf")
        .discovery_mode(discovery::mode::family_fallback);
    
    inline const auto tc_msvc = build::toolchain{"msvc"}
        .dialect(build::toolchain::dialect::msvc)
        .cc("cl")
        .cxx("cl")
        .ar("lib")
        .librarian("lib")
        .linker("link")
        .shared_linker("link")
        .resource_compiler("rc")
        .manifest_tool("mt")
        .tool(discovery::tool_role::debug_symbol_tool, "mspdbsrv")
        .discovery_mode(discovery::mode::family_fallback);
}

#endif
// ===== end include/mgmake/build/toolchain.hxx =====


// ===== begin include/mgmake/build/toolchain_registry.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_TOOLCHAIN_REGISTRY_HXX
#define MGMAKE_BUILD_TOOLCHAIN_REGISTRY_HXX

// skipped duplicate include: include/mgmake/build/toolchain.hxx

#include <concepts>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::build {
	struct toolchain_registry {
		std::vector<toolchain> m_toolchains{};

		constexpr toolchain_registry() = default;

		constexpr toolchain_registry(std::initializer_list<toolchain> toolchains)
			: m_toolchains{ toolchains } {}

		constexpr explicit toolchain_registry(std::vector<toolchain> toolchains)
			: m_toolchains{ std::move(toolchains) } {}

		[[nodiscard]] constexpr const toolchain* find(std::string_view name) const noexcept {
			for (const auto& tc : m_toolchains) {
				if (tc.m_name == name) {
					return &tc;
				}
			}

			return nullptr;
		}

		[[nodiscard]] constexpr toolchain_registry emplace(toolchain tc) const {
			auto result = *this;
			result.m_toolchains.emplace_back(std::move(tc));
			return result;
		}

		[[nodiscard]] constexpr std::string choices_string() const {
			std::string result;

			for (std::size_t i = 0; i < m_toolchains.size(); ++i) {
				if (i != 0) {
					result += ", ";
				}

				result += m_toolchains[i].m_name;
			}

			return result;
		}
	};

	template <typename Registry>
	concept toolchain_registry_like =
		requires(const Registry& registry, std::string_view name) {
			{ registry.find(name) } -> std::same_as<const toolchain*>;
			{ registry.choices_string() } -> std::convertible_to<std::string>;
		};

	inline const auto default_toolchains = toolchain_registry{
		tc_clang_mg,
		tc_clang,
		tc_gcc,
		tc_msvc
	};
}

#endif
// ===== end include/mgmake/build/toolchain_registry.hxx =====


// ===== begin include/mgmake/build/request.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_REQUEST_HXX
#define MGMAKE_BUILD_REQUEST_HXX

// skipped duplicate include: include/mgmake/discovery/resolved_tool.hxx
// skipped duplicate include: include/mgmake/discovery/tool_environment.hxx
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
        std::vector<discovery::resolved_tool> m_discovered_tools{};
        discovery::tool_environment m_tool_environment{};

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

        [[nodiscard]] inline const discovery::resolved_tool* discovered_tool(
            discovery::tool_role role
        ) const noexcept {
            for (const auto& tool : m_discovered_tools) {
                if (tool.m_role == role) {
                    return &tool;
                }
            }

            return nullptr;
        }

        [[nodiscard]] inline std::filesystem::path tool_path(
            discovery::tool_role role,
            std::filesystem::path fallback = {}
        ) const {
            if (const auto* tool = discovered_tool(role)) {
                return tool->m_path;
            }

            return fallback;
        }

        [[nodiscard]] inline const discovery::tool_environment& tool_environment() const noexcept {
            return m_tool_environment;
        }
    };
}

#endif
// ===== end include/mgmake/build/request.hxx =====


// ===== begin include/mgmake/build/request_from_options.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_REQUEST_FROM_OPTIONS_HXX
#define MGMAKE_BUILD_REQUEST_FROM_OPTIONS_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/build/toolchain_registry.hxx

// ===== begin include/mgmake/cli/options.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX


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
		tools,
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
		detail::enum_entry<action_kind::tools, "tools">,
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
		detail::enum_entry<action_kind::tools, "tools">,
		detail::enum_entry<action_kind::tools, "toolchains">,
		detail::enum_entry<action_kind::tools, "toolchain-info">,
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
			action_kind::tools,
			"Show discovered build tools and discovery diagnostics."
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

// skipped duplicate include: include/mgmake/discovery/mode.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <string>
#include <vector>

namespace mgmake::cli {
	struct options {
		action_kind m_action = action_kind::build;
		backend_kind m_backend = backend_kind::automatic;

		std::string m_toolchain = "clang-mg";
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
		bool m_refresh_tools = false;
		bool m_no_tool_cache = false;
		bool m_show_tool_search = false;

		discovery::mode m_tool_discovery = discovery::mode::automatic;

		std::string m_toolchain_root{};
		std::string m_sdk_root{};
		std::string m_sysroot{};
		std::string m_package_toolchain_root{};

		std::string m_cc{};
		std::string m_cxx{};
		std::string m_ar{};
		std::string m_ranlib{};
		std::string m_librarian{};
		std::string m_linker{};
		std::string m_shared_linker{};
		std::string m_rc{};
		std::string m_mt{};
		std::string m_dll_tool{};
		std::string m_strip{};
		std::string m_objcopy{};
		std::string m_objdump{};
		std::string m_nm{};
		std::string m_readelf{};
		std::string m_ninja{};
		std::string m_cmake{};
		std::string m_pkg_config{};
		std::string m_exe_wrapper{};
		std::string m_emulator{};

		std::string m_toolchain_version{};

		std::string m_android_ndk{};
		std::string m_android_abi{};
		int m_android_api = 0;

		std::string m_apple_sdk{};

		inline constexpr const std::vector<std::string>& targets() const {
			return m_targets;
		}
		
		[[nodiscard]] inline sys::target target_platform() const {
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


#include <expected>
#include <filesystem>
#include <format>
#include <string>

namespace mgmake::build {
	template <toolchain_registry_like Toolchains>
	[[nodiscard]] inline std::expected<request, std::string> request_from_options(
		const cli::options& opts,
		const Toolchains& toolchains
	) {
		const auto* selected_toolchain = toolchains.find(opts.m_toolchain);

		if (selected_toolchain == nullptr) {
			return std::unexpected{
				std::format(
					"mgmake: unknown toolchain '{}'; expected one of: {}",
					opts.m_toolchain,
					toolchains.choices_string()
				)
			};
		}

		request result{};

		result.m_tc = *selected_toolchain;

		if (!opts.m_target_triple.empty()) {
			result.m_tc.target_triple(opts.m_target_triple);
		}

		result.m_build_dir = std::filesystem::path{ opts.m_build_dir };
		result.m_targets = opts.m_targets;
		result.m_target = opts.target_platform();

		return result;
	}
}

#endif
// ===== end include/mgmake/build/request_from_options.hxx =====


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


// ===== begin include/mgmake/build/clean.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_CLEAN_HXX
#define MGMAKE_BUILD_CLEAN_HXX

// skipped duplicate include: include/mgmake/build/request.hxx

#include <expected>
#include <filesystem>
#include <string>

namespace mgmake::build {
	[[nodiscard]] inline std::expected<void, std::string> clean(
		const build::request& req
	) {
		std::error_code ec;
		std::filesystem::remove_all(req.build_dir(), ec);

		if (ec) {
			return std::unexpected{
				"mgmake: failed to clean build directory '" +
				req.build_dir().string() +
				"': " +
				ec.message()
			};
		}

		return {};
	}
}

#endif
// ===== end include/mgmake/build/clean.hxx =====

// skipped duplicate include: include/mgmake/cli/action.hxx
// skipped duplicate include: include/mgmake/cli/backend.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx

// ===== begin include/mgmake/cli/help.hxx =====
#pragma once

#ifndef MGMAKE_CLI_HELP_HXX
#define MGMAKE_CLI_HELP_HXX


// ===== begin include/mgmake/cli/parse.hxx =====
#pragma once

#ifndef MGMAKE_CLI_PARSE_HXX
#define MGMAKE_CLI_PARSE_HXX


// ===== begin include/mgmake/cli/option_parser.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTION_PARSER_HXX
#define MGMAKE_CLI_OPTION_PARSER_HXX


// ===== begin include/mgmake/cli/option_builder.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTION_BUILDER_HXX
#define MGMAKE_CLI_OPTION_BUILDER_HXX


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

// skipped duplicate include: include/mgmake/cli/options.hxx

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
// skipped duplicate include: include/mgmake/discovery/mode.hxx
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
	struct value_parser<discovery::mode> :
		enum_value_parser<discovery::mode_parse_names, discovery::mode_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown tool discovery mode '{}'; expected one of: {}",
				text,
				discovery::mode_names::choices_string()
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

	using toolchain_option =
		value_option<&options::m_toolchain, "toolchain">
			::value_name<"name">
			::description<"Select a compiler toolchain preset.">;

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

	using refresh_tools_option =
		flag_option<&options::m_refresh_tools, "refresh-tools">
			::description<"Ignore cached tool paths and rediscover tools.">;

	using no_tool_cache_option =
		flag_option<&options::m_no_tool_cache, "no-tool-cache">
			::description<"Do not read or write the tool discovery cache.">;

	using show_tool_search_option =
		flag_option<&options::m_show_tool_search, "show-tool-search">
			::description<"Print detailed tool discovery search diagnostics.">;

	using tool_discovery_option =
		value_option<&options::m_tool_discovery, "tool-discovery">
			::value_name<"mode">
			::description<"Set tool discovery mode: automatic, exact, family-fallback, auto-fallback, disabled.">;

	using toolchain_root_option =
		value_option<&options::m_toolchain_root, "toolchain-root">
			::value_name<"path">
			::description<"Add a toolchain root to tool discovery.">;

	using sdk_root_option =
		value_option<&options::m_sdk_root, "sdk">
			::value_name<"path">
			::description<"Set an SDK root for tool discovery.">;

	using sysroot_option =
		value_option<&options::m_sysroot, "sysroot">
			::value_name<"path">
			::description<"Set a target sysroot.">;

	using package_toolchain_root_option =
		value_option<&options::m_package_toolchain_root, "package-toolchain-root">
			::value_name<"path">
			::description<"Add a package-provided toolchain root.">;

	using cc_option =
		value_option<&options::m_cc, "cc">
			::value_name<"path-or-name">
			::description<"Override the C compiler.">;

	using cxx_option =
		value_option<&options::m_cxx, "cxx">
			::value_name<"path-or-name">
			::description<"Override the C++ compiler.">;

	using ar_option =
		value_option<&options::m_ar, "ar">
			::value_name<"path-or-name">
			::description<"Override the archiver.">;

	using ranlib_option =
		value_option<&options::m_ranlib, "ranlib">
			::value_name<"path-or-name">
			::description<"Override ranlib.">;

	using librarian_option =
		value_option<&options::m_librarian, "librarian">
			::value_name<"path-or-name">
			::description<"Override the librarian.">;

	using linker_option =
		value_option<&options::m_linker, "linker">
			::value_name<"path-or-name">
			::description<"Override the linker.">;

	using shared_linker_option =
		value_option<&options::m_shared_linker, "shared-linker">
			::value_name<"path-or-name">
			::description<"Override the shared-library linker.">;

	using rc_option =
		value_option<&options::m_rc, "rc">
			::value_name<"path-or-name">
			::description<"Override the resource compiler.">;

	using mt_option =
		value_option<&options::m_mt, "mt">
			::value_name<"path-or-name">
			::description<"Override the manifest tool.">;

	using dll_tool_option =
		value_option<&options::m_dll_tool, "dll-tool">
			::value_name<"path-or-name">
			::description<"Override dlltool.">;

	using strip_option =
		value_option<&options::m_strip, "strip">
			::value_name<"path-or-name">
			::description<"Override strip.">;

	using objcopy_option =
		value_option<&options::m_objcopy, "objcopy">
			::value_name<"path-or-name">
			::description<"Override objcopy.">;

	using objdump_option =
		value_option<&options::m_objdump, "objdump">
			::value_name<"path-or-name">
			::description<"Override objdump.">;

	using nm_option =
		value_option<&options::m_nm, "nm">
			::value_name<"path-or-name">
			::description<"Override nm.">;

	using readelf_option =
		value_option<&options::m_readelf, "readelf">
			::value_name<"path-or-name">
			::description<"Override readelf.">;

	using ninja_option =
		value_option<&options::m_ninja, "ninja">
			::value_name<"path-or-name">
			::description<"Override the Ninja executable.">;

	using cmake_option =
		value_option<&options::m_cmake, "cmake">
			::value_name<"path-or-name">
			::description<"Override CMake.">;

	using pkg_config_option =
		value_option<&options::m_pkg_config, "pkg-config">
			::value_name<"path-or-name">
			::description<"Override pkg-config.">;

	using exe_wrapper_option =
		value_option<&options::m_exe_wrapper, "exe-wrapper">
			::value_name<"path-or-name">
			::description<"Override the executable wrapper.">;

	using emulator_option =
		value_option<&options::m_emulator, "emulator">
			::value_name<"path-or-name">
			::description<"Override the emulator.">;

	using toolchain_version_option =
		value_option<&options::m_toolchain_version, "toolchain-version">
			::value_name<"version">
			::description<"Request a specific toolchain version.">;

	using android_ndk_option =
		value_option<&options::m_android_ndk, "android-ndk">
			::value_name<"path">
			::description<"Set the Android NDK root.">;

	using android_abi_option =
		value_option<&options::m_android_abi, "android-abi">
			::value_name<"abi">
			::description<"Set the Android ABI.">;

	using android_api_option =
		value_option<&options::m_android_api, "android-api">
			::value_name<"level">
			::description<"Set the Android API level.">;

	using apple_sdk_option =
		value_option<&options::m_apple_sdk, "apple-sdk">
			::value_name<"sdk">
			::description<"Set the Apple SDK name for xcrun discovery.">;

	using default_parser = option_parser<
		help_option,
		version_option,
		verbose_option,
		dry_run_option,
		backend_option,
		toolchain_option,
		build_dir_option,
		jobs_option,
		target_option,
		platform_option,
		arch_option,
		abi_option,
		target_triple_option,
		refresh_tools_option,
		no_tool_cache_option,
		show_tool_search_option,
		tool_discovery_option,
		toolchain_root_option,
		sdk_root_option,
		sysroot_option,
		package_toolchain_root_option,
		cc_option,
		cxx_option,
		ar_option,
		ranlib_option,
		librarian_option,
		linker_option,
		shared_linker_option,
		rc_option,
		mt_option,
		dll_tool_option,
		strip_option,
		objcopy_option,
		objdump_option,
		nm_option,
		readelf_option,
		ninja_option,
		cmake_option,
		pkg_config_option,
		exe_wrapper_option,
		emulator_option,
		toolchain_version_option,
		android_ndk_option,
		android_abi_option,
		android_api_option,
		apple_sdk_option
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

// skipped duplicate include: include/mgmake/build/toolchain_registry.hxx

#include <print>
#include <string_view>

namespace mgmake::cli {
	template <build::toolchain_registry_like Toolchains>
	inline void print_help(
		std::string_view program_name,
		const Toolchains& toolchains
	) {
		print_help(program_name);

		std::println("");
		std::println("Available toolchains:");
		std::println("  {}", toolchains.choices_string());
	}
}

#endif
// ===== end include/mgmake/cli/help.hxx =====

// skipped duplicate include: include/mgmake/cli/parse_result.hxx
// skipped duplicate include: include/mgmake/cli/option_parse_result.hxx
// skipped duplicate include: include/mgmake/cli/value_parser.hxx
// skipped duplicate include: include/mgmake/cli/enum_value_parser.hxx
// skipped duplicate include: include/mgmake/cli/option_builder.hxx
// skipped duplicate include: include/mgmake/cli/option_parser.hxx
// skipped duplicate include: include/mgmake/cli/parse.hxx
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

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx

#include <expected>
#include <string>

namespace mgmake::backend {
	trait generate_backend {
		void generate(const dag::graph& graph, const build::request& req) const;
	};

	trait build_backend {
		std::expected<void, std::string> build(
			const dag::graph& graph,
			const build::request& req
		) const;
	};
}

#endif
// ===== end include/mgmake/backend/traits.hxx =====


// ===== begin include/mgmake/backend/graphviz.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_GRAPHVIZ_HXX
#define MGMAKE_BACKEND_GRAPHVIZ_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx
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

        void generate(const cli::options&, const dag::graph& graph, const build::request& req) const {
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

#endif
// ===== end include/mgmake/backend/graphviz.hxx =====


// ===== begin include/mgmake/backend/ninja.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_NINJA_HXX
#define MGMAKE_BACKEND_NINJA_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx

// ===== begin include/mgmake/discovery/resolve.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_RESOLVE_HXX
#define MGMAKE_DISCOVERY_RESOLVE_HXX


// ===== begin include/mgmake/discovery/backend_requirement.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_BACKEND_REQUIREMENT_HXX
#define MGMAKE_DISCOVERY_BACKEND_REQUIREMENT_HXX

// skipped duplicate include: include/mgmake/discovery/tool_role.hxx

#include <string>

namespace mgmake::discovery {
	struct backend_tool_requirement {
		tool_role m_role{};
		std::string m_logical_name{};
		std::string m_needed_because{};
	};
}

#endif
// ===== end include/mgmake/discovery/backend_requirement.hxx =====


// ===== begin include/mgmake/discovery/diagnostic.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_DIAGNOSTIC_HXX
#define MGMAKE_DISCOVERY_DIAGNOSTIC_HXX


// ===== begin include/mgmake/discovery/tool_requirement.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_REQUIREMENT_HXX
#define MGMAKE_DISCOVERY_TOOL_REQUIREMENT_HXX

// skipped duplicate include: include/mgmake/discovery/tool_family.hxx
// skipped duplicate include: include/mgmake/discovery/tool_role.hxx
// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx

// ===== begin include/mgmake/spec/project.hxx =====
#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx
// skipped duplicate include: include/mgmake/detail/assert.hxx

// ===== begin include/mgmake/spec/executable.hxx =====
#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX


// ===== begin include/mgmake/spec/target.hxx =====
#pragma once

#ifndef MGMK_SPEC_TARGET_HXX
#define MGMK_SPEC_TARGET_HXX

// skipped duplicate include: include/mgmake/detail/assert.hxx

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
			mgmkassert(
				not file.empty(),
				"mgmake spec: target '" + m_name + "' cannot add an empty source path"
			);

			m_sources.emplace(file);
			return self();
		}
		inline constexpr auto& sources() const {
			return m_sources;
		}

		inline constexpr auto& add_include_dir(const std::filesystem::path& file) {
			mgmkassert(
				not file.empty(),
				"mgmake spec: target '" + m_name + "' cannot add an empty include directory"
			);

			m_include_dirs.emplace(file);
			return self();
		}
		inline constexpr auto& include_dirs() const {
			return m_include_dirs;
		}

		inline constexpr auto& link(std::string_view lib) {
			mgmkassert(
				not lib.empty(),
				"mgmake spec: target '" + m_name + "' cannot link an empty library name"
			);
			mgmkassert(
				lib != m_name,
				"mgmake spec: target '" + m_name + "' cannot link itself"
			);

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


#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct executable : public target<executable> {
		using id = std::vector<executable>::size_type;

		inline constexpr executable(std::string_view name)
			: target<executable>{ std::string{ name } } {
			mgmkassert(not m_name.empty(), "mgmake spec: executable target has no name");
		}
	};
}

#endif
// ===== end include/mgmake/spec/executable.hxx =====


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

		inline constexpr library(std::string_view name, kind k)
			: target<library>{ std::string{ name } }, m_kind{k} {
			mgmkassert(not m_name.empty(), "mgmake spec: library target has no name");
			mgmkassert(
				m_kind == kind::static_lib ||
					m_kind == kind::shared_lib ||
					m_kind == kind::interface,
				"mgmake spec: invalid library kind"
			);
		}

		inline constexpr auto& add_source(const std::filesystem::path& file) {
			mgmkassert(
				m_kind != kind::interface,
				"mgmake spec: interface library '" + m_name + "' cannot have sources"
			);

			target<library>::add_source(file);
			return *this;
		}
	};
}

#endif
// ===== end include/mgmake/spec/library.hxx =====


#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project {
		std::string m_name;
		std::vector<spec::executable> m_executables;
		std::vector<spec::library> m_libraries;

		inline constexpr project(std::string_view name)
			: m_name{name} {
			mgmkassert(not m_name.empty(), "mgmake spec: project has no name");
		}

		inline constexpr project& add_target(const spec::executable& exe) {
			mgmkassert(not exe.m_name.empty(), "mgmake spec: executable target has no name");
            mgmkassert(not find_library(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");
            mgmkassert(not find_executable(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");
			mgmkassert(not exe.m_sources.empty(), "mgmake spec: executable target '" + exe.m_name + "' has no sources");

			assert_known_libraries_for(exe.m_linked_libraries, exe.m_name);

			m_executables.emplace_back(exe);
			return *this;
		}
		inline constexpr project& add_target(const spec::library& lib) {
			mgmkassert(not lib.m_name.empty(), "mgmake spec: library target has no name");
            mgmkassert(not find_executable(lib.m_name).has_value(), "mgmake spec: target name conflict '" + lib.m_name + "'");
			mgmkassert(not find_library(lib.m_name).has_value(), "mgmake spec: target name conflict '" + lib.m_name + "'");

			switch (lib.m_kind) {
				case spec::library::kind::interface:
					mgmkassert(
						lib.m_sources.empty(),
						"mgmake spec: interface library '" + lib.m_name + "' cannot have sources"
					);
					break;

				case spec::library::kind::static_lib:
					mgmkassert(
						not lib.m_sources.empty(),
						"mgmake spec: static library '" + lib.m_name + "' has no sources"
					);
					break;

				case spec::library::kind::shared_lib:
					mgmkassert(
						not lib.m_sources.empty(),
						"mgmake spec: shared library '" + lib.m_name + "' has no sources"
					);
					break;

				default:
					mgmkassert(false, "mgmake spec: invalid library kind for target '" + lib.m_name + "'");
					break;
			}

			assert_known_libraries_for(lib.m_linked_libraries, lib.m_name);
			assert_library_link_closure_is_acyclic(lib);

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

	private:
		inline constexpr void assert_known_libraries_for(
			const std::set<std::string>& libraries,
			std::string_view owner_name
		) const {
			for (const auto& library_name : libraries) {
				mgmkassert(
					find_library(library_name).has_value(),
					"mgmake spec: target '" + std::string{owner_name} +
						"' links unknown library '" + library_name + "'"
				);
			}
		}

		inline constexpr bool library_stack_contains(
			const std::vector<std::string_view>& stack,
			std::string_view name
		) const {
			for (const auto existing : stack) {
				if (existing == name) {
					return true;
				}
			}

			return false;
		}

		inline constexpr void assert_library_link_closure_is_acyclic(
			std::string_view library_name,
			const std::set<std::string>& linked_libraries,
			std::vector<std::string_view>& active_libraries
		) const {
			mgmkassert(
				not library_stack_contains(active_libraries, library_name),
				"mgmake spec: cyclic library dependency involving '" + std::string{library_name} + "'"
			);

			active_libraries.emplace_back(library_name);

			for (const auto& linked_name : linked_libraries) {
				mgmkassert(
					not library_stack_contains(active_libraries, linked_name),
					"mgmake spec: cyclic library dependency involving '" + linked_name + "'"
				);

				const auto linked_id = find_library(linked_name);
				mgmkassert(
					linked_id.has_value(),
					"mgmake spec: library '" + std::string{library_name} +
						"' links unknown library '" + linked_name + "'"
				);

				const auto& linked_library = m_libraries.at(linked_id.value());
				assert_library_link_closure_is_acyclic(
					linked_library.m_name,
					linked_library.m_linked_libraries,
					active_libraries
				);
			}

			active_libraries.pop_back();
		}

		inline constexpr void assert_library_link_closure_is_acyclic(
			const spec::library& lib
		) const {
			std::vector<std::string_view> active_libraries{};
			assert_library_link_closure_is_acyclic(
				lib.m_name,
				lib.m_linked_libraries,
				active_libraries
			);
		}
	};
}

#endif
// ===== end include/mgmake/spec/project.hxx =====


#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::discovery {
	enum struct requirement_strength {
		required,
		optional,
		probe_only
	};

	struct tool_requirement {
		tool_role m_role{};
		requirement_strength m_strength = requirement_strength::required;
		std::string m_logical_name{};
		std::string m_needed_because{};
		tool_family m_expected_family = tool_family::unknown;
		object_format m_expected_object_format = object_format::unknown;
		std::string m_target_triple{};

		[[nodiscard]] inline bool required() const noexcept {
			return m_strength == requirement_strength::required;
		}
	};

	[[nodiscard]] inline std::vector<tool_requirement> required_tools(
		const cli::options&,
		const build::request& req,
		const spec::project& project
	) {
		std::vector<tool_requirement> result;
		const auto& tc = req.toolchain();

		bool has_c_sources = false;
		bool has_cxx_sources = false;
		bool has_asm_sources = false;
		bool has_rc_sources = false;
		bool has_idl_sources = false;
		bool has_static_library = false;
		bool has_shared_library = false;
		bool has_executable = false;

		auto inspect_sources = [&](const auto& target) {
			for (const auto& source : target.m_sources) {
				const auto ext = source.extension().string();

				if (ext == ".c") {
					has_c_sources = true;
				} else if (ext == ".cc" || ext == ".cpp" || ext == ".cxx" || ext == ".C") {
					has_cxx_sources = true;
				} else if (ext == ".s" || ext == ".S" || ext == ".asm") {
					has_asm_sources = true;
				} else if (ext == ".rc") {
					has_rc_sources = true;
				} else if (ext == ".idl") {
					has_idl_sources = true;
				} else {
					has_cxx_sources = true;
				}
			}
		};

		for (const auto& lib : project.m_libraries) {
			inspect_sources(lib);

			switch (lib.m_kind) {
				case spec::library::kind::interface:
					break;

				case spec::library::kind::static_lib:
					has_static_library = true;
					break;

				case spec::library::kind::shared_lib:
					has_shared_library = true;
					break;
			}
		}

		for (const auto& exe : project.m_executables) {
			inspect_sources(exe);
			has_executable = true;
		}

		if (has_c_sources) {
			result.push_back({tool_role::c_compiler, requirement_strength::required, std::string{tc.tool(tool_role::c_compiler)}, "the project has C sources"});
		}

		if (has_cxx_sources) {
			result.push_back({tool_role::cxx_compiler, requirement_strength::required, std::string{tc.tool(tool_role::cxx_compiler)}, "the project has C++ sources"});
		}

		if (has_asm_sources) {
			result.push_back({tool_role::assembler, requirement_strength::required, std::string{tc.tool(tool_role::assembler)}, "the project has assembly sources"});
		}

		if (has_rc_sources) {
			result.push_back({tool_role::resource_compiler, requirement_strength::required, std::string{tc.tool(tool_role::resource_compiler)}, "the project has Windows resource sources"});
		}

		if (has_idl_sources) {
			result.push_back({tool_role::midl_compiler, requirement_strength::required, std::string{tc.tool(tool_role::midl_compiler)}, "the project has IDL sources"});
		}

		if (has_static_library) {
			const auto role = tc.dialect() == build::toolchain::dialect::msvc
				? tool_role::librarian
				: tool_role::archiver;

			result.push_back({role, requirement_strength::required, std::string{tc.tool(role)}, "the project builds at least one static library"});

			if (!tc.tool(tool_role::ranlib).empty()) {
				result.push_back({tool_role::ranlib, requirement_strength::optional, std::string{tc.tool(tool_role::ranlib)}, "the selected toolchain declares ranlib"});
			}
		}

		if (has_shared_library) {
			const auto shared = tc.tool(tool_role::shared_linker);
			result.push_back({
				shared.empty() ? tool_role::linker : tool_role::shared_linker,
				requirement_strength::required,
				std::string{shared.empty() ? tc.tool(tool_role::linker) : shared},
				"the project builds at least one shared library"
			});
		}

		if (has_executable) {
			result.push_back({tool_role::linker, requirement_strength::required, std::string{tc.tool(tool_role::linker)}, "the project builds at least one executable"});
		}

		if (req.target_platform() == sys::platform::p_windows && tc.dialect() == build::toolchain::dialect::msvc) {
			if (!tc.tool(tool_role::manifest_tool).empty()) {
				result.push_back({tool_role::manifest_tool, requirement_strength::optional, std::string{tc.tool(tool_role::manifest_tool)}, "MSVC-style Windows builds may require manifest handling"});
			}
		}

		return result;
	}
}

#endif
// ===== end include/mgmake/discovery/tool_requirement.hxx =====

// skipped duplicate include: include/mgmake/discovery/tool_role.hxx

#include <string>
#include <vector>

namespace mgmake::discovery {
	struct diagnostic {
		tool_role m_role{};
		std::string m_toolchain{};
		std::string m_logical_name{};
		std::string m_needed_because{};
		std::vector<std::string> m_searched{};
		std::vector<std::string> m_rejected{};
		std::vector<std::string> m_notes{};
		std::vector<std::string> m_fixes{};

		[[nodiscard]] std::string format_missing_tool() const {
			std::string result;
			result += "mgmake: required tool not found\n\n";
			result += "toolchain:\n  " + m_toolchain + "\n\n";
			result += "required tool:\n  " + std::string{name(m_role)} + "\n\n";

			if (!m_logical_name.empty()) {
				result += "logical name:\n  " + m_logical_name + "\n\n";
			}

			if (!m_needed_because.empty()) {
				result += "needed because:\n  " + m_needed_because + "\n\n";
			}

			if (!m_searched.empty()) {
				result += "searched:\n";
				for (const auto& item : m_searched) {
					result += "  " + item + "\n";
				}
				result += "\n";
			}

			if (!m_rejected.empty()) {
				result += "rejected:\n";
				for (const auto& item : m_rejected) {
					result += "  " + item + "\n";
				}
				result += "\n";
			}

			if (!m_notes.empty()) {
				result += "notes:\n";
				for (const auto& item : m_notes) {
					result += "  " + item + "\n";
				}
				result += "\n";
			}

			if (!m_fixes.empty()) {
				result += "fixes:\n";
				for (const auto& item : m_fixes) {
					result += "  " + item + "\n";
				}
			}

			return result;
		}
	};

	[[nodiscard]] inline std::vector<std::string> fixes_for(const tool_requirement& req) {
		switch (req.m_role) {
			case tool_role::archiver:
			case tool_role::ranlib:
				return {
					"install LLVM/GNU binutils for the selected toolchain",
					"set the matching MGMK_* environment variable",
					"pass an explicit --ar or --ranlib path",
					"add the tool directory to PATH",
					"run './build tools --refresh'"
				};

			case tool_role::generator_ninja:
				return {
					"install Ninja",
					"set MGMK_NINJA",
					"pass --ninja <path>",
					"add Ninja to PATH",
					"run './build tools --refresh'"
				};

			default:
				return {
					"set the matching MGMK_* environment variable",
					"pass an explicit tool override",
					"add the tool directory to PATH",
					"run './build tools --refresh'"
				};
		}
	}
}

#endif
// ===== end include/mgmake/discovery/diagnostic.hxx =====


// ===== begin include/mgmake/discovery/environment_provider.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_ENVIRONMENT_PROVIDER_HXX
#define MGMAKE_DISCOVERY_ENVIRONMENT_PROVIDER_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx
// skipped duplicate include: include/mgmake/spec/project.hxx

// ===== begin include/mgmake/discovery/environment.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_ENVIRONMENT_HXX
#define MGMAKE_DISCOVERY_ENVIRONMENT_HXX

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace mgmake::discovery {
	[[nodiscard]] inline std::optional<std::string> getenv_string(std::string_view name) {
		std::string key{name};
		const char* value = std::getenv(key.c_str());

		if (value == nullptr || value[0] == '\0') {
			return std::nullopt;
		}

		return std::string{value};
	}

	[[nodiscard]] inline std::optional<std::filesystem::path> getenv_path(
		std::string_view name
	) {
		if (auto value = getenv_string(name)) {
			return std::filesystem::path{*value};
		}

		return std::nullopt;
	}
}

#endif
// ===== end include/mgmake/discovery/environment.hxx =====

// skipped duplicate include: include/mgmake/discovery/tool_environment.hxx

// ===== begin include/mgmake/discovery/windows/visual_studio.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_WINDOWS_VISUAL_STUDIO_HXX
#define MGMAKE_DISCOVERY_WINDOWS_VISUAL_STUDIO_HXX


// ===== begin include/mgmake/discovery/providers.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_PROVIDERS_HXX
#define MGMAKE_DISCOVERY_PROVIDERS_HXX


// ===== begin include/mgmake/discovery/context.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_CONTEXT_HXX
#define MGMAKE_DISCOVERY_CONTEXT_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx

// ===== begin include/mgmake/discovery/cache.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_CACHE_HXX
#define MGMAKE_DISCOVERY_CACHE_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

// ===== begin include/mgmake/discovery/filesystem.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_FILESYSTEM_HXX
#define MGMAKE_DISCOVERY_FILESYSTEM_HXX

// skipped duplicate include: include/mgmake/discovery/environment.hxx

#include <cctype>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#if defined(MGMK_PLATFORM_POSIX)
	#include <unistd.h>
#endif

namespace mgmake::discovery {
	[[nodiscard]] inline bool has_windows_drive_prefix(std::string_view text) noexcept {
		return text.size() >= 2
			&& std::isalpha(static_cast<unsigned char>(text[0]))
			&& text[1] == ':';
	}

	[[nodiscard]] inline bool is_path_like(std::string_view text) {
		return text.find('/') != std::string_view::npos
			|| text.find('\\') != std::string_view::npos
			|| has_windows_drive_prefix(text);
	}

	[[nodiscard]] inline bool is_explicit_path(std::string_view text) {
		return is_path_like(text);
	}

	[[nodiscard]] inline bool is_launchable_file(const std::filesystem::path& path) {
		std::error_code ec;

		if (!std::filesystem::exists(path, ec) || ec) {
			return false;
		}

		if (!std::filesystem::is_regular_file(path, ec) || ec) {
			return false;
		}

#if defined(_WIN32)
		return true;
#else
		return ::access(path.c_str(), X_OK) == 0;
#endif
	}

	[[nodiscard]] inline std::vector<std::string> executable_suffixes() {
#if defined(_WIN32)
		return {".com", ".exe", ".bat", ".cmd", ""};
#else
		return {""};
#endif
	}

	[[nodiscard]] inline std::optional<std::filesystem::path> find_in_directory(
		const std::filesystem::path& dir,
		std::string_view name
	) {
		if (dir.empty() || name.empty()) {
			return std::nullopt;
		}

		std::filesystem::path requested{name};

		if (requested.has_extension()) {
			auto candidate = dir / requested;

			if (is_launchable_file(candidate)) {
				return std::filesystem::absolute(candidate);
			}
		}

		for (const auto& suffix : executable_suffixes()) {
			auto candidate = dir / (std::string{name} + suffix);

			if (is_launchable_file(candidate)) {
				return std::filesystem::absolute(candidate);
			}
		}

		return std::nullopt;
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> path_entries() {
		std::vector<std::filesystem::path> result;
		auto path = getenv_string("PATH");

		if (!path) {
			return result;
		}

#if defined(_WIN32)
		constexpr char separator = ';';
#else
		constexpr char separator = ':';
#endif

		std::string_view view{*path};
		std::size_t start = 0;

		while (start <= view.size()) {
			const auto end = view.find(separator, start);
			const auto part = view.substr(
				start,
				end == std::string_view::npos ? std::string_view::npos : end - start
			);

			if (!part.empty()) {
				result.emplace_back(part);
			}

			if (end == std::string_view::npos) {
				break;
			}

			start = end + 1;
		}

		return result;
	}

	[[nodiscard]] inline std::optional<std::filesystem::path> find_on_path(
		std::string_view name
	) {
		if (is_explicit_path(name)) {
			std::filesystem::path path{name};

			if (is_launchable_file(path)) {
				return std::filesystem::absolute(path);
			}

			return std::nullopt;
		}

		for (const auto& dir : path_entries()) {
			if (auto found = find_in_directory(dir, name)) {
				return found;
			}
		}

		return std::nullopt;
	}
}

#endif
// ===== end include/mgmake/discovery/filesystem.hxx =====

// skipped duplicate include: include/mgmake/discovery/resolved_tool.hxx

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::discovery {
	struct cache_entry {
		std::string m_toolchain{};
		std::string m_host_key{};
		std::string m_target_key{};
		sys::target m_host{};
		sys::target m_target{};
		tool_role m_role{};
		std::string m_logical_name{};
		std::filesystem::path m_path{};
		tool_provider m_provider{};
		std::string m_version{};
	};

	[[nodiscard]] inline std::string target_key(const sys::target& target) {
		std::string result;
		result += std::string{sys::name(target.m_arch)};
		result += '-';
		result += std::string{sys::name(target.m_platform)};
		result += '-';
		result += std::string{sys::name(target.m_abi)};

		if (!target.m_triple.empty()) {
			result += '-';
			result += target.m_triple;
		}

		return result;
	}

	struct cache {
		std::vector<cache_entry> m_entries{};

		[[nodiscard]] std::optional<cache_entry> find(
			std::string_view toolchain,
			const sys::target& host,
			const sys::target& target,
			tool_role role,
			std::string_view logical_name
		) const {
			const auto host_key = target_key(host);
			const auto target_key_value = target_key(target);

			for (const auto& entry : m_entries) {
				const auto entry_host_key = entry.m_host_key.empty()
					? target_key(entry.m_host)
					: entry.m_host_key;
				const auto entry_target_key = entry.m_target_key.empty()
					? target_key(entry.m_target)
					: entry.m_target_key;

				if (entry.m_toolchain == toolchain
					&& entry_host_key == host_key
					&& entry_target_key == target_key_value
					&& entry.m_role == role
					&& entry.m_logical_name == logical_name
					&& is_launchable_file(entry.m_path)) {
					return entry;
				}
			}

			return std::nullopt;
		}

		void put(cache_entry entry) {
			if (entry.m_host_key.empty()) {
				entry.m_host_key = target_key(entry.m_host);
			}

			if (entry.m_target_key.empty()) {
				entry.m_target_key = target_key(entry.m_target);
			}

			for (auto& existing : m_entries) {
				if (existing.m_toolchain == entry.m_toolchain
					&& (existing.m_host_key.empty() ? target_key(existing.m_host) : existing.m_host_key) == entry.m_host_key
					&& (existing.m_target_key.empty() ? target_key(existing.m_target) : existing.m_target_key) == entry.m_target_key
					&& existing.m_role == entry.m_role
					&& existing.m_logical_name == entry.m_logical_name) {
					existing = std::move(entry);
					return;
				}
			}

			m_entries.emplace_back(std::move(entry));
		}
	};

	[[nodiscard]] inline std::filesystem::path cache_path(const build::request& req) {
		return req.build_dir() / "mgmake" / "tool-cache.txt";
	}

	[[nodiscard]] inline std::optional<std::string> value_after(
		std::string_view line,
		std::string_view prefix
	) {
		if (!line.starts_with(prefix)) {
			return std::nullopt;
		}

		return std::string{line.substr(prefix.size())};
	}

	[[nodiscard]] inline cache load_cache(const build::request& req) {
		cache result;
		std::ifstream in(cache_path(req));

		if (!in.is_open()) {
			return result;
		}

		std::string line;
		cache_entry current{};
		bool active = false;

		auto flush = [&] {
			if (active && !current.m_path.empty()) {
				result.m_entries.emplace_back(std::move(current));
			}

			current = cache_entry{};
			active = false;
		};

		while (std::getline(in, line)) {
			if (line.empty()) {
				flush();
				continue;
			}

			if (line == "mgmake-tool-cache-v1") {
				continue;
			}

			active = true;

			if (auto value = value_after(line, "toolchain=")) current.m_toolchain = *value;
			else if (auto value = value_after(line, "host=")) current.m_host_key = *value;
			else if (auto value = value_after(line, "target=")) current.m_target_key = *value;
			else if (auto value = value_after(line, "tool.role=")) {
				if (auto role = tool_role_names::from_string(*value)) current.m_role = *role;
			} else if (auto value = value_after(line, "tool.logical=")) current.m_logical_name = *value;
			else if (auto value = value_after(line, "tool.path=")) current.m_path = *value;
			else if (auto value = value_after(line, "tool.provider=")) {
				if (auto provider = tool_provider_names::from_string(*value)) current.m_provider = *provider;
			} else if (auto value = value_after(line, "tool.version=")) current.m_version = *value;
		}

		flush();
		return result;
	}

	inline void save_cache(const build::request& req, const cache& cache_data) {
		const auto path = cache_path(req);

		if (path.has_parent_path()) {
			std::filesystem::create_directories(path.parent_path());
		}

		std::ofstream out(path);

		if (!out.is_open()) {
			return;
		}

		out << "mgmake-tool-cache-v1\n\n";

		for (const auto& entry : cache_data.m_entries) {
			const auto host_key = entry.m_host_key.empty()
				? target_key(entry.m_host)
				: entry.m_host_key;
			const auto target_key_value = entry.m_target_key.empty()
				? target_key(entry.m_target)
				: entry.m_target_key;

			out << "toolchain=" << entry.m_toolchain << "\n";
			out << "host=" << host_key << "\n";
			out << "target=" << target_key_value << "\n";
			out << "tool.role=" << name(entry.m_role) << "\n";
			out << "tool.logical=" << entry.m_logical_name << "\n";
			out << "tool.path=" << entry.m_path.generic_string() << "\n";
			out << "tool.provider=" << name(entry.m_provider) << "\n";
			out << "tool.version=" << entry.m_version << "\n\n";
		}
	}
}

#endif
// ===== end include/mgmake/discovery/cache.hxx =====

// skipped duplicate include: include/mgmake/discovery/mode.hxx
// skipped duplicate include: include/mgmake/discovery/resolved_tool.hxx

#include <filesystem>
#include <string>
#include <vector>

namespace mgmake::spec { struct project; }

namespace mgmake::discovery {
	struct context {
		const cli::options* m_options = nullptr;
		const spec::project* m_project = nullptr;
		const build::request* m_request = nullptr;
		cache m_cache{};
		std::vector<resolved_tool> m_resolved_tools{};
		std::vector<rejected_tool_candidate> m_rejected{};
		std::vector<std::string> m_notes{};
		mode m_mode = mode::exact;
		bool m_use_cache = true;
		bool m_refresh_cache = false;
		bool m_verbose = false;
		bool m_show_search = false;

		[[nodiscard]] inline const cli::options& options() const {
			return *m_options;
		}

		[[nodiscard]] inline const build::request& request() const {
			return *m_request;
		}

		[[nodiscard]] inline const build::toolchain& toolchain() const {
			return request().toolchain();
		}
	};
}

#endif
// ===== end include/mgmake/discovery/context.hxx =====

// skipped duplicate include: include/mgmake/discovery/filesystem.hxx

// ===== begin include/mgmake/discovery/tool_names.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_NAMES_HXX
#define MGMAKE_DISCOVERY_TOOL_NAMES_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/build/toolchain.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx
// skipped duplicate include: include/mgmake/discovery/mode.hxx
// skipped duplicate include: include/mgmake/discovery/tool_role.hxx

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::discovery {
	[[nodiscard]] inline std::string environment_variable_for(tool_role role) {
		switch (role) {
			case tool_role::c_compiler: return "MGMK_CC";
			case tool_role::cxx_compiler: return "MGMK_CXX";
			case tool_role::archiver: return "MGMK_AR";
			case tool_role::ranlib: return "MGMK_RANLIB";
			case tool_role::librarian: return "MGMK_LIB";
			case tool_role::linker: return "MGMK_LINKER";
			case tool_role::shared_linker: return "MGMK_SHARED_LINKER";
			case tool_role::resource_compiler: return "MGMK_RC";
			case tool_role::manifest_tool: return "MGMK_MT";
			case tool_role::dll_tool: return "MGMK_DLLTOOL";
			case tool_role::strip: return "MGMK_STRIP";
			case tool_role::objcopy: return "MGMK_OBJCOPY";
			case tool_role::objdump: return "MGMK_OBJDUMP";
			case tool_role::nm: return "MGMK_NM";
			case tool_role::readelf: return "MGMK_READELF";
			case tool_role::cmake: return "MGMK_CMAKE";
			case tool_role::pkg_config: return "MGMK_PKG_CONFIG";
			case tool_role::generator_ninja: return "MGMK_NINJA";
			case tool_role::exe_wrapper: return "MGMK_EXE_WRAPPER";
			case tool_role::emulator: return "MGMK_EMULATOR";
			default: return {};
		}
	}

	[[nodiscard]] inline std::string cli_override_for(const cli::options& opts, tool_role role) {
		switch (role) {
			case tool_role::c_compiler: return opts.m_cc;
			case tool_role::cxx_compiler: return opts.m_cxx;
			case tool_role::archiver: return opts.m_ar;
			case tool_role::ranlib: return opts.m_ranlib;
			case tool_role::librarian: return opts.m_librarian;
			case tool_role::linker: return opts.m_linker;
			case tool_role::shared_linker: return opts.m_shared_linker;
			case tool_role::resource_compiler: return opts.m_rc;
			case tool_role::manifest_tool: return opts.m_mt;
			case tool_role::dll_tool: return opts.m_dll_tool;
			case tool_role::strip: return opts.m_strip;
			case tool_role::objcopy: return opts.m_objcopy;
			case tool_role::objdump: return opts.m_objdump;
			case tool_role::nm: return opts.m_nm;
			case tool_role::readelf: return opts.m_readelf;
			case tool_role::generator_ninja: return opts.m_ninja;
			case tool_role::cmake: return opts.m_cmake;
			case tool_role::pkg_config: return opts.m_pkg_config;
			case tool_role::exe_wrapper: return opts.m_exe_wrapper;
			case tool_role::emulator: return opts.m_emulator;
			default: return {};
		}
	}

	[[nodiscard]] inline std::vector<std::string> logical_names_for(
		const build::toolchain& tc,
		tool_role role
	) {
		std::vector<std::string> result;

		if (auto value = tc.tool(role); !value.empty()) {
			result.emplace_back(value);
		}

		return result;
	}

	[[nodiscard]] inline std::vector<std::string> fallback_names_for(
		const build::toolchain& tc,
		const build::request&,
		tool_role role
	) {
		if (tc.dialect() == build::toolchain::dialect::msvc) {
			switch (role) {
				case tool_role::c_compiler:
				case tool_role::cxx_compiler: return {"cl"};
				case tool_role::archiver:
				case tool_role::librarian: return {"lib"};
				case tool_role::linker:
				case tool_role::shared_linker: return {"link"};
				case tool_role::resource_compiler: return {"rc", "llvm-rc"};
				case tool_role::manifest_tool: return {"mt"};
				default: break;
			}
		}

		const bool clang_like = tc.name().find("clang") != std::string_view::npos;

		if (clang_like) {
			switch (role) {
				case tool_role::c_compiler: return {"clang"};
				case tool_role::cxx_compiler: return {"clang++"};
				case tool_role::archiver: return {"llvm-ar", "ar"};
				case tool_role::ranlib: return {"llvm-ranlib", "ranlib"};
				case tool_role::linker:
				case tool_role::shared_linker: return {"clang++", "clang"};
				case tool_role::resource_compiler: return {"llvm-rc", "windres"};
				case tool_role::strip: return {"llvm-strip", "strip"};
				case tool_role::objcopy: return {"llvm-objcopy", "objcopy"};
				case tool_role::objdump: return {"llvm-objdump", "objdump"};
				case tool_role::nm: return {"llvm-nm", "nm"};
				case tool_role::readelf: return {"llvm-readelf", "readelf"};
				default: break;
			}
		}

		switch (role) {
			case tool_role::c_compiler: return {"gcc", "cc"};
			case tool_role::cxx_compiler: return {"g++", "c++"};
			case tool_role::archiver: return {"gcc-ar", "ar"};
			case tool_role::ranlib: return {"gcc-ranlib", "ranlib"};
			case tool_role::linker:
			case tool_role::shared_linker: return {"g++", "gcc"};
			case tool_role::resource_compiler: return {"windres"};
			case tool_role::strip: return {"strip"};
			case tool_role::objcopy: return {"objcopy"};
			case tool_role::objdump: return {"objdump"};
			case tool_role::nm: return {"nm"};
			case tool_role::readelf: return {"readelf"};
			default: break;
		}

		return {};
	}

	[[nodiscard]] inline std::vector<std::string> candidate_names_for(
		const build::request& req,
		tool_role role,
		mode discovery_mode
	) {
		std::vector<std::string> result = logical_names_for(req.toolchain(), role);

		if (discovery_mode == mode::exact) {
			return result;
		}

		for (auto fallback : fallback_names_for(req.toolchain(), req, role)) {
			if (std::ranges::find(result, fallback) == result.end()) {
				result.emplace_back(std::move(fallback));
			}
		}

		return result;
	}

	[[nodiscard]] inline std::vector<std::string> target_prefixed_names_for(
		const build::request& req,
		tool_role role,
		mode discovery_mode
	) {
		std::vector<std::string> result;
		std::string triple;

		if (req.toolchain().target_triple().has_value()) {
			triple = *req.toolchain().target_triple();
		} else {
			triple = req.target().m_triple;
		}

		if (triple.empty()) {
			return result;
		}

		for (const auto& name : candidate_names_for(req, role, discovery_mode)) {
			if (!name.starts_with(triple + "-")) {
				result.emplace_back(triple + "-" + name);
			}
		}

		return result;
	}
}

#endif
// ===== end include/mgmake/discovery/tool_names.hxx =====

// skipped duplicate include: include/mgmake/discovery/tool_requirement.hxx

#include <algorithm>
#include <filesystem>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace mgmake::discovery {
	using candidate_list = std::vector<tool_candidate>;

	inline void add_candidate_if_found(
		candidate_list& out,
		const tool_requirement& req,
		std::string_view logical_name,
		const std::filesystem::path& dir,
		tool_provider provider,
		int priority,
		std::string reason,
		bool authoritative = false,
		std::optional<std::filesystem::path> provider_root = std::nullopt
	) {
		if (auto path = find_in_directory(dir, logical_name)) {
			out.push_back({
				.m_role = req.m_role,
				.m_logical_name = std::string{logical_name},
				.m_path = *path,
				.m_provider = provider,
				.m_reason = std::move(reason),
				.m_priority = priority,
				.m_authoritative = authoritative,
				.m_provider_root = std::move(provider_root)
			});
		}
	}

	inline void add_candidates_from_dirs(
		candidate_list& out,
		const build::request& build_req,
		const tool_requirement& req,
		const std::vector<std::filesystem::path>& dirs,
		tool_provider provider,
		int priority,
		std::string reason,
		bool include_target_prefixed = true,
		mode discovery_mode = mode::family_fallback,
		std::optional<std::filesystem::path> provider_root = std::nullopt
	) {
		std::vector<std::string> names;

		if (!req.m_logical_name.empty()) {
			names.emplace_back(req.m_logical_name);
		}

		for (auto name : candidate_names_for(build_req, req.m_role, discovery_mode)) {
			if (std::ranges::find(names, name) == names.end()) {
				names.emplace_back(std::move(name));
			}
		}

		if (include_target_prefixed) {
			for (auto name : target_prefixed_names_for(build_req, req.m_role, discovery_mode)) {
				if (std::ranges::find(names, name) == names.end()) {
					names.emplace_back(std::move(name));
				}
			}
		}

		for (const auto& name : names) {
			for (const auto& dir : dirs) {
				add_candidate_if_found(out, req, name, dir, provider, priority, reason, false, provider_root);
			}
		}
	}

	inline void add_explicit_path_candidates(context&, const tool_requirement& req, candidate_list& out) {
		if (!req.m_logical_name.empty() && is_explicit_path(req.m_logical_name)) {
			out.push_back({
				.m_role = req.m_role,
				.m_logical_name = req.m_logical_name,
				.m_path = std::filesystem::absolute(req.m_logical_name),
				.m_provider = tool_provider::explicit_path,
				.m_reason = "explicit tool path from selected toolchain",
				.m_priority = 0,
				.m_authoritative = true
			});
		}
	}

	inline void add_cli_override_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		const auto override = cli_override_for(ctx.options(), req.m_role);

		if (override.empty()) {
			return;
		}

		if (is_explicit_path(override)) {
			out.push_back({
				.m_role = req.m_role,
				.m_logical_name = override,
				.m_path = std::filesystem::absolute(override),
				.m_provider = tool_provider::cli_override,
				.m_reason = "CLI tool override",
				.m_priority = 10,
				.m_authoritative = true
			});
			return;
		}

		for (const auto& dir : path_entries()) {
			add_candidate_if_found(out, req, override, dir, tool_provider::cli_override, 10, "CLI tool override searched on PATH");
		}
	}

	inline void add_environment_override_candidates(context&, const tool_requirement& req, candidate_list& out) {
		const auto variable = environment_variable_for(req.m_role);

		if (variable.empty()) {
			return;
		}

		const auto value = getenv_string(variable);

		if (!value || value->empty()) {
			return;
		}

		if (is_explicit_path(*value)) {
			out.push_back({
				.m_role = req.m_role,
				.m_logical_name = *value,
				.m_path = std::filesystem::absolute(*value),
				.m_provider = tool_provider::environment_override,
				.m_reason = variable,
				.m_priority = 20,
				.m_authoritative = true
			});
			return;
		}

		for (const auto& dir : path_entries()) {
			add_candidate_if_found(out, req, *value, dir, tool_provider::environment_override, 20, "environment override " + variable);
		}
	}

	inline void add_cache_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		if (!ctx.m_use_cache || ctx.m_refresh_cache) {
			return;
		}

		if (auto entry = ctx.m_cache.find(
				ctx.toolchain().name(),
				sys::g_host_target,
				ctx.request().target(),
				req.m_role,
				req.m_logical_name
			)) {
			out.push_back({
				req.m_role,
				req.m_logical_name,
				entry->m_path,
				tool_provider::cache,
				"validated build directory cache entry",
				30
			});
		}
	}

	inline void add_root_candidates(
		context& ctx,
		const tool_requirement& req,
		candidate_list& out
	) {
		std::vector<std::filesystem::path> package_dirs;
		std::vector<std::filesystem::path> project_dirs;
		std::vector<std::filesystem::path> toolchain_dirs;
		std::vector<std::filesystem::path> sdk_dirs;
		std::vector<std::filesystem::path> sysroot_dirs;

		if (!ctx.options().m_package_toolchain_root.empty()) package_dirs.emplace_back(std::filesystem::path{ctx.options().m_package_toolchain_root} / "bin");
		if (ctx.toolchain().m_package_root.has_value()) package_dirs.emplace_back(std::filesystem::path{*ctx.toolchain().m_package_root} / "bin");
		if (!package_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, package_dirs, tool_provider::package_toolchain, 40, "package-provided toolchain root", true, ctx.m_mode);

		for (const auto& root : ctx.toolchain().m_search_roots) project_dirs.emplace_back(root);
		if (!project_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, project_dirs, tool_provider::project_toolchain_root, 50, "project toolchain search root", true, ctx.m_mode);

		if (!ctx.options().m_toolchain_root.empty()) toolchain_dirs.emplace_back(std::filesystem::path{ctx.options().m_toolchain_root} / "bin");
		if (ctx.toolchain().m_toolchain_root.has_value()) toolchain_dirs.emplace_back(std::filesystem::path{*ctx.toolchain().m_toolchain_root} / "bin");
		if (!toolchain_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, toolchain_dirs, tool_provider::toolchain_root, 60, "toolchain root", true, ctx.m_mode);

		if (!ctx.options().m_sdk_root.empty()) sdk_dirs.emplace_back(std::filesystem::path{ctx.options().m_sdk_root} / "bin");
		if (ctx.toolchain().m_sdk_root.has_value()) sdk_dirs.emplace_back(std::filesystem::path{*ctx.toolchain().m_sdk_root} / "bin");
		if (!sdk_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, sdk_dirs, tool_provider::sdk_root, 70, "SDK root", true, ctx.m_mode);

		if (!ctx.options().m_sysroot.empty()) sysroot_dirs.emplace_back(std::filesystem::path{ctx.options().m_sysroot} / "bin");
		if (ctx.toolchain().sysroot().has_value()) sysroot_dirs.emplace_back(std::filesystem::path{*ctx.toolchain().sysroot()} / "bin");
		if (!sysroot_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, sysroot_dirs, tool_provider::sysroot_root, 80, "sysroot bin directory", true, ctx.m_mode);
	}

	inline void add_sibling_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;

		for (const auto& tool : ctx.m_resolved_tools) {
			if (tool.m_path.has_parent_path()) {
				dirs.emplace_back(tool.m_path.parent_path());
			}
		}

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::sibling, 90, "sibling of an already resolved tool", true, ctx.m_mode);
	}

	inline void add_target_prefix_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		for (const auto& prefixed_name : target_prefixed_names_for(ctx.request(), req.m_role, ctx.m_mode)) {
			for (const auto& dir : path_entries()) {
				add_candidate_if_found(out, req, prefixed_name, dir, tool_provider::target_prefix, 100, "target-prefixed tool on PATH");
			}
		}
	}

	inline void add_path_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		add_candidates_from_dirs(out, ctx.request(), req, path_entries(), tool_provider::path, 110, "PATH", true, ctx.m_mode);
	}

	inline void add_known_install_root_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;

#if defined(_WIN32)
		dirs.emplace_back("C:/Program Files/LLVM/bin");
		dirs.emplace_back("C:/Program Files (x86)/LLVM/bin");
		dirs.emplace_back("C:/Tools/Ninja");
#elif defined(__APPLE__)
		dirs.emplace_back("/opt/homebrew/bin");
		dirs.emplace_back("/usr/local/bin");
		dirs.emplace_back("/opt/local/bin");
#else
		dirs.emplace_back("/usr/local/bin");
		dirs.emplace_back("/usr/bin");
		dirs.emplace_back("/bin");
		dirs.emplace_back("/opt/bin");
#endif

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::known_install_root, 120, "known install root", true, ctx.m_mode);
	}

	void add_windows_registry_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_vswhere_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_visual_studio_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_windows_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_standalone_llvm_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_msys2_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_mingw_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_cygwin_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_xcrun_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_homebrew_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_macports_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_unix_system_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_distro_llvm_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_distro_gcc_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_android_ndk_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_emscripten_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_embedded_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out);

	[[nodiscard]] inline candidate_list candidates_for(context& ctx, const tool_requirement& req) {
		candidate_list out;

		add_explicit_path_candidates(ctx, req, out);
		add_cli_override_candidates(ctx, req, out);
		add_environment_override_candidates(ctx, req, out);
		add_cache_candidates(ctx, req, out);
		add_root_candidates(ctx, req, out);
		add_sibling_candidates(ctx, req, out);
		add_target_prefix_candidates(ctx, req, out);
		add_path_candidates(ctx, req, out);
		add_known_install_root_candidates(ctx, req, out);
		add_windows_registry_candidates(ctx, req, out);
		add_vswhere_candidates(ctx, req, out);
		add_visual_studio_candidates(ctx, req, out);
		add_windows_sdk_candidates(ctx, req, out);
		add_standalone_llvm_candidates(ctx, req, out);
		add_msys2_candidates(ctx, req, out);
		add_mingw_candidates(ctx, req, out);
		add_cygwin_candidates(ctx, req, out);
		add_xcrun_candidates(ctx, req, out);
		add_homebrew_candidates(ctx, req, out);
		add_macports_candidates(ctx, req, out);
		add_unix_system_candidates(ctx, req, out);
		add_distro_llvm_candidates(ctx, req, out);
		add_distro_gcc_candidates(ctx, req, out);
		add_android_ndk_candidates(ctx, req, out);
		add_emscripten_sdk_candidates(ctx, req, out);
		add_embedded_sdk_candidates(ctx, req, out);

		std::ranges::sort(out, {}, &tool_candidate::m_priority);
		return out;
	}
}

#endif
// ===== end include/mgmake/discovery/providers.hxx =====


// ===== begin include/mgmake/discovery/validate.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_VALIDATE_HXX
#define MGMAKE_DISCOVERY_VALIDATE_HXX

// skipped duplicate include: include/mgmake/sys/command_line.hxx
// skipped duplicate include: include/mgmake/sys/util.hxx
// skipped duplicate include: include/mgmake/discovery/context.hxx
// skipped duplicate include: include/mgmake/discovery/filesystem.hxx

#include <chrono>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>

namespace mgmake::discovery {
	[[nodiscard]] inline std::expected<std::string, std::string> capture_command(
		sys::command_line command
	) {
		const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
		auto output_path = std::filesystem::temp_directory_path()
			/ ("mgmake-tool-probe-" + std::to_string(now) + ".txt");

		std::string shell = command.full_command();
		shell += " > ";
		shell += sys::shell_escape(output_path.string());
		shell += " 2>&1";

#if defined(MGMK_PLATFORM_WINDOWS)
		std::string shell_command;
		shell_command.reserve(shell.size() + 2);
		shell_command += '"';
		shell_command += shell;
		shell_command += '"';
		const int exit_code = std::system(shell_command.c_str());
#else
		const int exit_code = std::system(shell.c_str());
#endif

		std::ifstream in(output_path);
		std::string text;

		if (in.is_open()) {
			text.assign(
				std::istreambuf_iterator<char>{in},
				std::istreambuf_iterator<char>{}
			);
		}

		std::error_code ec;
		std::filesystem::remove(output_path, ec);

		if (exit_code != 0 && text.empty()) {
			return std::unexpected{"version probe failed"};
		}

		return text;
	}

	[[nodiscard]] inline std::expected<std::string, std::string> probe_version(
		const tool_candidate& candidate
	) {
		for (std::string flag : {"--version", "-v", "/?"}) {
			sys::command_line command;
			command.m_args.emplace_back(candidate.m_path.string());
			command.m_args.emplace_back(std::move(flag));

			auto result = capture_command(std::move(command));

			if (result && !result->empty()) {
				return result;
			}
		}

		return std::string{};
	}

	[[nodiscard]] inline tool_family classify_tool_family(
		const tool_requirement& req,
		const tool_candidate& candidate,
		std::string_view version_output
	) {
		const auto filename = candidate.m_path.filename().string();
		const auto text = std::string{version_output};

		if (filename.find("clang-cl") != std::string::npos) return tool_family::clang_cl;
		if (filename == "cl.exe" || filename == "cl") return tool_family::msvc;
		if (filename.find("llvm-") != std::string::npos) return tool_family::llvm_binutils;
		if (filename == "lib.exe" || filename == "lib") return tool_family::msvc_binutils;
		if (text.find("Apple clang") != std::string::npos) return tool_family::apple_clang;
		if (text.find("clang") != std::string::npos || filename.find("clang") != std::string::npos) return tool_family::clang;
		if (text.find("gcc") != std::string::npos || filename.find("gcc") != std::string::npos || filename.find("g++") != std::string::npos) return tool_family::gcc;
		if (filename.find("windres") != std::string::npos || filename.find("mingw") != std::string::npos) return tool_family::mingw;

		return req.m_expected_family;
	}

	[[nodiscard]] inline std::expected<resolved_tool, std::string> validate_candidate(
		context&,
		const tool_requirement& req,
		const tool_candidate& candidate
	) {
		if (!is_launchable_file(candidate.m_path)) {
			return std::unexpected{"path is not a launchable file"};
		}

		if (req.m_role == tool_role::generator_ninja) {
			const auto stem = candidate.m_path.stem().string();

			if (stem != "ninja") {
				return std::unexpected{"cached or discovered path is not ninja"};
			}
		}

		std::string version;
#if !defined(_WIN32)

		if (auto probe = probe_version(candidate)) {
			version = *probe;
		}
#endif

		resolved_tool result{};
		result.m_role = candidate.m_role;
		result.m_logical_name = candidate.m_logical_name;
		result.m_path = candidate.m_path;
		result.m_provider = candidate.m_provider;
		result.m_reason = candidate.m_reason;
		result.m_version = version;
		result.m_family = classify_tool_family(req, candidate, version);
		result.m_target_triple = req.m_target_triple;
		result.m_provider_root = candidate.m_provider_root;
		return result;
	}
}

#endif
// ===== end include/mgmake/discovery/validate.hxx =====


#include <ranges>
#include <sstream>
#include <utility>

namespace mgmake::discovery::windows {
	struct visual_studio_instance {
		std::filesystem::path m_root{};
		std::string m_version{};
		std::string m_display_name{};
		bool m_has_cpp_tools = false;
	};

	[[nodiscard]] inline std::optional<std::filesystem::path> find_vswhere() {
		if (auto override = getenv_path("MGMK_VSWHERE")) {
			if (is_launchable_file(*override)) return std::filesystem::absolute(*override);
		}

		for (const auto& dir : path_entries()) {
			if (auto found = find_in_directory(dir, "vswhere")) return found;
		}

		std::vector<std::filesystem::path> candidates;

		if (auto pf86 = getenv_path("ProgramFiles(x86)")) {
			candidates.emplace_back(*pf86 / "Microsoft Visual Studio" / "Installer" / "vswhere.exe");
		}

		if (auto pf = getenv_path("ProgramFiles")) {
			candidates.emplace_back(*pf / "Microsoft Visual Studio" / "Installer" / "vswhere.exe");
		}

		for (const auto& candidate : candidates) {
			if (is_launchable_file(candidate)) return std::filesystem::absolute(candidate);
		}

		return std::nullopt;
	}

	[[nodiscard]] inline std::vector<visual_studio_instance> common_visual_studio_instances() {
		std::vector<visual_studio_instance> result;

#if defined(_WIN32)
		std::vector<std::filesystem::path> roots;
		if (auto pf = getenv_path("ProgramFiles")) roots.emplace_back(*pf / "Microsoft Visual Studio" / "2022");
		if (auto pf86 = getenv_path("ProgramFiles(x86)")) roots.emplace_back(*pf86 / "Microsoft Visual Studio" / "2019");

		for (const auto& year_root : roots) {
			for (std::string edition : {"Community", "Professional", "Enterprise", "BuildTools"}) {
				auto root = year_root / edition;
				if (std::filesystem::exists(root)) {
					result.push_back({root, {}, edition, std::filesystem::exists(root / "VC" / "Tools" / "MSVC")});
				}
			}
		}
#endif

		return result;
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> vswhere_installation_paths() {
		std::vector<std::filesystem::path> result;

#if defined(_WIN32)
		auto vswhere = find_vswhere();

		if (!vswhere) {
			return result;
		}

		sys::command_line command;
		command.m_args.emplace_back(vswhere->string());
		command.m_args.emplace_back("-all");
		command.m_args.emplace_back("-products");
		command.m_args.emplace_back("*");
		command.m_args.emplace_back("-requires");
		command.m_args.emplace_back("Microsoft.VisualStudio.Component.VC.Tools.x86.x64");
		command.m_args.emplace_back("-property");
		command.m_args.emplace_back("installationPath");

		auto text = capture_command(std::move(command));

		if (!text) {
			return result;
		}

		std::istringstream in{*text};
		std::string line;

		while (std::getline(in, line)) {
			while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
				line.pop_back();
			}

			if (!line.empty() && std::filesystem::exists(line)) {
				result.emplace_back(line);
			}
		}
#endif

		return result;
	}

	[[nodiscard]] inline bool same_existing_path(
		const std::filesystem::path& left,
		const std::filesystem::path& right
	) {
		std::error_code ec;
		const bool same = std::filesystem::equivalent(left, right, ec);
		return !ec && same;
	}

	[[nodiscard]] inline std::vector<visual_studio_instance> visual_studio_instances() {
		std::vector<visual_studio_instance> result;

		for (const auto& path : vswhere_installation_paths()) {
			result.push_back({
				.m_root = path,
				.m_display_name = path.filename().string(),
				.m_has_cpp_tools = std::filesystem::exists(path / "VC" / "Tools" / "MSVC")
			});
		}

		for (auto fallback : common_visual_studio_instances()) {
			const bool already_added = std::ranges::any_of(result, [&](const auto& instance) {
				return same_existing_path(instance.m_root, fallback.m_root);
			});

			if (!already_added) {
				result.emplace_back(std::move(fallback));
			}
		}

		return result;
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> visual_studio_tool_dirs(
		const visual_studio_instance& vs
	) {
		std::vector<std::filesystem::path> result;
		auto msvc_root = vs.m_root / "VC" / "Tools" / "MSVC";
		std::error_code ec;

		if (std::filesystem::exists(msvc_root, ec)) {
			for (const auto& entry : std::filesystem::directory_iterator(msvc_root, ec)) {
				if (!entry.is_directory()) continue;
				const auto root = entry.path() / "bin";
				result.emplace_back(root / "Hostx64" / "x64");
				result.emplace_back(root / "Hostx64" / "x86");
				result.emplace_back(root / "Hostx64" / "arm64");
				result.emplace_back(root / "Hostx86" / "x86");
				result.emplace_back(root / "Hostx86" / "x64");
			}
		}

		result.emplace_back(vs.m_root / "VC" / "Tools" / "Llvm" / "bin");
		result.emplace_back(vs.m_root / "VC" / "Tools" / "Llvm" / "x64" / "bin");
		return result;
	}

	[[nodiscard]] inline tool_environment visual_studio_environment(
		const visual_studio_instance& vs,
		const build::request& req
	) {
		tool_environment result;
		auto vcvarsall = vs.m_root / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat";

		if (is_launchable_file(vcvarsall)) {
			result.m_setup_script = vcvarsall;

			switch (req.target().m_arch) {
				case sys::arch::x86: result.m_setup_args.emplace_back("x86"); break;
				case sys::arch::aarch64: result.m_setup_args.emplace_back("arm64"); break;
				default: result.m_setup_args.emplace_back("x64"); break;
			}
		}

		return result;
	}
}

namespace mgmake::discovery {
	inline void add_vswhere_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		if (req.m_logical_name != "vswhere") {
			return;
		}

		if (auto vswhere = windows::find_vswhere()) {
			out.push_back({req.m_role, req.m_logical_name.empty() ? "vswhere" : req.m_logical_name, *vswhere, tool_provider::vswhere, "Visual Studio locator", 140});
		}
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_visual_studio_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		for (const auto& vs : windows::visual_studio_instances()) {
			add_candidates_from_dirs(out, ctx.request(), req, windows::visual_studio_tool_dirs(vs), tool_provider::visual_studio, 150, "Visual Studio tool directory", true, ctx.m_mode, vs.m_root);
		}
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_standalone_llvm_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_LLVM_ROOT")) dirs.emplace_back(*root / "bin");
		dirs.emplace_back("C:/Program Files/LLVM/bin");
		dirs.emplace_back("C:/Program Files (x86)/LLVM/bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::standalone_llvm, 170, "standalone LLVM installation", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}
}

#endif
// ===== end include/mgmake/discovery/windows/visual_studio.hxx =====


namespace mgmake::discovery {
	[[nodiscard]] inline tool_environment discover_tool_environment(
		const cli::options&,
		const build::request& req,
		const spec::project&
	) {
		tool_environment env;

#if defined(_WIN32)
		if (req.toolchain().dialect() == build::toolchain::dialect::msvc) {
			if (getenv_string("VCINSTALLDIR") && getenv_string("WindowsSdkDir")) {
				return env;
			}

			for (const auto& vs : windows::visual_studio_instances()) {
				auto candidate = windows::visual_studio_environment(vs, req);
				if (!candidate.empty()) {
					return candidate;
				}
			}
		}
#endif

		return env;
	}
}

#endif
// ===== end include/mgmake/discovery/environment_provider.hxx =====

// skipped duplicate include: include/mgmake/discovery/providers.hxx
// skipped duplicate include: include/mgmake/discovery/validate.hxx

#include <expected>
#include <string>

namespace mgmake::discovery {
	[[nodiscard]] inline discovery::mode effective_discovery_mode(
		const cli::options& opts,
		const build::toolchain& tc
	) {
		if (opts.m_tool_discovery == discovery::mode::automatic) {
			return tc.discovery_mode();
		}

		return opts.m_tool_discovery;
	}

	[[nodiscard]] inline std::expected<resolved_tool, std::string> resolve_tool(
		context& ctx,
		const tool_requirement& req
	) {
		diagnostic diag;
		diag.m_role = req.m_role;
		diag.m_toolchain = std::string{ctx.toolchain().name()};
		diag.m_logical_name = req.m_logical_name;
		diag.m_needed_because = req.m_needed_because;

		for (const auto& candidate : candidates_for(ctx, req)) {
			diag.m_searched.emplace_back(
				std::string{name(candidate.m_provider)} + ": " + candidate.m_path.string()
			);

			auto validated = validate_candidate(ctx, req, candidate);

			if (validated) {
				ctx.m_resolved_tools.emplace_back(*validated);
				return validated;
			}

			diag.m_rejected.emplace_back(candidate.m_path.string() + ": " + validated.error());

			if (candidate.m_authoritative) {
				diag.m_fixes = fixes_for(req);
				return std::unexpected{diag.format_missing_tool()};
			}
		}

		if (req.required()) {
			diag.m_fixes = fixes_for(req);
			return std::unexpected{diag.format_missing_tool()};
		}

		return std::unexpected{std::string{}};
	}

	[[nodiscard]] inline cache_entry make_cache_entry(
		const build::request& req,
		const resolved_tool& tool
	) {
		cache_entry entry;
		entry.m_toolchain = std::string{req.toolchain().name()};
		entry.m_host = sys::g_host_target;
		entry.m_target = req.target();
		entry.m_host_key = target_key(entry.m_host);
		entry.m_target_key = target_key(entry.m_target);
		entry.m_role = tool.m_role;
		entry.m_logical_name = tool.m_logical_name;
		entry.m_path = tool.m_path;
		entry.m_provider = tool.m_provider;
		entry.m_version = tool.m_version;
		return entry;
	}

	[[nodiscard]] inline std::expected<build::request, std::string> resolve_request(
		const cli::options& opts,
		const build::request& req,
		const spec::project& project
	) {
		const auto discovery_mode = effective_discovery_mode(opts, req.toolchain());

		if (discovery_mode == mode::disabled) {
			return req;
		}

		context ctx;
		ctx.m_options = &opts;
		ctx.m_project = &project;
		ctx.m_request = &req;
		ctx.m_use_cache = !opts.m_no_tool_cache;
		ctx.m_refresh_cache = opts.m_refresh_tools;
		ctx.m_verbose = opts.m_verbose;
		ctx.m_show_search = opts.m_show_tool_search;
		ctx.m_mode = discovery_mode;

		if (ctx.m_use_cache) {
			ctx.m_cache = load_cache(req);
		}

		build::request resolved = req;

		for (const auto& requirement : required_tools(opts, req, project)) {
			auto tool = resolve_tool(ctx, requirement);

			if (!tool) {
				if (requirement.required()) {
					return std::unexpected{tool.error()};
				}

				continue;
			}

			resolved.m_discovered_tools.emplace_back(*tool);

			switch (tool->m_role) {
				case tool_role::c_compiler:
					resolved.m_tc.cc(tool->path_string());
					break;

				case tool_role::cxx_compiler:
					resolved.m_tc.cxx(tool->path_string());
					break;

				case tool_role::archiver:
				case tool_role::librarian:
					resolved.m_tc.ar(tool->path_string());
					break;

				case tool_role::linker:
				case tool_role::shared_linker:
					resolved.m_tc.linker(tool->path_string());
					break;

				default:
					resolved.m_tc.tool(tool->m_role, tool->path_string());
					break;
			}

			if (ctx.m_use_cache) {
				ctx.m_cache.put(make_cache_entry(req, *tool));
			}
		}

		resolved.m_tool_environment = discover_tool_environment(opts, resolved, project);

		if (ctx.m_use_cache) {
			save_cache(req, ctx.m_cache);
		}

		return resolved;
	}

	[[nodiscard]] inline std::expected<resolved_tool, std::string> resolve_backend_tool(
		const cli::options& opts,
		const build::request& req,
		backend_tool_requirement requirement
	) {
		const auto discovery_mode = effective_discovery_mode(opts, req.toolchain());

		if (discovery_mode == mode::disabled) {
			return resolved_tool{
				.m_role = requirement.m_role,
				.m_logical_name = requirement.m_logical_name,
				.m_path = requirement.m_logical_name,
				.m_provider = tool_provider::explicit_path,
				.m_reason = "tool discovery disabled"
			};
		}

		context ctx;
		ctx.m_options = &opts;
		ctx.m_request = &req;
		ctx.m_use_cache = !opts.m_no_tool_cache;
		ctx.m_refresh_cache = opts.m_refresh_tools;
		ctx.m_verbose = opts.m_verbose;
		ctx.m_show_search = opts.m_show_tool_search;
		ctx.m_mode = discovery_mode;

		if (ctx.m_use_cache) {
			ctx.m_cache = load_cache(req);
		}

		tool_requirement req_tool;
		req_tool.m_role = requirement.m_role;
		req_tool.m_logical_name = std::move(requirement.m_logical_name);
		req_tool.m_needed_because = std::move(requirement.m_needed_because);

		auto resolved = resolve_tool(ctx, req_tool);

		if (resolved && ctx.m_use_cache) {
			ctx.m_cache.put(make_cache_entry(req, *resolved));
			save_cache(req, ctx.m_cache);
		}

		return resolved;
	}
}

#endif
// ===== end include/mgmake/discovery/resolve.hxx =====

// skipped duplicate include: include/mgmake/sys/util.hxx

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <utility>

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

        void generate(const cli::options&, const dag::graph& graph, const build::request& req) const {
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
                auto command_text = discovery::wrap_command_for_environment(
                    req.tool_environment(),
                    action.m_command.full_command()
                );
                out << "  command = " << command_text << "\n";

                if (!action.m_description.empty()) {
                    out << "  description = " << detail::ninja_escape_variable_text(action.m_description) << "\n";
                } else if (!action.m_name.empty()) {
                    out << "  description = " << detail::ninja_escape_variable_text(action.m_name) << "\n";
                }

                if (!action.m_working_directory.empty()) {
#if defined(_WIN32)
                    command_text = "cd /d ";
#else
                    command_text = "cd ";
#endif
                    command_text += sys::shell_escape(action.m_working_directory.string());
                    command_text += " && ";
                    command_text += action.m_command.full_command();
                    command_text = discovery::wrap_command_for_environment(
                        req.tool_environment(),
                        std::move(command_text)
                    );
                    out << "  command = " << command_text << "\n";
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

		std::expected<void, std::string> build(
            const cli::options& opts,
            const dag::graph& graph,
            const build::request& req
        ) const {
            auto output_path = req.build_dir() / m_output_file;
            if (output_path.has_parent_path()) {
                std::filesystem::create_directories(output_path.parent_path());
            }
			generate(opts, graph, req);

            auto ninja = discovery::resolve_backend_tool(
                opts,
                req,
                discovery::backend_tool_requirement{
                    .m_role = discovery::tool_role::generator_ninja,
                    .m_logical_name = opts.m_ninja.empty() ? "ninja" : opts.m_ninja,
                    .m_needed_because = "the selected backend is ninja and this action invokes ninja"
                }
            );

            if (!ninja) {
                return std::unexpected{ninja.error()};
            }

			sys::command_line command;
			command.m_args.emplace_back(ninja->path_string());
			command.m_args.emplace_back("-f");
			command.m_args.emplace_back(output_path.string());

			for (const auto& target : req.m_targets) {
				command.m_args.emplace_back(target);
			}

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

#endif
// ===== end include/mgmake/backend/ninja.hxx =====


// ===== begin include/mgmake/backend/registry.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_REGISTRY_HXX
#define MGMAKE_BACKEND_REGISTRY_HXX

// skipped duplicate include: include/mgmake/backend/graphviz.hxx
// skipped duplicate include: include/mgmake/backend/ninja.hxx
// skipped duplicate include: include/mgmake/cli/backend.hxx

#include <type_traits>

namespace mgmake::backend {
	template <cli::backend_kind Kind>
	struct for_kind {
		using type = void;
	};

	template <>
	struct for_kind<cli::backend_kind::automatic> {
		using type = backend::ninja;
	};

	template <>
	struct for_kind<cli::backend_kind::ninja> {
		using type = backend::ninja;
	};

	template <>
	struct for_kind<cli::backend_kind::graphviz> {
		using type = backend::graphviz<>;
	};

	template <cli::backend_kind Kind>
	using for_kind_t = typename for_kind<Kind>::type;

	template <cli::backend_kind Kind>
	inline constexpr bool implemented_v =
		!std::is_void_v<for_kind_t<Kind>>;
}

#endif
// ===== end include/mgmake/backend/registry.hxx =====


// ===== begin include/mgmake/backend/capabilities.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_CAPABILITIES_HXX
#define MGMAKE_BACKEND_CAPABILITIES_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx

#include <concepts>
#include <expected>
#include <string>

namespace mgmake::backend {
	template <typename Backend>
	concept can_generate =
		requires(
			Backend backend,
			const cli::options& opts,
			const dag::graph& graph,
			const build::request& req
		) {
			backend.generate(opts, graph, req);
		};

	template <typename Backend>
	concept can_build =
		requires(
			Backend backend,
			const cli::options& opts,
			const dag::graph& graph,
			const build::request& req
		) {
			{ backend.build(opts, graph, req) } -> std::same_as<std::expected<void, std::string>>;
		};
}

#endif
// ===== end include/mgmake/backend/capabilities.hxx =====


// ===== begin include/mgmake/backend/execute.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_EXECUTE_HXX
#define MGMAKE_BACKEND_EXECUTE_HXX

// skipped duplicate include: include/mgmake/backend/capabilities.hxx
// skipped duplicate include: include/mgmake/backend/registry.hxx
// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/cli/action.hxx
// skipped duplicate include: include/mgmake/cli/backend.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx

#include <expected>
#include <string>
#include <type_traits>

namespace mgmake::backend {
	template <cli::backend_kind Kind>
	[[nodiscard]] inline std::expected<void, std::string> generate(
		const cli::options& opts,
		const dag::graph& graph,
		const build::request& req
	) {
		using backend_type = backend::for_kind_t<Kind>;

		if constexpr (std::is_void_v<backend_type>) {
			return std::unexpected{
				"mgmake: backend '" +
				std::string{ cli::backend_name(Kind) } +
				"' is not implemented yet"
			};
		} else if constexpr (backend::can_generate<backend_type>) {
			backend_type{}.generate(opts, graph, req);
			return {};
		} else {
			return std::unexpected{
				"mgmake: backend '" +
				std::string{ cli::backend_name(Kind) } +
				"' does not support action 'generate'"
			};
		}
	}

	template <cli::backend_kind Kind>
	[[nodiscard]] inline std::expected<void, std::string> build(
		const cli::options& opts,
		const dag::graph& graph,
		const build::request& req
	) {
		using backend_type = backend::for_kind_t<Kind>;

		if constexpr (std::is_void_v<backend_type>) {
			return std::unexpected{
				"mgmake: backend '" +
				std::string{ cli::backend_name(Kind) } +
				"' is not implemented yet"
			};
		} else if constexpr (backend::can_build<backend_type>) {
			return backend_type{}.build(opts, graph, req);
		} else {
			return std::unexpected{
				"mgmake: backend '" +
				std::string{ cli::backend_name(Kind) } +
				"' does not support action 'build'"
			};
		}
	}

	template <cli::backend_kind Kind>
	[[nodiscard]] inline std::expected<void, std::string> execute_project_action_for_backend(
		const cli::options& opts,
		const build::request& req,
		const dag::graph& graph
	) {
		switch (opts.m_action) {
			case cli::action_kind::generate:
				return backend::generate<Kind>(opts, graph, req);

			case cli::action_kind::build:
				return backend::build<Kind>(opts, graph, req);

			case cli::action_kind::run:
				return std::unexpected{
					"mgmake: run action is not implemented yet"
				};

			case cli::action_kind::clean:
			case cli::action_kind::tools:
			case cli::action_kind::help:
			case cli::action_kind::version:
				return {};

			case cli::action_kind::count:
				break;
		}

		return std::unexpected{ "mgmake: unknown action" };
	}

	[[nodiscard]] inline std::expected<void, std::string> execute_project_action(
		const cli::options& opts,
		const build::request& req,
		const dag::graph& graph
	) {
		switch (opts.m_backend) {
			case cli::backend_kind::automatic:
				return execute_project_action_for_backend<
					cli::backend_kind::automatic
				>(opts, req, graph);

			case cli::backend_kind::ninja:
				return execute_project_action_for_backend<
					cli::backend_kind::ninja
				>(opts, req, graph);

			case cli::backend_kind::graphviz:
				return execute_project_action_for_backend<
					cli::backend_kind::graphviz
				>(opts, req, graph);

			case cli::backend_kind::make:
				return execute_project_action_for_backend<
					cli::backend_kind::make
				>(opts, req, graph);

			case cli::backend_kind::direct:
				return execute_project_action_for_backend<
					cli::backend_kind::direct
				>(opts, req, graph);

			case cli::backend_kind::count:
				break;
		}

		return std::unexpected{ "mgmake: unknown backend" };
	}
}

#endif
// ===== end include/mgmake/backend/execute.hxx =====

// skipped duplicate include: include/mgmake/spec/executable.hxx

// ===== begin include/mgmake/spec/executable_impl.hxx =====
#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_IMPL_HXX
#define MGMK_SPEC_EXECUTABLE_IMPL_HXX

// skipped duplicate include: include/mgmake/spec/executable.hxx

#endif
// ===== end include/mgmake/spec/executable_impl.hxx =====

// skipped duplicate include: include/mgmake/spec/library.hxx

// ===== begin include/mgmake/spec/library_impl.hxx =====
#pragma once

#ifndef MGMK_SPEC_LIBRARY_IMPL_HXX
#define MGMK_SPEC_LIBRARY_IMPL_HXX

// skipped duplicate include: include/mgmake/spec/library.hxx

#endif
// ===== end include/mgmake/spec/library_impl.hxx =====

// skipped duplicate include: include/mgmake/spec/project.hxx

// ===== begin include/mgmake/discovery/discovery.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_DISCOVERY_HXX
#define MGMAKE_DISCOVERY_DISCOVERY_HXX

// skipped duplicate include: include/mgmake/discovery/tool_role.hxx
// skipped duplicate include: include/mgmake/discovery/tool_provider.hxx
// skipped duplicate include: include/mgmake/discovery/mode.hxx
// skipped duplicate include: include/mgmake/discovery/tool_family.hxx
// skipped duplicate include: include/mgmake/discovery/tool_binding.hxx
// skipped duplicate include: include/mgmake/discovery/resolved_tool.hxx
// skipped duplicate include: include/mgmake/discovery/tool_environment.hxx
// skipped duplicate include: include/mgmake/discovery/tool_requirement.hxx
// skipped duplicate include: include/mgmake/discovery/backend_requirement.hxx
// skipped duplicate include: include/mgmake/discovery/environment.hxx
// skipped duplicate include: include/mgmake/discovery/filesystem.hxx
// skipped duplicate include: include/mgmake/discovery/tool_names.hxx
// skipped duplicate include: include/mgmake/discovery/cache.hxx
// skipped duplicate include: include/mgmake/discovery/context.hxx
// skipped duplicate include: include/mgmake/discovery/diagnostic.hxx
// skipped duplicate include: include/mgmake/discovery/validate.hxx
// skipped duplicate include: include/mgmake/discovery/providers.hxx

// ===== begin include/mgmake/discovery/windows/registry.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_WINDOWS_REGISTRY_HXX
#define MGMAKE_DISCOVERY_WINDOWS_REGISTRY_HXX

// skipped duplicate include: include/mgmake/discovery/providers.hxx

#include <optional>
#include <string>
#include <string_view>

#if defined(_WIN32)
	#include <windows.h>
	#pragma comment(lib, "Advapi32.lib")
#endif

namespace mgmake::discovery::windows {
#if defined(_WIN32)
	[[nodiscard]] inline std::optional<std::string> read_registry_string(
		HKEY hive,
		std::string_view key,
		std::string_view value_name,
		REGSAM view = KEY_WOW64_64KEY
	) {
		HKEY handle{};
		const std::string key_text{key};

		if (RegOpenKeyExA(hive, key_text.c_str(), 0, KEY_READ | view, &handle) != ERROR_SUCCESS) {
			return std::nullopt;
		}

		DWORD type = 0;
		DWORD size = 0;
		const std::string value_text{value_name};
		const char* value_ptr = value_text.empty() ? nullptr : value_text.c_str();

		if (RegQueryValueExA(handle, value_ptr, nullptr, &type, nullptr, &size) != ERROR_SUCCESS
			|| (type != REG_SZ && type != REG_EXPAND_SZ)) {
			RegCloseKey(handle);
			return std::nullopt;
		}

		std::string result(size, '\0');

		if (RegQueryValueExA(
				handle,
				value_ptr,
				nullptr,
				&type,
				reinterpret_cast<BYTE*>(result.data()),
				&size
			) != ERROR_SUCCESS) {
			RegCloseKey(handle);
			return std::nullopt;
		}

		RegCloseKey(handle);

		while (!result.empty() && result.back() == '\0') {
			result.pop_back();
		}

		return result;
	}
#endif
}

namespace mgmake::discovery {
	inline void add_windows_registry_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;

		if (auto kits = windows::read_registry_string(
				HKEY_LOCAL_MACHINE,
				R"(SOFTWARE\Microsoft\Windows Kits\Installed Roots)",
				"KitsRoot10"
			)) {
			dirs.emplace_back(std::filesystem::path{*kits} / "bin");
		}

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::windows_registry, 130, "Windows registry and registered install roots", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}
}

#endif
// ===== end include/mgmake/discovery/windows/registry.hxx =====

// skipped duplicate include: include/mgmake/discovery/windows/visual_studio.hxx

// ===== begin include/mgmake/discovery/windows/windows_sdk.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_WINDOWS_WINDOWS_SDK_HXX
#define MGMAKE_DISCOVERY_WINDOWS_WINDOWS_SDK_HXX

// skipped duplicate include: include/mgmake/discovery/providers.hxx

#include <functional>

namespace mgmake::discovery {
	[[nodiscard]] inline bool is_windows_sdk_role(tool_role role) noexcept {
		switch (role) {
			case tool_role::resource_compiler:
			case tool_role::manifest_tool:
			case tool_role::midl_compiler:
				return true;

			default:
				return false;
		}
	}

	inline void add_windows_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		if (!is_windows_sdk_role(req.m_role)) {
			return;
		}

		std::vector<std::filesystem::path> roots;

		if (auto root = getenv_path("WindowsSdkDir")) {
			roots.emplace_back(*root);
		}

		if (auto pf86 = getenv_path("ProgramFiles(x86)")) {
			roots.emplace_back(*pf86 / "Windows Kits" / "10");
		}

		if (auto pf = getenv_path("ProgramFiles")) {
			roots.emplace_back(*pf / "Windows Kits" / "10");
		}

		std::vector<std::filesystem::path> dirs;

		for (const auto& root : roots) {
			auto kits = root / "bin";
			std::error_code ec;

			if (std::filesystem::exists(kits, ec)) {
				std::vector<std::filesystem::path> versions;

				for (const auto& version : std::filesystem::directory_iterator(kits, ec)) {
					if (!version.is_directory()) continue;
					versions.emplace_back(version.path());
				}

				std::ranges::sort(versions, std::greater<>{});

				for (const auto& version : versions) {
					dirs.emplace_back(version / "x64");
					dirs.emplace_back(version / "x86");
					dirs.emplace_back(version / "arm64");
				}
			}
		}

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::windows_sdk, 160, "Windows SDK tool directory", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_msys2_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs = {
			"C:/msys64/ucrt64/bin",
			"C:/msys64/mingw64/bin",
			"C:/msys64/clang64/bin",
			"C:/msys64/mingw32/bin",
			"C:/msys64/clangarm64/bin"
		};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::msys2, 180, "MSYS2 toolchain root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_mingw_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_MINGW_ROOT")) dirs.emplace_back(*root / "bin");
		dirs.emplace_back("C:/MinGW/bin");
		dirs.emplace_back("C:/mingw64/bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::mingw, 190, "MinGW root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_cygwin_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs = {"C:/cygwin64/bin", "C:/cygwin/bin"};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::cygwin, 200, "Cygwin root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}
}

#endif
// ===== end include/mgmake/discovery/windows/windows_sdk.hxx =====


// ===== begin include/mgmake/discovery/macos/xcrun.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_MACOS_XCRUN_HXX
#define MGMAKE_DISCOVERY_MACOS_XCRUN_HXX

// skipped duplicate include: include/mgmake/discovery/providers.hxx
// skipped duplicate include: include/mgmake/discovery/validate.hxx

namespace mgmake::discovery {
	inline void add_xcrun_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(__APPLE__)
		for (const auto& logical : candidate_names_for(ctx.request(), req.m_role, ctx.m_mode)) {
			sys::command_line command;
			command.m_args.emplace_back("xcrun");
			if (!ctx.options().m_apple_sdk.empty()) {
				command.m_args.emplace_back("--sdk");
				command.m_args.emplace_back(ctx.options().m_apple_sdk);
			}
			command.m_args.emplace_back("--find");
			command.m_args.emplace_back(logical);

			auto found = capture_command(command);
			if (found && !found->empty()) {
				std::string path = *found;
				while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) path.pop_back();
				if (is_launchable_file(path)) {
					out.push_back({req.m_role, logical, path, tool_provider::xcrun, "xcrun --find", 210});
				}
			}
		}
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_homebrew_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(__APPLE__)
		std::vector<std::filesystem::path> dirs = {
			"/opt/homebrew/opt/llvm/bin",
			"/usr/local/opt/llvm/bin",
			"/opt/homebrew/bin",
			"/usr/local/bin"
		};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::homebrew, 220, "Homebrew tool root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_macports_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(__APPLE__)
		add_candidates_from_dirs(out, ctx.request(), req, {"/opt/local/bin"}, tool_provider::macports, 230, "MacPorts tool root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}
}

#endif
// ===== end include/mgmake/discovery/macos/xcrun.hxx =====


// ===== begin include/mgmake/discovery/unix/unix_tools.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_UNIX_UNIX_TOOLS_HXX
#define MGMAKE_DISCOVERY_UNIX_UNIX_TOOLS_HXX

// skipped duplicate include: include/mgmake/discovery/providers.hxx

namespace mgmake::discovery {
	inline void add_unix_system_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(MGMK_PLATFORM_POSIX)
		std::vector<std::filesystem::path> dirs = {"/usr/local/bin", "/usr/bin", "/bin", "/opt/bin", "/opt/local/bin"};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::unix_system, 240, "Unix system tool root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_distro_llvm_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(MGMK_PLATFORM_POSIX)
		std::vector<std::filesystem::path> dirs;
		for (int version = 30; version >= 10; --version) {
			dirs.emplace_back("/usr/lib/llvm-" + std::to_string(version) + "/bin");
			dirs.emplace_back("/usr/local/llvm-" + std::to_string(version) + "/bin");
		}
		dirs.emplace_back("/opt/llvm/bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::distro_llvm, 250, "distro LLVM root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_distro_gcc_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(MGMK_PLATFORM_POSIX)
		std::vector<std::filesystem::path> dirs = {"/usr/bin", "/usr/local/bin"};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::distro_gcc, 260, "distro GCC root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}
}

#endif
// ===== end include/mgmake/discovery/unix/unix_tools.hxx =====


// ===== begin include/mgmake/discovery/android/ndk.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_ANDROID_NDK_HXX
#define MGMAKE_DISCOVERY_ANDROID_NDK_HXX

// skipped duplicate include: include/mgmake/discovery/providers.hxx

namespace mgmake::discovery {
	[[nodiscard]] inline std::string android_host_tag() {
#if defined(_WIN32)
		return "windows-x86_64";
#elif defined(__APPLE__)
		#if defined(__aarch64__)
			return "darwin-arm64";
		#else
			return "darwin-x86_64";
		#endif
#else
		return "linux-x86_64";
#endif
	}

	inline void add_android_ndk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> roots;
		if (!ctx.options().m_android_ndk.empty()) roots.emplace_back(ctx.options().m_android_ndk);
		if (auto root = getenv_path("ANDROID_NDK_ROOT")) roots.emplace_back(*root);
		if (auto root = getenv_path("ANDROID_NDK_HOME")) roots.emplace_back(*root);

		for (const auto& root : roots) {
			auto bin = root / "toolchains" / "llvm" / "prebuilt" / android_host_tag() / "bin";
			add_candidates_from_dirs(out, ctx.request(), req, {bin}, tool_provider::android_ndk, 270, "Android NDK LLVM toolchain", true, ctx.m_mode);
		}
	}

	inline void add_emscripten_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("EMSDK")) {
			dirs.emplace_back(*root / "upstream" / "emscripten");
			dirs.emplace_back(*root / "upstream" / "bin");
		}
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::emscripten_sdk, 280, "Emscripten SDK root", true, ctx.m_mode);
	}

	inline void add_embedded_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_EMBEDDED_SDK")) dirs.emplace_back(*root / "bin");
		if (auto root = getenv_path("ARM_GCC_ROOT")) dirs.emplace_back(*root / "bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::embedded_sdk, 290, "embedded SDK root", true, ctx.m_mode);
	}
}

#endif
// ===== end include/mgmake/discovery/android/ndk.hxx =====

// skipped duplicate include: include/mgmake/discovery/environment_provider.hxx
// skipped duplicate include: include/mgmake/discovery/resolve.hxx

// ===== begin include/mgmake/discovery/print_tools.hxx =====
#pragma once

#ifndef MGMAKE_DISCOVERY_PRINT_TOOLS_HXX
#define MGMAKE_DISCOVERY_PRINT_TOOLS_HXX

// skipped duplicate include: include/mgmake/discovery/resolve.hxx

#include <expected>
#include <print>
#include <string>

namespace mgmake::discovery {
	inline void print_resolved_tool(const resolved_tool& tool) {
		std::println("  {}", name(tool.m_role));
		std::println("    logical: {}", tool.m_logical_name);
		std::println("    path: {}", tool.m_path.string());
		std::println("    provider: {}", name(tool.m_provider));

		if (tool.m_family != tool_family::unknown) {
			std::println("    family: {}", discovery::name(tool.m_family));
		}

		if (!tool.m_version.empty()) {
			std::println("    version: {}", tool.m_version.substr(0, tool.m_version.find('\n')));
		}
	}

	[[nodiscard]] inline std::expected<void, std::string> print_tools(
		const cli::options& opts,
		const build::request& req,
		const spec::project& project
	) {
		auto resolved_req = resolve_request(opts, req, project);

		if (!resolved_req) {
			return std::unexpected{resolved_req.error()};
		}

		std::println("mgmake tools\n");
		std::println("request:");
		std::println("  host: {}", target_key(sys::g_host_target));
		std::println("  target: {}", target_key(resolved_req->target()));
		std::println("  toolchain: {}", resolved_req->toolchain().name());
		std::println("  backend: {}", cli::backend_name(opts.m_backend));
		std::println("  discovery mode: {}\n", discovery::name(opts.m_tool_discovery));

		std::println("target tools:");
		for (const auto& tool : resolved_req->m_discovered_tools) {
			print_resolved_tool(tool);
		}

		std::println("\nbackend tools:");

		if (opts.m_backend == cli::backend_kind::automatic || opts.m_backend == cli::backend_kind::ninja) {
			auto ninja = resolve_backend_tool(
				opts,
				*resolved_req,
				backend_tool_requirement{
					.m_role = tool_role::generator_ninja,
					.m_logical_name = opts.m_ninja.empty() ? "ninja" : opts.m_ninja,
					.m_needed_because = "the selected backend is ninja"
				}
			);

			if (!ninja) return std::unexpected{ninja.error()};
			print_resolved_tool(*ninja);
		}

		std::println("\nenvironment:");
		if (resolved_req->m_tool_environment.empty()) {
			std::println("  none");
		} else if (resolved_req->m_tool_environment.m_setup_script.has_value()) {
			std::println("  setup script: {}", resolved_req->m_tool_environment.m_setup_script->string());
		}

		std::println("\ncache:");
		std::println("  {}", cache_path(*resolved_req).string());
		return {};
	}
}

#endif
// ===== end include/mgmake/discovery/print_tools.hxx =====


#endif
// ===== end include/mgmake/discovery/discovery.hxx =====


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
			const bool is_c_source = source.extension().string() == ".c";
			command.m_args.emplace_back(is_c_source ? tc.cc() : tc.cxx());

			build::append_target_args(command, tc, request());

			for (const auto& flag : tc.compile_flags()) {
				command.m_args.emplace_back(flag);
			}

			if (is_c_source) {
				for (const auto& flag : tc.c_flags()) {
					command.m_args.emplace_back(flag);
				}
			} else {
				for (const auto& flag : tc.cxx_flags()) {
					command.m_args.emplace_back(flag);
				}
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


// ===== begin include/mgmake/detail/project_factory.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_PROJECT_FACTORY_HXX
#define MGMAKE_DETAIL_PROJECT_FACTORY_HXX

// skipped duplicate include: include/mgmake/cli/options.hxx
// skipped duplicate include: include/mgmake/spec/project.hxx

#include <functional>
#include <type_traits>
#include <utility>

namespace mgmake::detail {
	template <typename>
	inline constexpr bool always_false_v = false;

	template <typename ProjectFactory>
	[[nodiscard]] inline constexpr spec::project make_project(
		ProjectFactory&& factory,
		const cli::options& opts
	) {
		if constexpr (std::is_invocable_r_v<
			spec::project,
			ProjectFactory&&,
			const cli::options&
		>) {
			return std::invoke(
				std::forward<ProjectFactory>(factory),
				opts
			);
		} else if constexpr (std::is_invocable_r_v<
			spec::project,
			ProjectFactory&&
		>) {
			return std::invoke(std::forward<ProjectFactory>(factory));
		} else {
			static_assert(
				always_false_v<ProjectFactory>,
				"mgmake entry: project factory must return mgmake::spec::project"
			);
		}
	}
}

#endif
// ===== end include/mgmake/detail/project_factory.hxx =====


// ===== begin include/mgmake/entry/exit_code.hxx =====
#pragma once

#ifndef MGMAKE_ENTRY_EXIT_CODE_HXX
#define MGMAKE_ENTRY_EXIT_CODE_HXX

namespace mgmake::detail {
	inline constexpr int entry_exit_success = 0;
	inline constexpr int entry_exit_action_failure = 1;
	inline constexpr int entry_exit_usage_error = 2;
}

#endif
// ===== end include/mgmake/entry/exit_code.hxx =====


// ===== begin include/mgmake/entry/entry.hxx =====
#pragma once

#ifndef MGMAKE_ENTRY_ENTRY_HXX
#define MGMAKE_ENTRY_ENTRY_HXX

// skipped duplicate include: include/mgmake/backend/execute.hxx
// skipped duplicate include: include/mgmake/build/clean.hxx
// skipped duplicate include: include/mgmake/build/request_from_options.hxx
// skipped duplicate include: include/mgmake/build/toolchain_registry.hxx
// skipped duplicate include: include/mgmake/cli/help.hxx
// skipped duplicate include: include/mgmake/cli/parse.hxx
// skipped duplicate include: include/mgmake/detail/project_factory.hxx
// skipped duplicate include: include/mgmake/discovery/discovery.hxx
// skipped duplicate include: include/mgmake/spec/project.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx
// skipped duplicate include: include/mgmake/entry/exit_code.hxx

#include <print>
#include <type_traits>
#include <utility>

namespace mgmake {
	template <build::toolchain_registry_like Toolchains>
	[[nodiscard]] inline int entry(
		const sys::command_line& command_line,
		const Toolchains& toolchains
	) {
		auto parsed = cli::parse(command_line.user_args());

		if (!parsed) {
			std::println(stderr, "mgmake: error: {}", parsed.m_error);
			std::println(stderr, "try '{} help'", command_line.program_name());
			return detail::entry_exit_usage_error;
		}

		const cli::options& opts = parsed.m_value;

		if (opts.m_show_help || opts.m_action == cli::action_kind::help) {
			cli::print_help(command_line.program_name(), toolchains);
			return detail::entry_exit_success;
		}

		if (opts.m_show_version || opts.m_action == cli::action_kind::version) {
			std::println("mgmake");
			return detail::entry_exit_success;
		}

		auto req_result = build::request_from_options(opts, toolchains);

		if (!req_result) {
			std::println(stderr, "{}", req_result.error());
			return detail::entry_exit_usage_error;
		}

		auto req = std::move(*req_result);

		if (opts.m_action == cli::action_kind::clean) {
			const auto clean_result = build::clean(req);

			if (!clean_result) {
				std::println(stderr, "{}", clean_result.error());
				return detail::entry_exit_action_failure;
			}

			return detail::entry_exit_success;
		}

		std::println(
			stderr,
			"mgmake: error: action '{}' requires a project, but no project was provided",
			cli::action_name(opts.m_action)
		);

		return detail::entry_exit_usage_error;
	}

	[[nodiscard]] inline int entry(const sys::command_line& command_line) {
		return entry(command_line, build::default_toolchains);
	}

	template <
		typename ProjectFactory,
		build::toolchain_registry_like Toolchains
	>
	[[nodiscard]] inline int entry(
		const sys::command_line& command_line,
		ProjectFactory&& project_factory,
		const Toolchains& toolchains
	) {
		auto parsed = cli::parse(command_line.user_args());

		if (!parsed) {
			std::println(stderr, "mgmake: error: {}", parsed.m_error);
			std::println(stderr, "try '{} help'", command_line.program_name());
			return detail::entry_exit_usage_error;
		}

		const cli::options& opts = parsed.m_value;

		if (opts.m_show_help || opts.m_action == cli::action_kind::help) {
			cli::print_help(command_line.program_name(), toolchains);
			return detail::entry_exit_success;
		}

		if (opts.m_show_version || opts.m_action == cli::action_kind::version) {
			std::println("mgmake");
			return detail::entry_exit_success;
		}

		auto req_result = build::request_from_options(opts, toolchains);

		if (!req_result) {
			std::println(stderr, "{}", req_result.error());
			return detail::entry_exit_usage_error;
		}

		auto req = std::move(*req_result);

		if (opts.m_action == cli::action_kind::clean) {
			const auto clean_result = build::clean(req);

			if (!clean_result) {
				std::println(stderr, "{}", clean_result.error());
				return detail::entry_exit_action_failure;
			}

			return detail::entry_exit_success;
		}

		auto proj = detail::make_project(
			std::forward<ProjectFactory>(project_factory),
			opts
		);

		if (opts.m_action == cli::action_kind::tools) {
			const auto tools_result = discovery::print_tools(opts, req, proj);

			if (!tools_result) {
				std::println(stderr, "{}", tools_result.error());
				return detail::entry_exit_usage_error;
			}

			return detail::entry_exit_success;
		}

		auto resolved_req_result = discovery::resolve_request(opts, req, proj);

		if (!resolved_req_result) {
			std::println(stderr, "{}", resolved_req_result.error());
			return detail::entry_exit_usage_error;
		}

		auto resolved_req = std::move(*resolved_req_result);
		auto graph = proj.graph(resolved_req);

		const auto action_result = backend::execute_project_action(
			opts,
			resolved_req,
			graph
		);

		if (!action_result) {
			std::println(stderr, "{}", action_result.error());
			return detail::entry_exit_action_failure;
		}

		return detail::entry_exit_success;
	}

	template <typename ProjectFactory>
		requires(!build::toolchain_registry_like<std::remove_cvref_t<ProjectFactory>>)
	[[nodiscard]] inline int entry(
		const sys::command_line& command_line,
		ProjectFactory&& project_factory
	) {
		return entry(
			command_line,
			std::forward<ProjectFactory>(project_factory),
			build::default_toolchains
		);
	}
}

#endif
// ===== end include/mgmake/entry/entry.hxx =====


// ===== begin include/mgmake/entry/macro.hxx =====
#pragma once

#ifndef MGMAKE_ENTRY_MACRO_HXX
#define MGMAKE_ENTRY_MACRO_HXX

// skipped duplicate include: include/mgmake/entry/entry.hxx

namespace mgmk = mgmake;

#if defined(MGMK_PLATFORM_WINDOWS) && defined(MGMK_INCLUDED_WINDOWS)

#define MGMK_DETAIL_ENTRY_0()                                                   \
int wmain(int argc, wchar_t** argv) {                                            \
    auto args = ::mgmk::sys::args_from_wide(argc, argv);                         \
    return ::mgmk::entry(args);                                                  \
}

#define MGMK_DETAIL_ENTRY_1(ProjectFactory)                                      \
int wmain(int argc, wchar_t** argv) {                                            \
    auto args = ::mgmk::sys::args_from_wide(argc, argv);                         \
    return ::mgmk::entry(args, ProjectFactory);                                  \
}

#define MGMK_DETAIL_ENTRY_2(ProjectFactory, Toolchains)                          \
int wmain(int argc, wchar_t** argv) {                                            \
    auto args = ::mgmk::sys::args_from_wide(argc, argv);                         \
    return ::mgmk::entry(args, ProjectFactory, Toolchains);                      \
}

#else

#define MGMK_DETAIL_ENTRY_0()                                                   \
int main(int argc, char** argv) {                                                \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv);                         \
    return ::mgmk::entry(args);                                                  \
}

#define MGMK_DETAIL_ENTRY_1(ProjectFactory)                                      \
int main(int argc, char** argv) {                                                \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv);                         \
    return ::mgmk::entry(args, ProjectFactory);                                  \
}

#define MGMK_DETAIL_ENTRY_2(ProjectFactory, Toolchains)                          \
int main(int argc, char** argv) {                                                \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv);                         \
    return ::mgmk::entry(args, ProjectFactory, Toolchains);                      \
}

#endif

#define MGMK_DETAIL_ENTRY_SELECT(_0, _1, _2, NAME, ...) NAME

#define MGMK_ENTRY(...)                                                         \
    MGMK_DETAIL_ENTRY_SELECT(                                                   \
        _0 __VA_OPT__(,) __VA_ARGS__,                                           \
        MGMK_DETAIL_ENTRY_2,                                                    \
        MGMK_DETAIL_ENTRY_1,                                                    \
        MGMK_DETAIL_ENTRY_0                                                     \
    )(__VA_ARGS__)

#define MGMAKE_BUILD_ENTRY(...) MGMK_ENTRY(__VA_ARGS__)
#define MGMK_BUILD_ENTRY(...) MGMK_ENTRY(__VA_ARGS__)

#endif
// ===== end include/mgmake/entry/macro.hxx =====


#endif
// ===== end include/mgmake/mgmake.hxx =====


#endif // MGMAKE_SINGLE_HEADER_HXX
