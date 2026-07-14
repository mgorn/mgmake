// This file is generated. Do not edit directly.
// Source: include/mgmake/mgmake.hxx

#ifndef MGMAKE_SINGLE_HEADER_HXX
#define MGMAKE_SINGLE_HEADER_HXX


// ===== begin include/mgmake/mgmake.hxx =====
#pragma once

#ifndef MGMAKE_MGMAKE_HXX
#define MGMAKE_MGMAKE_HXX


// ===== begin include/mgmake/cli/entry.hxx =====
#pragma once

#ifndef MGMAKE_CLI_ENTRY_HXX
#define MGMAKE_CLI_ENTRY_HXX


// ===== begin include/mgmake/cli/options.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX


// ===== begin include/mgmake/task/build.hxx =====
#pragma once

#ifndef MGMAKE_TASK_BUILD_HXX
#define MGMAKE_TASK_BUILD_HXX


// ===== begin include/mgmake/cli/option.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTION_HXX
#define MGMAKE_CLI_OPTION_HXX


// ===== begin include/mgmake/cli/value_parser.hxx =====
#pragma once

#ifndef MGMAKE_CLI_VALUE_PARSER_HXX
#define MGMAKE_CLI_VALUE_PARSER_HXX

#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>

// Value parsers convert one option argument string into a typed destination value.

namespace mgmake::cli {
	template<typename type_t>
	struct value_parser {
		// Hint for the value type in help menu
		static inline constexpr std::string_view help_hint = "value";
	};

	template<> struct value_parser<std::string> {
		static inline constexpr std::string_view help_hint = "text";

		[[nodiscard]] static std::expected<std::string, std::string> parse(std::string_view text) {
			return std::string{ text };
		}
	};

	template<> struct value_parser<int> {
		static inline constexpr std::string_view help_hint = "integer";

		[[nodiscard]] static std::expected<int, std::string> parse(std::string_view text) {
			if (text.empty()) {
				return std::unexpected(std::format("invalid integer value '{}' (empty)", text));
			}

			try {
				// Why can't std::stoi take a string_view???
				return std::stoi(std::string{ text });
				// Why does the alternative `std::from_chars` return a `std::from_chars_result` instead of a `std::expected` or something??
			} catch (...) {}
			return std::unexpected(std::format("invalid integer value '{}'", text));
		}
	};

	template<> struct value_parser<std::filesystem::path> {
		static inline constexpr std::string_view help_hint = "path";

		[[nodiscard]] static std::expected<std::filesystem::path, std::string> parse(std::string_view text) {
			return std::filesystem::path{ text };
		}
	};
}

#endif // MGMAKE_CLI_VALUE_PARSER_HXX
// ===== end include/mgmake/cli/value_parser.hxx =====



// ===== begin include/mgmake/detail/assert.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_ASSERT_HXX
#define MGMAKE_DETAIL_ASSERT_HXX

#include <cstdlib>
#include <iostream>
#include <source_location>
#include <string_view>

// mgmkassert is the lightweight invariant check used throughout spec construction and lowering.

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
    #endif // NDEBUG
#endif // MGMK_ENABLE_ASSERTS

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
#endif // MGMK_ENABLE_ASSERTS

#endif // MGMAKE_DETAIL_ASSERT_HXX
// ===== end include/mgmake/detail/assert.hxx =====


// ===== begin include/mgmake/detail/index_bit.hxx =====
#pragma once

// skipped duplicate include: include/mgmake/detail/assert.hxx

#include <bit>
#include <bitset>
#include <cstddef>
#include <limits>
#include <utility>

namespace mgmake::detail {
	// Returns the index of the single set bit.
	template<std::size_t N>
	[[nodiscard]] constexpr std::size_t index_bit(std::bitset<N> bits) noexcept {
		static_assert(N > 0);

		using chunk_t = unsigned long long;
		constexpr std::size_t chunk_bits = std::numeric_limits<chunk_t>::digits;

		// Select only the lowest chunk so that `to_ullong()` cannot overflow.
		const auto chunk_mask = ~(~std::bitset<N>{} << chunk_bits);

		mgmkassert(bits.count() == 1, "index_bit requires a bitset with exactly 1 bit set");

		for (std::size_t offset = 0; offset < N; offset += chunk_bits) {
			const auto selected_chunk = bits & chunk_mask;

			if (selected_chunk.any()) {
				const auto chunk = selected_chunk.to_ullong();

				return offset + static_cast<std::size_t>(std::countr_zero(chunk));
			}

			bits >>= chunk_bits;
		}

		std::unreachable();
	}
}// ===== end include/mgmake/detail/index_bit.hxx =====


// ===== begin include/mgmake/meta/member_access.hxx =====
#pragma once

#ifndef MGMAKE_META_MEMBER_ACCESS_HXX
#define MGMAKE_META_MEMBER_ACCESS_HXX

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>


// ===== begin include/mgmake/meta/member_traits.hxx =====
#pragma once

#ifndef MGMAKE_META_MEMBER_TRAITS_HXX
#define MGMAKE_META_MEMBER_TRAITS_HXX

#include <type_traits>


// ===== begin include/mgmake/meta/type_list.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_LIST_HXX
#define MGMAKE_META_TYPE_LIST_HXX

#include <array>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>

// Compile-time list of types.
//
// `type_list` stores a variadic type pack as a named type. It has no runtime
// state. The list can be queried, extended, merged, filtered, sorted, folded,
// or applied to another variadic template with `apply`.

namespace mgmake::meta {
	template<typename... types_t>
	struct type_list {
		static consteval std::size_t size() {
			return sizeof...(types_t);
		}

		template<std::size_t index>
		using type_at = std::tuple_element_t<index, std::tuple<types_t...>>;

		template<typename type_t>
		static consteval std::size_t count() {
			return (std::size_t{0} + ... + (std::same_as<type_t, types_t> ? 1 : 0));
		}

		template<typename type_t>
		static consteval bool has() {
			return (count<type_t>() != 0);
		}

		template<typename type_t>
		static consteval bool unique() {
			return (count<type_t>() == 1);
		}

		template<typename type_t>
		static consteval std::size_t index() {
			static_assert(unique<type_t>(), "type_list::index<type_t>() requires exactly one matching type.");

			constexpr std::array<bool, size()> matches {
				std::same_as<type_t, types_t>...
			};

			for (std::size_t i = 0; i < matches.size(); ++i) {
				if (matches.at(i)) {
					return i;
				}
			}

			return size();
		}

		template<typename... other_types_t>
		using append_types = type_list<types_t..., other_types_t...>;

		template<typename type_t>
		using append = append_types<type_t>;

	private:
		// Needed because each unique append depends on the list produced by the previous append.
		template<typename current_list_t, typename... other_types_t>
		struct append_types_unique_type {
			using type = current_list_t;
		};

		template<typename current_list_t, typename first_t, typename... rest_t>
		struct append_types_unique_type<current_list_t, first_t, rest_t...> {
			using next_list_t = std::conditional_t<
				current_list_t::template has<first_t>(),
				current_list_t,
				typename current_list_t::template append<first_t>
			>;

			using type = typename append_types_unique_type<
				next_list_t,
				rest_t...
			>::type;
		};

		template<typename current_list_t, typename... other_types_t>
		struct prepend_types_unique_type {
			using type = current_list_t;
		};

		template<typename current_list_t, typename first_t, typename... rest_t>
		struct prepend_types_unique_type<current_list_t, first_t, rest_t...> {
			using next_list_t = std::conditional_t<
				current_list_t::template has<first_t>(),
				current_list_t,
				typename current_list_t::template prepend<first_t>
			>;

			using type = typename prepend_types_unique_type<
				next_list_t,
				rest_t...
			>::type;
		};

	public:
		template<typename... other_types_t>
		using append_types_unique = typename append_types_unique_type<
			type_list<types_t...>,
			other_types_t...
		>::type;

		template<typename type_t, bool check = true>
		struct append_unique_type {
			static_assert((not check) or (not has<type_t>()), "type_list::append_unique cannot append a duplicate type.");

			using type = append_types_unique<type_t>;
		};

		template<typename type_t, bool check = true>
		using append_unique = typename append_unique_type<type_t, check>::type;

		template<typename... other_types_t>
		using prepend_types = type_list<other_types_t..., types_t...>;

		template<typename type_t>
		using prepend = prepend_types<type_t>;

		template<typename... other_types_t>
		using prepend_types_unique = typename prepend_types_unique_type<
			type_list<types_t...>,
			other_types_t...
		>::type;

		template<typename type_t, bool check = true>
		struct prepend_unique_type {
			static_assert((not check) or (not has<type_t>()), "type_list::prepend_unique cannot prepend a duplicate type.");

			using type = std::conditional_t<
				has<type_t>(),
				type_list<types_t...>,
				prepend<type_t>
			>;
		};

		template<typename type_t, bool check = true>
		using prepend_unique = typename prepend_unique_type<type_t, check>::type;

		template<typename other_list_t>
		using append_list = typename other_list_t::template apply<append_types>;

		template<typename other_list_t>
		using append_list_unique = typename other_list_t::template apply<append_types_unique>;

		template<typename other_list_t>
		using prepend_list = typename other_list_t::template apply<prepend_types>;

		template<typename other_list_t>
		using prepend_list_unique = typename other_list_t::template apply<prepend_types_unique>;

		// Invoke a variadic template with this list's stored type pack.
		template<template<typename...> typename pack_t>
		using apply = pack_t<types_t...>;

	private:
		template<auto operation, typename state_t, typename... remaining_t>
		struct fold_type;

		template<auto operation, typename state_t>
		struct fold_type<operation, state_t> {
			using type = state_t;
		};

		template<auto operation, typename state_t, typename first_t, typename... rest_t>
		struct fold_type<operation, state_t, first_t, rest_t...> {
			using next_state_t = typename decltype(operation.template operator()<state_t, first_t>())::type;

			using type = typename fold_type<operation, next_state_t, rest_t...>::type;
		};

	public:
		// Fold the stored types from left to right into an accumulated type.
		//
		// The operation should be callable as:
		//     `operation.template operator()<state_t, type_t>()`
		// and return `std::type_identity<next_state_t>`.
		template<auto operation, typename initial_t>
		using fold = typename fold_type<operation, initial_t, types_t...>::type;

		// Keep the types for which the consteval NTTP predicate returns true.
		//
		// The predicate should be callable as:
		//     `predicate.template operator()<type_t>()`
		template<auto predicate>
		using filter = fold<
			[]<typename filtered_t, typename type_t>() consteval {
				return std::type_identity<
					std::conditional_t<
						static_cast<bool>(predicate.template operator()<type_t>()),
						typename filtered_t::template append<type_t>,
						filtered_t
					>
				>{};
			},
			type_list<>
		>;

	private:
		template<auto compare, typename sorted_t, typename type_t>
		struct insert_sorted_type;

		template<auto compare, typename type_t>
		struct insert_sorted_type<compare, type_list<>, type_t> {
			using type = type_list<type_t>;
		};

		template<auto compare, typename type_t, typename first_t, typename... rest_t>
		struct insert_sorted_type<compare, type_list<first_t, rest_t...>, type_t> {
			using type = std::conditional_t<
				static_cast<bool>(compare.template operator()<type_t, first_t>()),
				type_list<type_t, first_t, rest_t...>,
				typename insert_sorted_type<
					compare,
					type_list<rest_t...>,
					type_t
				>::type::template prepend<first_t>
			>;
		};

	public:
		// Sort this `type_list` using a consteval NTTP comparator.
		//
		// The comparator should be callable as:
		//     `compare.template operator()<left_t, right_t>()`
		// and return true when `left_t` should appear before `right_t`.
		template<auto compare>
		using sort = fold<
			[]<typename sorted_t, typename type_t>() consteval {
				return std::type_identity<
					typename insert_sorted_type<
						compare,
						sorted_t,
						type_t
					>::type
				>{};
			},
			type_list<>
		>;

		// Invoke the callable with the type at the runtime-selected index.
		template<typename callable_t>
		static constexpr decltype(auto) type_switch(callable_t&& callable, std::size_t index) {
			// A return type cannot be inferred without at least one stored type.
			static_assert(size() > 0, "Cannot type-switch over an empty type_list.");
			mgmkassert(index < size(), "type_switch index is outside the bounds of the type_list");

			using first_type = type_at<0>;
			using return_t = decltype(std::declval<callable_t&&>().template operator()<first_type>());
			// A single dispatch table requires every specialization to share a return type.
			static_assert((std::same_as<return_t, decltype(std::declval<callable_t&&>().template operator()<types_t>())> and ...), "Every type_switch invocation must return the same type.");

			using dispatch_t = return_t (*)(callable_t&&);
			// Generate one reusable dispatch entry for each type.
			static constexpr std::array<dispatch_t, size()> dispatch {
				+[](callable_t&& callable) -> return_t {
					return std::forward<callable_t>(callable).template operator()<types_t>();
				}...
			};

			return dispatch[index](std::forward<callable_t>(callable));
		}
	};
}

#endif // MGMAKE_META_TYPE_LIST_HXX// ===== end include/mgmake/meta/type_list.hxx =====


namespace mgmake::meta {
    // Extracts information specifically from a member-function pointer.
    template<typename type_t>
    struct member_function_traits;

    // Extracts information from either a member-function or member-object pointer.
    template<typename type_t>
    struct member_traits;

    // Member functions use the more specific member-function trait.
    template<typename type_t> requires std::is_member_function_pointer_v<type_t>
    struct member_traits<type_t> : member_function_traits<type_t> {};

    // Member objects expose their declaring class and stored type.
    template<typename member_t, typename class_t> requires (not std::is_function_v<member_t>)
    struct member_traits<member_t class_t::*> {
        using class_type = class_t;
        using member_type = member_t;

        static constexpr auto is_function = false;
        static constexpr auto is_object = true;
    };

	// Macro bc we need like 12 different versions of the same thing to cover all bases
	// fucking hell C++ just steal Zig ideas already
#define MGMAKE_META_MEMBER_FUNCTION_TRAITS(...)                            \
    template<                                                              \
        bool noexcept_v,                                                    \
        typename return_t,                                                  \
        typename class_t,                                                   \
        typename... args_t                                                  \
    >                                                                       \
    struct member_function_traits<                                          \
        return_t (class_t::*)(args_t...) __VA_ARGS__ noexcept(noexcept_v)   \
    > {                                                                     \
        using class_type = class_t;                                         \
        using member_type =                                                 \
            return_t(args_t...) __VA_ARGS__ noexcept(noexcept_v);           \
        using return_type = return_t;                                       \
        using arg_types = type_list<args_t...>;                             \
                                                                            \
        static constexpr auto is_function = true;                           \
        static constexpr auto is_object = false;                            \
        static constexpr auto is_noexcept = noexcept_v;                     \
    }

    MGMAKE_META_MEMBER_FUNCTION_TRAITS();
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(const);
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(volatile);
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(const volatile);

    MGMAKE_META_MEMBER_FUNCTION_TRAITS(&);
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(const &);
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(volatile &);
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(const volatile &);

    MGMAKE_META_MEMBER_FUNCTION_TRAITS(&&);
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(const &&);
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(volatile &&);
    MGMAKE_META_MEMBER_FUNCTION_TRAITS(const volatile &&);

#undef MGMAKE_META_MEMBER_FUNCTION_TRAITS
}

#endif // MGMAKE_META_MEMBER_TRAITS_HXX// ===== end include/mgmake/meta/member_traits.hxx =====


namespace mgmake::meta {
    // Provides uniform get/set access through a compile-time bound member-object pointer.
    template<auto member_ptr = nullptr>
    struct member_access {
		static inline constexpr bool valid = true;
        using pointer_type = decltype(member_ptr);

        static_assert(std::is_member_object_pointer_v<pointer_type>, "member_access requires a pointer to a non-static data member");
        static_assert(member_ptr != nullptr, "member_access requires a non-null member pointer");

        using traits = member_traits<pointer_type>;
        using class_type = typename traits::class_type;
        using value_type = typename traits::member_type;

        static constexpr auto pointer = member_ptr;

        template<typename object_t> requires std::invocable<pointer_type, object_t&&>
        [[nodiscard]] static constexpr decltype(auto) get(object_t&& object)
			noexcept(std::is_nothrow_invocable_v<pointer_type, object_t&&>) {
            return std::invoke(pointer, std::forward<object_t>(object));
        }

        template<typename object_t, typename new_value_t> requires (std::invocable<pointer_type, object_t&&> and std::is_assignable_v<std::invoke_result_t<pointer_type, object_t&&>, new_value_t&&>)
        static constexpr void set(object_t&& object, new_value_t&& new_value) noexcept(std::is_nothrow_invocable_v<pointer_type, object_t&&> and std::is_nothrow_assignable_v<std::invoke_result_t<pointer_type, object_t&&>, new_value_t&&>) {
            std::invoke(pointer, std::forward<object_t>(object)) = std::forward<new_value_t>(new_value);
        }
    };
	template<>
	struct member_access<nullptr> {
		static inline constexpr bool valid = false;
	};
}

#endif // MGMAKE_META_MEMBER_ACCESS_HXX// ===== end include/mgmake/meta/member_access.hxx =====


// ===== begin include/mgmake/meta/type_builder.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_BUILDER_HXX
#define MGMAKE_META_TYPE_BUILDER_HXX


// ===== begin include/mgmake/meta/static_string.hxx =====
#pragma once

#ifndef MGMAKE_META_STATIC_STRING_HXX
#define MGMAKE_META_STATIC_STRING_HXX

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

// static_string carries compile-time strings through templates without relying on runtime storage.

namespace mgmake::meta {
    template<std::size_t N>
    struct static_string {
        std::array<char, N> m_data{};

        constexpr static_string() = default;

        // Allows implicit conversion from string literals.
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
            return { m_data.data(), size() };
        }

        constexpr operator std::string_view() const noexcept {
            return view();
        }

		template<std::unsigned_integral hash_t = std::size_t>
        [[nodiscard]] constexpr hash_t hash() const noexcept {
            constexpr hash_t offset_basis = sizeof(hash_t) <= 4
				? static_cast<hash_t>(2166136261u)
				: static_cast<hash_t>(14695981039346656037ull);

            constexpr hash_t prime = sizeof(hash_t) <= 4
				? static_cast<hash_t>(16777619u)
				: static_cast<hash_t>(1099511628211ull);

            auto result = offset_basis;
            for (const auto c : view()) {
                result ^= static_cast<hash_t>(static_cast<uint8_t>(c));
                result *= prime;
            }

            return result;
        }
    };

    template<std::size_t N1, std::size_t N2>
    constexpr bool operator==(const static_string<N1>& a, const static_string<N2>& b) noexcept {
        if constexpr (N1 != N2) {
            return false;
        } else {
            for (std::size_t i = 0; i < N1; ++i) {
                if (a.m_data[i] != b.m_data[i]) {
                    return false;
                }
            }

            return true;
        }
    }

    template<std::size_t N1, std::size_t N2>
    constexpr auto operator+(const static_string<N1>& a, const static_string<N2>& b) {
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

namespace std {
    template<std::size_t N>
    struct hash<::mgmake::meta::static_string<N>> {
        [[nodiscard]] constexpr std::size_t operator()(
            const ::mgmake::meta::static_string<N>& value
        ) const noexcept {
            return value.hash();
        }
    };
}

#endif // MGMAKE_META_STATIC_STRING_HXX// ===== end include/mgmake/meta/static_string.hxx =====


// ===== begin include/mgmake/meta/type_map.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_MAP_HXX
#define MGMAKE_META_TYPE_MAP_HXX

// skipped duplicate include: include/mgmake/meta/type_list.hxx

// ===== begin include/mgmake/meta/type_pair.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_PAIR_HXX
#define MGMAKE_META_TYPE_PAIR_HXX

namespace mgmake::meta {
    template<typename key_t, typename value_t>
    struct type_pair {
        using key_type = key_t;
        using value_type = value_t;
    };
}

#endif // MGMAKE_META_TYPE_PAIR_HXX// ===== end include/mgmake/meta/type_pair.hxx =====


#include <array>
#include <concepts>
#include <cstddef>

namespace mgmake::meta {
	template<typename storage_t = type_list<>>
	struct type_map;

	template<typename... pairs_t>
	struct type_map<type_list<pairs_t...>> {
		using storage_type = type_list<pairs_t...>;

	private:
		using key_list_type = type_list<typename pairs_t::key_type...>;

		static_assert(
			(key_list_type::template unique<typename pairs_t::key_type>() and ...),
			"type_map storage cannot contain duplicate keys."
		);

	public:
		static consteval std::size_t size() {
			return storage_type::size();
		}

		template<typename key_t>
		static consteval std::size_t key_index() {
			constexpr std::array<bool, size()> matches {
				std::same_as<key_t, typename pairs_t::key_type>...
			};

			for (std::size_t i = 0; i < matches.size(); ++i) {
				if (matches.at(i)) {
					return i;
				}
			}

			return size();
		}

		template<typename key_t>
		static consteval bool has() {
			return key_index<key_t>() != size();
		}

	private:
		template<typename key_t, bool check = true, bool exists = has<key_t>()>
		struct at_type;

		template<typename key_t, bool check>
		struct at_type<key_t, check, true> {
			using pair_type = typename storage_type::template type_at<key_index<key_t>()>;
			using type = typename pair_type::value_type;
		};

		template<typename key_t, bool check>
		struct at_type<key_t, check, false> {
			static_assert(
				not check or has<key_t>(),
				"type_map::at<key_t> requires key_t to exist."
			);

			using type = void;
		};

    public:
		template<typename key_t, bool check = true>
		using at = typename at_type<key_t, check>::type;

    private:
        template<typename pair_t, typename key_t, typename value_t>
        using replace_pair = std::conditional_t<
            std::same_as<key_t, typename pair_t::key_type>,
            type_pair<key_t, value_t>,
            pair_t
        >;

        template<typename key_t, typename value_t>
        struct emplace_type {
            using type = std::conditional_t<
                has<key_t>(),
                type_map<type_list<replace_pair<pairs_t, key_t, value_t>...>>,
                type_map<typename storage_type::template append<type_pair<key_t, value_t>>>
            >;
        };

        template<typename key_t, typename value_t>
        struct emplace_unique_type {
            static_assert(
                not has<key_t>(),
                "type_map::emplace_unique<key_t, value_t> cannot emplace a duplicate key."
            );

            using type = type_map<
                typename storage_type::template append<type_pair<key_t, value_t>>
            >;
        };

    public:
        template<typename key_t, typename value_t>
        using emplace = typename emplace_type<key_t, value_t>::type;

        template<typename key_t, typename value_t>
        using emplace_unique = typename emplace_unique_type<key_t, value_t>::type;
	};
}

#endif // MGMAKE_META_TYPE_MAP_HXX// ===== end include/mgmake/meta/type_map.hxx =====


// ===== begin include/mgmake/meta/type_value.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_VALUE_HXX
#define MGMAKE_META_TYPE_VALUE_HXX

// Wrap a comptime value as a type

namespace mgmake::meta {
    template<auto value_v>
    struct type_value {
        static inline constexpr auto value = value_v;
    };

    template<typename value_t, auto default_v>
    struct type_value_or {
        static constexpr auto value = value_t::value;
    };

    template<auto default_v>
    struct type_value_or<void, default_v> {
        static constexpr auto value = default_v;
    };
}

#endif // MGMAKE_META_TYPE_VALUE_HXX// ===== end include/mgmake/meta/type_value.hxx =====


namespace mgmake::meta {
    template<typename storage_t = type_map<>>
    struct type_builder {
        template<static_string key_v, bool check_v = true>
        using get = typename storage_t::template at<type_value<key_v>, check_v>;

        template<static_string key_v, typename value_t>
        using set = type_builder<typename storage_t::template emplace<type_value<key_v>, value_t>>;

        template<template<typename> typename consumer_t>
        using build = consumer_t<storage_t>;
    };
}

// When defining builder fields, ensure `builder_t` is the name of the `meta::type_builder`
#define MGMAKE_TYPE_BUILDER_TYPE_FIELD(wrapper_t, alias_t) \
	MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS( \
		wrapper_t, \
		alias_t, \
		::mgmake::meta::static_string{#alias_t} \
	)

#define MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(wrapper_t, alias_t, key_v) \
	template<typename alias_t##_t> \
	using alias_t = wrapper_t< \
		typename builder_t::template set<key_v, alias_t##_t> \
	>

#define MGMAKE_TYPE_BUILDER_VALUE_FIELD(wrapper_t, alias_t, value_t) \
	MGMAKE_TYPE_BUILDER_VALUE_FIELD_AS( \
		wrapper_t, \
		alias_t, \
		value_t, \
		::mgmake::meta::static_string{#alias_t} \
	)

#define MGMAKE_TYPE_BUILDER_VALUE_FIELD_AS(wrapper_t, alias_t, value_t, key_v) \
	MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS( \
		wrapper_t, \
		alias_t##_type, \
		key_v \
	); \
	template<value_t alias_t##_v> \
	using alias_t = alias_t##_type< \
		::mgmake::meta::type_value<alias_t##_v> \
	>

#define MGMAKE_TYPE_CONSUMER_TYPE_FIELD(alias_t, default_t) \
	MGMAKE_TYPE_CONSUMER_TYPE_FIELD_AS( \
		alias_t, \
		::mgmake::meta::static_string{#alias_t}, \
		default_t \
	)

#define MGMAKE_TYPE_CONSUMER_TYPE_FIELD_AS(alias_t, key_v, default_t) \
	using alias_t##_type = typename storage_t::template at< \
		::mgmake::meta::type_value<key_v>, \
		false \
	>; \
	using alias_t = std::conditional_t< \
		std::same_as<alias_t##_type, void>, \
		default_t, \
		alias_t##_type \
	>

#define MGMAKE_TYPE_CONSUMER_VALUE_FIELD(alias_t, default_v) \
	MGMAKE_TYPE_CONSUMER_VALUE_FIELD_AS( \
		alias_t, \
		::mgmake::meta::static_string{#alias_t}, \
		default_v \
	)

#define MGMAKE_TYPE_CONSUMER_VALUE_FIELD_AS(alias_t, key_v, default_v) \
	MGMAKE_TYPE_CONSUMER_TYPE_FIELD_AS( \
		alias_t##_type, \
		key_v, \
		::mgmake::meta::type_value<default_v> \
	); \
	static inline constexpr auto alias_t##_value = alias_t##_type::value

#endif // MGMAKE_META_TYPE_BUILDER_HXX// ===== end include/mgmake/meta/type_builder.hxx =====


#include <expected>

namespace mgmake::cli {
    // Actual option impl, consume the configuration in the type map
    template<typename storage_t = meta::type_map<>>
    struct option_impl {
        MGMAKE_TYPE_CONSUMER_VALUE_FIELD(name, meta::static_string{ "" });
        MGMAKE_TYPE_CONSUMER_VALUE_FIELD(description, meta::static_string{ "" });
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(short_name, '\0');
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(callback, nullptr);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(assign, void);
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(task, false);
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(flag, true);

		static inline constexpr bool match(std::string_view arg) {
			if (arg.empty()) {
				return false;
			}

			// If the option is a task
			if constexpr (task_value) {
				if (arg == name_value) {
					return true;
				}
			}

			// Parse as switch
			if constexpr (flag_value) {
				if (arg.starts_with("--")) {
					return match_long(arg.substr(2));
				} else if (arg.starts_with("-")) {
					return match_short(arg.substr(1));
				}
			}
			return false;
		}

		static inline constexpr bool match_long(std::string_view arg) {
			return arg.starts_with(name_value);
		}

		static inline constexpr bool match_short(std::string_view arg) {
			// handle short = val (e.g. -g=ninja/-g ninja or smth)
			return arg.size() == 1 and arg.front() == short_name_value;
		}

		static inline constexpr bool is_callback = not std::is_same_v<std::decay_t<decltype(callback_value)>, std::nullptr_t>;
		static inline constexpr std::expected<void, std::string> handle_callback(auto& opts, std::string_view arg) {
			mgmkassert(is_callback, "option_impl::handle_callback called for non-callback option");
			mgmkassert(match(arg), "handling a callback for the incorrect arg");

			if constexpr (is_callback) {
				callback_value(opts);
				return {};
			} else {
				return std::unexpected("option_impl::handle_callback called for an option with no callback");
			}
		}

		static inline constexpr auto is_assign = [] -> bool {
			if constexpr(not std::is_same_v<assign_type, void>) {
				return assign_type::valid;
			}
			return false;
		}();
		static inline constexpr std::expected<void, std::string> handle_assign(auto& opts, std::string_view arg, std::string_view value) {
			mgmkassert(match(arg), "handling a switch with the incorrect arg");
			mgmkassert(is_assign, "handling a normal switch as a value assign switch");

			if constexpr (is_assign) {
				// What is the expected value type?
				using value_type = assign_type::value_type;

				// parse it
				using vp = value_parser<value_type>;
				auto result = vp::parse(value);
				if (not result.has_value()) {
					return std::unexpected(std::format("Error parsing value for arg '{}': {}", arg, result.error()));
				}

				// assign
				assign_type::set(opts, result.value());
			}
			return {};
		}

		template<typename dispatcher_t>
		static inline constexpr std::expected<void, std::string> handle_task(auto& opts, std::string_view arg) {
			mgmkassert(match(arg), "handling a task with the incorrect arg");
			mgmkassert(task_value, "handling a normal switch as a task");

			auto matches = dispatcher_t::match(arg);
			if (not matches.any()) {
				return std::unexpected(std::format("Unknown task '{}' (cli::option_impl::handle_task no match from dispatcher_t::match for arg)", arg));
			}
			opts.m_task = detail::index_bit(matches);
			return {};
		}
    };

    // Build a compile-time map for the option settings
    template<typename builder_t = meta::type_builder<>>
    struct option_builder {
        using builder_type = builder_t;

        MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, name, meta::static_string);
        MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, description, meta::static_string);
        MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, short_name, char);
        MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, callback, auto);
		// option accepts a value (`--switch=value` or `--switch value`) and assigns its value to the option member
		// pass a `meta::member_access<>` for the member to assign.
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(option_builder, assign);
		// Sets the value at the member to the default value.
		template<typename member_t = meta::member_access<>, auto value_v = nullptr>
        using set = callback<[](auto& opts) {
			member_t::set(opts, value_v);
		}>;
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, task, bool);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, flag, bool); // aka switch

        using build = typename builder_type::template build<option_impl>;
    };
    // default builder alias
    using option = option_builder<>;
}

#endif // MGMAKE_CLI_OPTION_HXX// ===== end include/mgmake/cli/option.hxx =====


// ===== begin include/mgmake/sys/exit_code.hxx =====
#pragma once

#ifndef MGMAKE_SYS_EXIT_CODE_HXX
#define MGMAKE_SYS_EXIT_CODE_HXX

namespace mgmake::sys {
    enum struct exit_code : int {
        success,
        task_failure,
        usage_error
    };
}

#endif// ===== end include/mgmake/sys/exit_code.hxx =====


namespace mgmake::task {
	struct build {
		using option_type = cli::option
			::name<"build">
			::description<"Build the project.">
			::task<true>::flag<false>
			::build;
		
		template<typename config_t>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, auto& opts) {
			// TODO: This would be the entrypoint/root for build
			std::println("Build task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_BUILD_HXX// ===== end include/mgmake/task/build.hxx =====


// ===== begin include/mgmake/task/default_tasks.hxx =====
#pragma once

#ifndef MGMAKE_CLI_DEFAULT_TASKS_HXX
#define MGMAKE_CLI_DEFAULT_TASKS_HXX

// skipped duplicate include: include/mgmake/task/build.hxx

// ===== begin include/mgmake/task/help.hxx =====
#pragma once

#ifndef MGMAKE_TASK_HELP_HXX
#define MGMAKE_TASK_HELP_HXX


// ===== begin include/mgmake/task/task_traits.hxx =====
#pragma once

#ifndef MGMAKE_TASK_TASK_TRAITS_HXX
#define MGMAKE_TASK_TASK_TRAITS_HXX

#include <concepts>
#include <expected>
#include <string>

// Forward decl if needed
namespace mgmake::cli {
	struct options;
}
namespace mgmake::sys {
	struct shell;
}

namespace mgmake::task {
	template<typename task_t, typename config_t>
	concept task_handler = requires(const sys::shell& cmd, const cli::options& opts) {
		{
			task_t::template handle<config_t>(cmd, opts)
		} -> std::same_as<std::expected<sys::exit_code, std::string>>;
	};

	template<typename task_t>
	struct task_traits {
		using task_type = task_t;
		using option_type = task_type::option_type;
		template<typename config_t>
		static constexpr bool valid_handler = task_handler<task_type, config_t>;

		static constexpr std::string_view name() {
			return option_type::name_value.view();
		}
		static constexpr std::string_view description() {
			return option_type::description_value.view();
		}

		static constexpr bool match(std::string_view arg) {
			return option_type::match(arg);
		}
	};
}

#endif // MGMAKE_TASK_TASK_HXX// ===== end include/mgmake/task/task_traits.hxx =====


// skipped duplicate include: include/mgmake/cli/option.hxx
// skipped duplicate include: include/mgmake/sys/exit_code.hxx

#include <sstream>
#include <string>

namespace mgmake::task {
	struct help {
		using option_type = cli::option
			::name<"help">::short_name<'h'>
			::description<"Show help.">
			::task<true>
			//::set<meta::member_access<&cli::options::m_task>, 0>
			::callback<[](auto& opts) {
				// Set index with callback bc cyclical include :(
				opts.m_task = 0;
			}>
			::build;
		
		template<typename config_t>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, auto& opts) {
			using config_type = config_t;

			std::println("Usage:");
			std::println("\t{} [task] [options]", cmd.program_name());
			using tasks_type = config_type::tasks_type;
			using options_type = config_type::options_type;
			
			std::println("\nTasks:");
			static constexpr auto task_help = []<typename task_t>(auto& cmd){
				using traits_type = task_traits<task_t>;
				std::println("\t{:<10} {}", traits_type::name(), traits_type::description());
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>, auto& cmd) {
				(task_help.template operator()<typename tasks_type::template type_at<Is>>(cmd), ...);
			}(std::make_index_sequence<tasks_type::size()>{}, cmd);

			std::println("\nOptions:");
			static constexpr auto option_help = []<typename opt_t>{
				// Only print switches, tasks will be shown first
				if constexpr (opt_t::flag_value) {
					std::stringstream ss;
					if constexpr (opt_t::short_name_value != '\0') {
						std::print(ss, "-{}, ", opt_t::short_name_value);
					}
					std::print(ss, "--{}", opt_t::name_value.view());
					if constexpr (opt_t::is_assign) {
						using vp = cli::value_parser<typename opt_t::assign_type::value_type>;
						std::print(ss, "=<{}>", vp::help_hint);
					}
					std::println("\t{:<24} {}", ss.str(), opt_t::description_value.view());
				}
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>) {
				(option_help.template operator()<typename options_type::template type_at<Is>>(), ...);
			}(std::make_index_sequence<options_type::size()>{});

			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_HELP_HXX// ===== end include/mgmake/task/help.hxx =====


// skipped duplicate include: include/mgmake/meta/type_list.hxx
// skipped duplicate include: include/mgmake/sys/exit_code.hxx

#include <sstream>

/*
 * Why are tasks seperate from normal options? (Why can't they just be callback options?)
 *
 * Option callbacks are invoked during parsing and are functions to initialize the `cli::options` structure.
 *
 * The flow:
 * main -> parse -> cli::options -> tasks
 *			|-> match
 *			|-> invoke callbacks
 *
 * but options have a `task` setting? What's with that?
 * You still need to provide the tasks as options to the CLI parser.
 * They simply assign the task value in the `cli::options` structure.
 * Later on, that value is consumed to invoke the respective task handler.
 *
 * To make a task:
 *		1) Create a struct for your task
 *		2) Create an option alias with your desired settings
 *		3) Use the `::task<true>` option in your option alias
 *		3) Create a `handle` function:
 *```
 * template<typename config_t>
 * static inline constexpr std::expected<sys::exit_code, std::string> handle(const sys::shell& cmd, const sli::options& opts)
 *```
 * 
 * NOTE: You only provide the CLI option when making the task. Do not pass them as options in your config seperately.
 */

namespace mgmake::task {
	// Type list of default tasks
	//
    // this way you can add your own tasks to 
    // default_tasks before passing the list 
    // to your mgmake config for your own CLI
	using default_tasks = meta::type_list<
		task::help,
		task::build
	>;
}

#endif // MGMAKE_CLI_DEFAULT_TASKS_HXX// ===== end include/mgmake/task/default_tasks.hxx =====


#include <filesystem>
#include <optional>

namespace mgmake::cli {
	// Store parsed CLI options
	struct options {
		std::optional<std::size_t> m_task = task::default_tasks::index<task::build>();

		bool m_verbose = false;
		bool m_dry_run = false;

		std::filesystem::path m_build_dir = std::filesystem::current_path() / ".build";

		inline constexpr auto task() const {
			return m_task;
		}
	};
}

#endif // MGMAKE_CLI_OPTIONS_HXX// ===== end include/mgmake/cli/options.hxx =====


// ===== begin include/mgmake/cli/parser.hxx =====
#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX

// skipped duplicate include: include/mgmake/cli/options.hxx

// skipped duplicate include: include/mgmake/detail/index_bit.hxx
// skipped duplicate include: include/mgmake/meta/type_list.hxx

// ===== begin include/mgmake/sys/shell.hxx =====
#pragma once

#ifndef MGMAKE_SYS_SHELL_HXX
#define MGMAKE_SYS_SHELL_HXX

#include <algorithm>   // std::ranges::find_first_of
#include <array>       // std::array
#include <cstddef>     // std::size_t
#include <cstdlib>     // std::system
#include <format>      // std::format
#include <print>       // std::println
#include <ranges>      // views, ranges::to
#include <span>        // std::span
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::exchange
#include <vector>      // std::vector

namespace mgmake::sys {
    struct shell {
        inline constexpr std::string_view program_name() const {
            return m_args.empty() ? std::string_view{} : m_args.at(0);
        }

        inline constexpr std::span<const std::string> user_args() const {
            if (m_args.size() <= 1) {
                return {};
            }
            return view().subspan(1);
        }

        inline constexpr std::span<const std::string> view() const {
            return { m_args };
        }

        // Escapes and joins the arguments into the command-line text used for execution and backend generation.
        inline constexpr std::string full_command() const {
            return m_args
                | std::views::transform(arg_escape)
                | std::views::join_with(' ')
                | std::ranges::to<std::string>();
        }

        auto invoke(bool verbose, bool dry_run) const {
            const auto command = full_command();

            if (verbose or dry_run) {
                std::println("{}", command);
            }

            if (dry_run) {
                return 0;
            }

#ifdef MGMK_PLATFORM_WINDOWS
            // cmd.exe strips outer quotes for /c; keep inner quotes around a spaced executable path intact.
            return std::system(std::format("\"{}\"", command).c_str());
#else
            return std::system(command.c_str());
#endif
        }

        // Returns true when an argument needs escaping before being written into a
        // generated shell command.
        //
        // This is required for empty arguments, whitespace, quotes, and characters that
        // may affect how the shell interprets the command line.
        static inline constexpr bool arg_needs_escape(std::string_view arg) {
            return arg.empty() or (std::ranges::find_first_of(arg, special_characters) != arg.end());
        }

        static inline constexpr std::array special_characters{
#ifdef MGMK_PLATFORM_WINDOWS
        ' ', '\t', '"', '&', '|', '<', '>', '^', '%', '!', '(', ')'
#else
        ' ', '\t', '\'', '"', '$', '\\', '&', ';', '(', ')', '<', '>', '|'
#endif // MGMK_PLATFORM_WINDOWS
        };

        static inline constexpr std::string_view empty_arg_escape{
#ifdef MGMK_PLATFORM_WINDOWS
            "\"\""
#else
            "''"
#endif // MGMK_PLATFORM_WINDOWS
        };

        static inline constexpr char quote_character{
#ifdef MGMK_PLATFORM_WINDOWS
            '"'
#else
            '\''
#endif // MGMK_PLATFORM_WINDOWS
        };

        // Escapes one command-line argument for use in a generated shell command.
        //
        // This is only for individual arguments, not whole commands, pipelines,
        // redirections, or multiple concatenated arguments.
        static inline constexpr std::string arg_escape(std::string_view arg) {
            if (not arg_needs_escape(arg)) {
                return std::string{ arg };
            }

            if (arg.empty()) {
                return std::string{ empty_arg_escape };
            }

            std::string result;
            result.reserve(arg.size() + 2);
            result += quote_character;

#ifdef MGMK_PLATFORM_WINDOWS
            static constexpr char escape_character = '\\';
            std::size_t backslashes = 0;

            for (const char ch : arg) {
                if (ch == escape_character) {
                    ++backslashes;
                    continue;
                }

                if (ch == quote_character) {
                    result.append((std::exchange(backslashes, 0) * 2) + 1, escape_character);
                    result.push_back(quote_character);
                    continue;
                }

                result.append(std::exchange(backslashes, 0), escape_character);
                result.push_back(ch);
            }

            result.append(backslashes * 2, escape_character);
#else
            static constexpr std::string_view posix_quote_escape = "'\\''";

            auto escaped_body = arg
                | std::views::split(quote_character)
                | std::views::join_with(posix_quote_escape);

            result.append_range(escaped_body);
#endif // MGMK_PLATFORM_WINDOWS

            result += quote_character;
            return result;
        }

        // Copies the already-tokenized argc/argv arguments; escaping is handled when command text is generated.
        static inline constexpr shell from_args(int argc, const char* const* argv) {
            if (argc <= 0 or argv == nullptr) {
                return {};
            }

            return {
                std::views::counted(argv, argc)
                | std::ranges::to<std::vector<std::string>>()
            };
        }

        std::vector<std::string> m_args{};
    };
}

#endif // MGMAKE_SYS_SHELL_HXX// ===== end include/mgmake/sys/shell.hxx =====


#include <bitset>
#include <expected>
#include <optional>
#include <string>
#include <utility>

namespace mgmake::cli {
    template<typename list_t = meta::type_list<>>
    struct parser {
		using list_type = list_t;

		// Task options (first arg, no - or --)
		using tasks_type = typename list_type::template filter<[]<typename opt_t> -> bool {
			return opt_t::task_value;
		}>;
		// Switch option (- or -- prefix)
		using switches_type = typename list_type::template filter<[]<typename opt_t> -> bool {
			return not opt_t::task_value;
		}>;

		template<typename dispatcher_t>
        static inline constexpr std::expected<options, std::string> parse(const sys::shell& cmd) {
			// The resulting options
			options opts{};

			auto args = cmd.user_args();

			// Match switches
			for (auto it = args.begin(); it != args.end(); ++it) {
				std::string_view arg = *it;

				auto is_long = arg.starts_with("--");
				auto is_short = arg.starts_with("-");
				auto is_switch = is_long or is_short;
				auto is_task = it == args.begin() and not is_switch; // First and isn't switch? -> Task
				// Invalid usage errors + hints
				if (not is_switch and not is_task) {
					std::string error_hint = "";

					// 1) See if it could have been a short switch
					if (error_hint.empty()) {
						auto matches = match(std::format("-{}", arg));
						if (matches.any()) {
							error_hint = std::format("Did you mean '-{}'?", arg);
						}
					}

					// 2) See if it could have been a long switch
					if (error_hint.empty()) {
						auto matches = match(std::format("--{}", arg));
						if (matches.any()) {
							error_hint = std::format("Did you mean '--{}'?", arg);
						}
					}

					// 3) See if the arg is meant to be used as an task
					if (error_hint.empty()) {
						using task_parser = parser<tasks_type>;
						auto matches = task_parser::match(arg);
						if (matches.any()) {
							auto corrected = std::format("{} {} ...", cmd.program_name(), arg);
							error_hint = std::format("'{}' is a task, did you mean '{}'?", arg, corrected);
						}
					}

					if (not error_hint.empty()) {
						return std::unexpected(std::format("Invalid argument: {} ({})", arg, error_hint));
					}
					return std::unexpected(std::format("Invalid argument: {}", arg));
				}
				mgmkassert(is_task or is_switch, "Values for switches should be skipped/parsed by the switch needing it");

				// Shrimply doesn't exit?
				auto matches = match(arg);
				if (not matches.any()) {
					return std::unexpected(std::format("Unknown argument: '{}'", arg));
				}
				// If this happens, there's a conflict with option names (either long or short)
				mgmkassert(matches.count() == 1, "Matched arg to more than one option?");

				auto index = detail::index_bit(matches);
				auto result = list_type::type_switch([&]<typename opt_t> -> std::expected<bool, std::string> {
					// If the option expects a value
					if constexpr (opt_t::is_assign) {
						// What is the expected value type?
						using assign_type = opt_t::assign_type;
						// TODO: If value_type is a std::vector or other container,
						// we need to keep reading each arg, parse them, and store...
						using value_type = assign_type::value_type;

						// Is it `--switch=value` or `--switch value`?
						std::string_view value_text{};
						bool move_next = false; // If we need to move the iterator after consuming an arg
						if (const auto seperator = arg.find_first_of("="); seperator != std::string_view::npos) {
							value_text = arg.substr(seperator+1);
							arg = arg.substr(0, seperator);
						} else {
							// Get the next arg
							auto next_it = std::next(it);
							if (next_it == args.end()) {
								return std::unexpected(std::format("argument '{}' expects a value", arg));
							}

							value_text = *next_it;
							move_next = true;
						}

						// assign
						auto result = opt_t::handle_assign(opts, arg, value_text);
						if (not result) {
							return std::unexpected(std::format("opt_t::handle_assign failed: {}", result.error()));
						}

						// Move the iterator
						if (move_next)
							it = std::next(it);
						return true;
					}
					
					// If the option invokes a callback
					if constexpr (opt_t::is_callback) {
						auto result = opt_t::handle_callback(opts, arg);
						if (not result) {
							return std::unexpected(std::format("opt_t::handle_callback failed: {}", result.error()));
						}

						return true;
					}

					// If the option is a task
					if constexpr (opt_t::task_value) {
						auto result = opt_t::template handle_task<dispatcher_t>(opts, arg);
						if (not result) {
							return std::unexpected(std::format("opt_t::handle_task failed: {}", result.error()));
						}

						return true;
					}

					return std::unexpected("cli::parser::parse: Not implemented");
				}, index);

				if (not result.has_value()) {
					return std::unexpected(result.error());
				}
			}

            return opts;
        }

		using matches_type = std::bitset<list_type::size()>;
		static inline constexpr matches_type match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type matches{};
				(matches.set(Is, list_type::template type_at<Is>::match(arg)), ...);
				return matches;
			}(std::make_index_sequence<list_type::size()>{}, arg);
		}
    };
}

#endif // MGMAKE_CLI_PARSER_HXX// ===== end include/mgmake/cli/parser.hxx =====


// skipped duplicate include: include/mgmake/sys/exit_code.hxx
// skipped duplicate include: include/mgmake/sys/shell.hxx

// ===== begin include/mgmake/task/dispatcher.hxx =====
#pragma once

#ifndef MGMAKE_TASK_DISPATCHER_HXX
#define MGMAKE_TASK_DISPATCHER_HXX

// skipped duplicate include: include/mgmake/task/task_traits.hxx

// skipped duplicate include: include/mgmake/cli/options.hxx
// skipped duplicate include: include/mgmake/sys/exit_code.hxx
// skipped duplicate include: include/mgmake/sys/shell.hxx

// task::dispatcher
// consumes the cli::options and executes the required task(s)

namespace mgmake::task {
	// The mgmake config
    template<typename config_t>
	struct dispatcher {
		using config_type = config_t;
		using list_type = config_type::tasks_type;

		static inline constexpr std::expected<sys::exit_code, std::string> invoke(const sys::shell& cmd, const cli::options& opts) {
			if (not opts.task().has_value()) {
				return std::unexpected("cli::dispatcher::invoke cannot invoke without a task!");
			}
			return list_type::type_switch([&]<typename task_t> -> std::expected<sys::exit_code, std::string> {
				using traits_type = task_traits<task_t>;
				static_assert(traits_type::template valid_handler<config_type>, "task is missing a handle function");

				return task_t::template handle<config_type>(cmd, opts);
			}, opts.task().value());
		}

		using matches_type = std::bitset<list_type::size()>;
		static inline constexpr matches_type match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type matches{};
				(matches.set(Is, task_traits<typename list_type::template type_at<Is>>::match(arg)), ...);
				return matches;
			}(std::make_index_sequence<list_type::size()>{}, arg);
		}
	};
}

#endif // MGMAKE_TASK_DISPATCHER_HXX// ===== end include/mgmake/task/dispatcher.hxx =====


// ===== begin include/mgmake/config.hxx =====
#pragma once

#ifndef MGMAKE_CONFIG_HXX
#define MGMAKE_CONFIG_HXX


// ===== begin include/mgmake/cli/default_options.hxx =====
#pragma once

#ifndef MGMAKE_CLI_DEFAULT_OPTIONS_HXX
#define MGMAKE_CLI_DEFAULT_OPTIONS_HXX

// skipped duplicate include: include/mgmake/cli/option.hxx
// skipped duplicate include: include/mgmake/cli/options.hxx

// skipped duplicate include: include/mgmake/meta/type_list.hxx

#include <print>

namespace mgmake::cli {
	using verbose_option = option
		::name<"verbose">::short_name<'v'>
		::description<"Print commands before executing them.">
		::set<meta::member_access<&options::m_verbose>, true>
		::build;
	
	using dry_run_option = option
		::name<"dry-run">
		::description<"Print commands without executing them.">
		::set<meta::member_access<&options::m_dry_run>, true>
		::build;

	using build_dir_option = option
		::name<"build-dir">
		::description<"Set the build directory.">
		::assign<meta::member_access<&options::m_build_dir>>
		// ::assign_hint<"path"> - Derive based on type..?
		::build;
	
    // Type list of default options
	//
    // this way you can add your own options to 
    // default_options before passing the list 
    // to your mgmake config for your own CLI
    using default_options = meta::type_list<
		verbose_option,
		dry_run_option,
		build_dir_option
    >;
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX// ===== end include/mgmake/cli/default_options.hxx =====

// skipped duplicate include: include/mgmake/meta/type_builder.hxx

// ===== begin include/mgmake/meta/type_or.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_OR_HXX
#define MGMAKE_META_TYPE_OR_HXX

#include <type_traits>

// If the type is void, return the default type instead

namespace mgmake::meta {
    template<typename type_t, typename default_t, typename none_t = void>
    struct type_or {
        using type = std::remove_cvref_t<std::invoke_result_t<decltype([] consteval {
			if constexpr (std::is_same_v<type_t, none_t>) {
				return std::type_identity<default_t>{};
			} else {
				return std::type_identity<type_t>{};
			}
		})>>::type;

		static_assert(not std::is_same_v<default_t, none_t>, "If your default_t is none_t, you don't need `meta::type_or`...");
    };

	template<typename type_t, typename default_t, typename none_t = void>
	using type_or_t = type_or<type_t, default_t, none_t>::type;
}

#endif // MGMAKE_META_TYPE_OR_HXX// ===== end include/mgmake/meta/type_or.hxx =====

// skipped duplicate include: include/mgmake/task/default_tasks.hxx

namespace mgmake {
	template<typename storage_t = meta::type_map<>>
	struct config_impl {
		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(project, nullptr);
		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(toolchains, nullptr);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(tasks, void);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(options, void);
	};

	template<typename builder_t = meta::type_builder<>>
    struct config_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(config_builder, project, auto);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(config_builder, toolchains, auto);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(config_builder, tasks);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(config_builder, options);

		using build = std::decay_t<std::invoke_result_t<decltype([] consteval {
			/* Automatically add task options to options */
			// Get the tasks
			using tasks_type = meta::type_or_t<typename builder_type::template get<"tasks", false>, task::default_tasks>;
			// Get the options
			using options_type = meta::type_or_t<typename builder_type::template get<"options", false>, cli::default_options>;

			// Collect the option associated with every task.
			using task_options = typename tasks_type::template fold<[]<typename state_t, typename task_t>() consteval {
				return std::type_identity<typename state_t::template append<typename task_t::option_type>>{};
			}, meta::type_list<>>;

			// Append the contents of task_options, not task_options itself.
			using actual_options_type = typename options_type::template prepend_list<task_options>;

			// Create a new config builder with the task options appended
			// Update the existing builder directly. Creating another config_builder
			// specialization here would recursively instantiate its build alias.
			using actual_builder_type = typename builder_type::template set<"options", actual_options_type>;

			// Use the builder type from that instead
			using result_type = typename actual_builder_type::template build<config_impl>;

			// Now we have a config where options has task options appended
			return result_type{};
		})>>;
	};
	using config = config_builder<>::tasks<task::default_tasks>::options<cli::default_options>;
}

#endif // MGMAKE_CONFIG_HXX// ===== end include/mgmake/config.hxx =====


#include <print>
#include <utility>

namespace mgmake::cli {
    template<typename config_builder_t = config>
    inline sys::exit_code entry(sys::shell cmd) {
		// Finalize the given config
		using config_type = config_builder_t::build;

        // construct the parser & dispatcher at compile time :)
		using d = task::dispatcher<config_type>;
        using p = parser<typename config_type::options_type>;

        // parse cmd at runtime
        if (auto result = p::template parse<d>(cmd)) {
			auto opts = result.value();
			d::invoke(cmd, opts);
            return sys::exit_code::success;
        } else {
            std::println(stderr, "{}", result.error());
            return sys::exit_code::usage_error;
        }
    }

    template<typename config_builder_t = config>
    inline sys::exit_code entry(int argc, char* argv[]) {
        return entry<config_builder_t>(sys::shell::from_args(argc, argv));
    }
}

// Pass a `cli::config` type with your project
#define MGMK_ENTRY(...) \
int main(int argc, char* argv[]) { \
    return std::to_underlying(::mgmake::cli::entry<__VA_ARGS__>(argc, argv)); \
}

#endif // MGMAKE_CLI_ENTRY_HXX// ===== end include/mgmake/cli/entry.hxx =====


namespace mgmk = mgmake;

#endif // MGMAKE_MGMAKE_HXX
// ===== end include/mgmake/mgmake.hxx =====


#endif // MGMAKE_SINGLE_HEADER_HXX
