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


// ===== begin include/mgmake/cli/default_options.hxx =====
#pragma once

#ifndef MGMAKE_CLI_DEFAULT_OPTIONS_HXX
#define MGMAKE_CLI_DEFAULT_OPTIONS_HXX


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
	struct value_parser;

	template<> struct value_parser<std::string> {
		[[nodiscard]] static std::expected<std::string, std::string> parse(std::string_view text) {
			return std::string{ text };
		}
	};

	template<> struct value_parser<int> {
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

		template<typename type_t>
		using prepend = type_list<type_t, types_t...>;

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
        template<static_string key_v>
        using get = typename storage_t::template at<type_value<key_v>>;

        template<static_string key_v, typename value_t>
        using set = type_builder<typename storage_t::template emplace<type_value<key_v>, value_t>>;

        template<template<typename> typename consumer_t>
        using build = consumer_t<storage_t>;
    };
}

// When defining builder fields, ensure `builder_t` is the name of the `meta::type_builder`
#define MGMAKE_META_TYPE_BUILDER_FIELD(wrapper_t, alias_t, ...) \
	MGMAKE_META_TYPE_BUILDER_FIELD_AS(wrapper_t, alias_t, ::mgmake::meta::static_string{ #alias_t }, __VA_ARGS__)

#define MGMAKE_META_TYPE_BUILDER_FIELD_AS(wrapper_t, alias_t, key_v, ...) \
	template<__VA_ARGS__ alias_t##_v> \
	using alias_t = wrapper_t< \
		typename builder_t::template set<key_v, meta::type_value<alias_t##_v>> \
	>

// When defining consumers, ensure `storage_t` is the name of the `meta::type_map`
#define MGMAKE_META_TYPE_CONSUMER_FIELD(alias_t, default_v) \
    MGMAKE_META_TYPE_CONSUMER_FIELD_AS(alias_t, ::mgmake::meta::static_string{ #alias_t }, default_v)

#define MGMAKE_META_TYPE_CONSUMER_FIELD_AS(alias_t, key_v, default_v) \
    using alias_t##_type = typename storage_t::template at< \
        ::mgmake::meta::type_value<key_v>, \
        false \
    >; \
    static constexpr auto alias_t##_value = \
        ::mgmake::meta::type_value_or<alias_t##_type, default_v>::value

#endif // MGMAKE_META_TYPE_BUILDER_HXX// ===== end include/mgmake/meta/type_builder.hxx =====


#include <expected>

namespace mgmake::cli {
    enum struct option_mode {
        deduce, // Auto deduce the mode based on the definition
        flag, // bool toggle on/off flag
        assign, // set text, int value, etc. assign the option value
        append, // append to the option list value. E.g. std::vector<std::string>
        callback // invoke a callback function when the option is passed
    };

    // Actual option impl, consume the configuration in the type map
    template<typename storage_t = meta::type_map<>>
    struct option_impl {
        MGMAKE_META_TYPE_CONSUMER_FIELD(name, meta::static_string{ "" });
        MGMAKE_META_TYPE_CONSUMER_FIELD(description, meta::static_string{ "" });
	    MGMAKE_META_TYPE_CONSUMER_FIELD(short_name, '\0');
	    MGMAKE_META_TYPE_CONSUMER_FIELD(mode, option_mode::deduce);
	    MGMAKE_META_TYPE_CONSUMER_FIELD(callback, nullptr);
	    using assign_type = typename storage_t::template at<meta::type_value<meta::static_string{ "assign" }>, false>;
        MGMAKE_META_TYPE_CONSUMER_FIELD(assign_hint, meta::static_string{ "value" });
	    using set_type = typename storage_t::template at<meta::type_value<meta::static_string{ "set" }>, false>;
	    MGMAKE_META_TYPE_CONSUMER_FIELD(action, false);
	    MGMAKE_META_TYPE_CONSUMER_FIELD(flag, true);

		static inline constexpr bool match(std::string_view arg) {
			if (arg.empty()) {
				return false;
			}

			// If the option is an action
			if constexpr (action_value) {
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
    };

    // Build a compile-time map for the option settings
    template<typename builder_t = meta::type_builder<>>
    struct option_builder {
        using builder_type = builder_t;

        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, name, meta::static_string);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, description, meta::static_string);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, short_name, char);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, mode, option_mode);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, callback, auto);
		// option accepts a value (`--switch=value` or `--switch value`) and assigns its value to the option member
		// pass a `meta::member_access<>` for the member to assign.
		template<typename member_t = meta::member_access<>>
        using assign = option_builder<typename builder_type::template set<"assign", member_t>>;
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, assign_hint, meta::static_string);
		// Sets the value at the member to the default value.
		template<typename member_t = meta::member_access<>, auto value_v = nullptr>
        using set = callback<[](auto& opts) {
			member_t::set(opts, value_v);
		}>;
		MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, action, bool);
		MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, flag, bool); // aka switch

        using build = typename builder_t::template build<option_impl>;
    };
    // default builder alias
    using option = option_builder<>;
}

#endif // MGMAKE_CLI_OPTION_HXX// ===== end include/mgmake/cli/option.hxx =====


// ===== begin include/mgmake/cli/options.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX


// ===== begin include/mgmake/cli/action.hxx =====
#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

#include <cstdint>

namespace mgmake::cli {
    struct action {
        enum struct kind : uint8_t {
            // meta actions, no cli required
            help,
            version,

            // normal actions, require cli
            tools,
            clean,
            generate,
            build, // default (see options.hxx)
            run,
            graph,

            // # of action kinds std::to_underlying(kind::count)
            count
        };
    };
}

#endif // MGMAKE_CLI_ACTION_HXX// ===== end include/mgmake/cli/action.hxx =====


#include <filesystem>

namespace mgmake::cli {
	// Store parsed CLI options
	struct options {
		// Build action by default
		action::kind m_action = action::kind::build;

		bool m_verbose = false;
		bool m_dry_run = false;

		std::filesystem::path m_build_dir = std::filesystem::current_path() / ".build";
	};
}

#endif // MGMAKE_CLI_OPTIONS_HXX// ===== end include/mgmake/cli/options.hxx =====


// skipped duplicate include: include/mgmake/meta/type_list.hxx

#include <print>

namespace mgmake::cli {
	// Actions
    using help_option = option
        ::name<"help">::short_name<'h'>
        ::description<"Show help.">
		::action<true>
		::set<meta::member_access<&options::m_action>, action::kind::help>
		::build;
	
	using build_option = option
		::name<"build">
		::description<"Build the project.">
		::action<true>::flag<false>
		::set<meta::member_access<&options::m_action>, action::kind::build>
		::build;
	
	// Switches
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
		::assign_hint<"path">
		::build;

    // Type list of default options
    // this way you can add your own option to default_options
    // before passing the list to the entry for your own CLI
    // options
    using default_options = meta::type_list<
        help_option,
		build_option,
		verbose_option,
		dry_run_option,
		build_dir_option
    >;
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX// ===== end include/mgmake/cli/default_options.hxx =====

// skipped duplicate include: include/mgmake/cli/options.hxx

// ===== begin include/mgmake/cli/parser.hxx =====
#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX

// skipped duplicate include: include/mgmake/cli/options.hxx


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

// skipped duplicate include: include/mgmake/meta/type_list.hxx

// ===== begin include/mgmake/sys/shell.hxx =====
#pragma once

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
}// ===== end include/mgmake/sys/shell.hxx =====


#include <bitset>
#include <expected>
#include <optional>
#include <string>
#include <utility>

namespace mgmake::cli {
    template<typename list_t = meta::type_list<>>
    struct parser {
		using list_type = list_t;

		// Action options (first arg, no - or --)
		using actions_type = typename list_type::template filter<[]<typename opt_t> -> bool {
			return opt_t::action_value;
		}>;
		// Switch option (- or -- prefix)
		using switches_type = typename list_type::template filter<[]<typename opt_t> -> bool {
			return not opt_t::action_value;
		}>;

        static inline constexpr std::expected<options, std::string> parse(const sys::shell& cmd) {
			// The resulting options
			options opts{};

			// Make the action parser
			//using action_parser = parser<actions_type>;
			//using action_match_t = action_parser::matches_type;

			auto args = cmd.user_args();
			/*
			auto has_action = args.size() > 0 and not args.at(0).starts_with("-");
			action_match_t action_matches{};
			// Have action? -> match it
			if (has_action) {
				auto action = args.at(0);

				// Any matches on actions?
				action_matches = action_parser::match(action);
				if (not action_matches.any()) {
					return std::unexpected{ std::format("Unknown action: '{}'", action) };
				}

				// args should now only be the switches
				args = args.subspan(1);
			}
			// Now we hold on to the matches until we parse the rest of the options
			*/

			// TODO: Figure out what to do about parsing values and assign switches
			// Iterate assign switches seperately?
			// Pass switch & value args together - DONE
			// Use the `assign_type` to know if the option is expecting a value - DONE
			// Use `set` to know the default? or if it is used without a value?
			// Use the `member_access::value_type` to know the expected type? - Done?
			// automatically handle value/assign hints from the value type?
			//   (E.g. a `std::filesystem::path` would say `--switch=path` instead of just `--switch=value`)
			//   (E.g. maybe also `std::string` would say `--switch=text`?)

			// Match switches
			for (auto it = args.begin(); it != args.end(); ++it) {
				std::string_view arg = *it;

				auto is_long = arg.starts_with("--");
				auto is_short = arg.starts_with("-");
				auto is_switch = is_long or is_short;
				auto is_action = it == args.begin() and not is_switch; // First and isn't switch? -> Action
				if (not is_switch and not is_action) {
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

					// 3) See if the arg is meant to be used as an action
					if (error_hint.empty()) {
						using action_parser = parser<actions_type>;
						auto matches = action_parser::match(arg);
						if (matches.any()) {
							auto corrected = std::format("{} {} ...", cmd.program_name(), arg);
							error_hint = std::format("'{}' is an action, did you mean '{}'?", arg, corrected);
						}
					}

					if (not error_hint.empty()) {
						return std::unexpected(std::format("Invalid argument: {} ({})", arg, error_hint));
					}
					return std::unexpected(std::format("Invalid argument: {}", arg));
				}
				mgmkassert(is_action or is_switch, "Values for switches should be skipped/parsed by the switch needing it");

				auto matches = match(arg);
				if (not matches.any()) {
					return std::unexpected(std::format("Unknown argument: '{}'", arg));
				}
				mgmkassert(matches.count() == 1, "Matched arg to more than one option?");

				auto index = detail::index_bit(matches);
				auto result = list_type::type_switch([&]<typename opt_t> -> std::expected<bool, std::string> {
					// If the option expects a value
					if constexpr (opt_t::is_assign) {
						// What is the expected value type?
						using assign_type = opt_t::assign_type;
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

					return std::unexpected("Not implemented");
				}, index);

				if (not result.has_value()) {
					return std::unexpected(result.error());
				}
			}

            return opts;
        }

		using matches_type = std::bitset<list_t::size()>;
		static inline constexpr matches_type match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type matches{};
				(matches.set(Is, list_t::template type_at<Is>::match(arg)), ...);
				return matches;
			}(std::make_index_sequence<list_t::size()>{}, arg);
		}
    };
}

#endif // MGMAKE_CLI_PARSER_HXX// ===== end include/mgmake/cli/parser.hxx =====



// ===== begin include/mgmake/sys/exit_code.hxx =====
#pragma once

#ifndef MGMAKE_SYS_EXIT_CODE_HXX
#define MGMAKE_SYS_EXIT_CODE_HXX

namespace mgmake::sys {
    enum struct exit_code : int {
        success,
        action_failure,
        usage_error
    };
}

#endif// ===== end include/mgmake/sys/exit_code.hxx =====

// skipped duplicate include: include/mgmake/sys/shell.hxx

#include <print>
#include <utility>

namespace mgmake::cli {
    template<auto project_v = nullptr, auto toolchains_v = nullptr, typename options_t = default_options>
    inline sys::exit_code entry(sys::shell cmd) {
        // construct the parser at compile time :)
        using p = parser<options_t>;

        // parse cmd at runtime
        if (auto result = p::parse(cmd)) {
			auto opts = result.value();
			if (opts.m_action == action::kind::help) {
				std::println("Help menu");
			}
            return sys::exit_code::success;
        } else {
            std::println(stderr, "{}", result.error());
            return sys::exit_code::usage_error;
        }
    }

    template<auto project_v = nullptr, auto toolchains_v = nullptr, typename options_t = default_options>
    inline sys::exit_code entry(int argc, char* argv[]) {
        return entry<project_v, toolchains_v, options_t>(sys::shell::from_args(argc, argv));
    }
}

#define MGMK_ENTRY(...) \
int main(int argc, char* argv[]) { \
    return std::to_underlying(::mgmake::cli::entry<__VA_ARGS__>(argc, argv)); \
}

#endif // MGMAKE_CLI_ENTRY_HXX// ===== end include/mgmake/cli/entry.hxx =====


#endif // MGMAKE_MGMAKE_HXX
// ===== end include/mgmake/mgmake.hxx =====


#endif // MGMAKE_SINGLE_HEADER_HXX
