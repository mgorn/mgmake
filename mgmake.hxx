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


// ===== begin include/mgmake/cli/parser.hxx =====
#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX


// ===== begin include/mgmake/cli/option_storage.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTION_STORAGE_HXX
#define MGMAKE_CLI_OPTION_STORAGE_HXX


// ===== begin include/mgmake/meta/static_dict.hxx =====
#pragma once

#ifndef MGMAKE_META_STATIC_DICT_HXX
#define MGMAKE_META_STATIC_DICT_HXX


// ===== begin include/mgmake/meta/static_string.hxx =====
#pragma once

#ifndef MGMAKE_META_STATIC_STRING_HXX
#define MGMAKE_META_STATIC_STRING_HXX

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
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

		[[nodiscard]] constexpr bool contains(auto other) const noexcept {
            return view().contains(other);
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

		[[nodiscard]] constexpr std::size_t distance(const std::convertible_to<std::string_view> auto& other) const noexcept {
			const auto lhs = view();
			const std::string_view rhs = other;

			std::array<std::size_t, N> previous{};
			std::array<std::size_t, N> current{};

			for (std::size_t i = 0; i <= lhs.size(); ++i) {
				previous[i] = i;
			}

			for (std::size_t j = 1; j <= rhs.size(); ++j) {
				current[0] = j;

				for (std::size_t i = 1; i <= lhs.size(); ++i) {
					const auto deletion = previous[i] + 1;
					const auto insertion = current[i - 1] + 1;
					const auto substitution = previous[i - 1] + (lhs[i - 1] == rhs[j - 1] ? 0 : 1);

					current[i] = std::min({ deletion, insertion, substitution });
				}

				previous.swap(current);
			}

			return previous.back();
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

	template<std::size_t N>
    struct formatter<::mgmake::meta::static_string<N>, char> : formatter<std::string_view, char> {
        using base_type = formatter<std::string_view, char>;

        template<typename format_context_t>
        constexpr auto format(const ::mgmake::meta::static_string<N>& value, format_context_t& context) const {
            return base_type::format(value.view(), context);
        }
    };
}

#endif // MGMAKE_META_STATIC_STRING_HXX// ===== end include/mgmake/meta/static_string.hxx =====


// ===== begin include/mgmake/meta/type_map.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_MAP_HXX
#define MGMAKE_META_TYPE_MAP_HXX


// ===== begin include/mgmake/meta/type_list.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_LIST_HXX
#define MGMAKE_META_TYPE_LIST_HXX


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


#include <array>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

// Compile-time list of types.
//
// `type_list` stores a variadic type pack as a named type. It has no runtime
// state. The list can be queried, extended, merged, filtered, sorted, folded,
// or applied to another variadic template with `apply`.

namespace mgmake::meta {
	template<typename... type_ts>
	struct type_list {
		static consteval std::size_t size() {
			return sizeof...(type_ts);
		}
		static consteval bool empty() {
			return size() == 0;
		}

		template<std::size_t index>
		using type_at = std::tuple_element_t<index, std::tuple<type_ts...>>;

		template<typename type_t>
		static consteval std::size_t count() {
			return (std::size_t{0} + ... + (std::same_as<type_t, type_ts> ? 1 : 0));
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
				std::same_as<type_t, type_ts>...
			};

			for (std::size_t i = 0; i < matches.size(); ++i) {
				if (matches.at(i)) {
					return i;
				}
			}

			return size();
		}

		template<typename... other_type_ts>
		using append_types = type_list<type_ts..., other_type_ts...>;

		template<typename type_t>
		using append = append_types<type_t>;

	private:
		// Needed because each unique append depends on the list produced by the previous append.
		template<typename current_list_t, typename... other_type_ts>
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

		template<typename current_list_t, typename... other_type_ts>
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
		template<typename... other_type_ts>
		using append_types_unique = typename append_types_unique_type<
			type_list<type_ts...>,
			other_type_ts...
		>::type;

		template<typename type_t, bool check = true>
		struct append_unique_type {
			static_assert((not check) or (not has<type_t>()), "type_list::append_unique cannot append a duplicate type.");

			using type = append_types_unique<type_t>;
		};

		template<typename type_t, bool check = true>
		using append_unique = typename append_unique_type<type_t, check>::type;

		template<typename... other_type_ts>
		using prepend_types = type_list<other_type_ts..., type_ts...>;

		template<typename type_t>
		using prepend = prepend_types<type_t>;

		template<typename... other_type_ts>
		using prepend_types_unique = typename prepend_types_unique_type<
			type_list<type_ts...>,
			other_type_ts...
		>::type;

		template<typename type_t, bool check = true>
		struct prepend_unique_type {
			static_assert((not check) or (not has<type_t>()), "type_list::prepend_unique cannot prepend a duplicate type.");

			using type = std::conditional_t<
				has<type_t>(),
				type_list<type_ts...>,
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
		using apply = pack_t<type_ts...>;

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
		using fold = typename fold_type<operation, initial_t, type_ts...>::type;

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

		template<typename callable_t>
		static constexpr void for_each(callable_t&& callable) {
			(callable.template operator()<type_ts>(), ...);
		}

		// Invoke the callable with the type at the runtime-selected index.
		template<typename callable_t>
		static constexpr decltype(auto) type_switch(callable_t&& callable, std::size_t index) {
			// A return type cannot be inferred without at least one stored type.
			//static_assert(size() > 0, "Cannot type-switch over an empty type_list.");
			// If the list is empty, return void
			if constexpr (empty()) {
				return;
			} else {
				mgmkassert(index < size(), "type_switch index is outside the bounds of the type_list");

				using first_type = type_at<0>;
				using return_t = decltype(std::declval<callable_t&&>().template operator()<first_type>());
				// A single dispatch table requires every specialization to share a return type.
				static_assert((std::same_as<return_t, decltype(std::declval<callable_t&&>().template operator()<type_ts>())> and ...), "Every type_switch invocation must return the same type.");

				using dispatch_t = return_t (*)(callable_t&&);
				// Generate one reusable dispatch entry for each type.
				static constexpr std::array<dispatch_t, size()> dispatch {
					+[](callable_t&& callable) -> return_t {
						return std::forward<callable_t>(callable).template operator()<type_ts>();
					}...
				};

				return dispatch[index](std::forward<callable_t>(callable));
			}
		}
	};
}

#endif // MGMAKE_META_TYPE_LIST_HXX// ===== end include/mgmake/meta/type_list.hxx =====


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
#include <type_traits>

namespace mgmake::meta {
	template<typename storage_t = type_list<>>
	struct type_map;

	template<typename... pair_ts>
	struct type_map<type_list<pair_ts...>> {
		using storage_type = type_list<pair_ts...>;
		using key_types = type_list<typename pair_ts::key_type...>;
		static_assert((key_types::template unique<typename pair_ts::key_type>() and ...), "type_map storage cannot contain duplicate keys.");
		using value_types = type_list<typename pair_ts::value_type...>;
		
		static consteval std::size_t size() {
			return storage_type::size();
		}

		template<typename key_t>
		static consteval std::size_t key_index() {
			constexpr std::array<bool, size()> matches {
				std::same_as<key_t, typename pair_ts::key_type>...
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
			return key_index<key_t>() < size();
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
                type_map<type_list<replace_pair<pair_ts, key_t, value_t>...>>,
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

// skipped duplicate include: include/mgmake/meta/type_pair.hxx

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


#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mgmake::meta {
	//md
	// `map_t = meta::type_map<meta::type_list<meta::type_pair<meta::type_value<key>, value>>>`
	// `key` = A `meta::static_string` value wrapped as `meta::type_value<key>`
	// `value` = The type for the value at that key
	// The actual `value`s are runtime, `key`s are known at compile time
	template<typename map_t = meta::type_map<>>
	struct static_dict {
		using map_type = map_t;
		using pairs_type = typename map_type::storage_type;
		// The types for keys (the `meta::type_value`s of `meta::static_string`s)
		using key_types = typename map_type::key_types;
		// The types for the values
		using value_types = typename map_type::value_types;

		// Store the values in a `std::tuple`
		using storage_type = typename value_types::template apply<std::tuple>;

		// Does the key exist?
		template<meta::static_string key_v>
		static consteval bool has() {
			using key_t = meta::type_value<key_v>;
			return map_type::template has<key_t>();
		}

		// Get the storage index for a key
		template<meta::static_string key_v>
		static consteval std::size_t key_index() {
			static_assert(has<key_v>(), "static_dict key doesn't exist");
			using key_t = meta::type_value<key_v>;
			return map_type::template key_index<key_t>();
		}

		// Reference to the stored value
		template<meta::static_string key_v>
		constexpr decltype(auto) storage() {
			constexpr std::size_t index = key_index<key_v>();
			return std::get<index>(m_storage);
		}
		template<meta::static_string key_v>
		constexpr decltype(auto) storage() const {
			constexpr std::size_t index = key_index<key_v>();
			return std::get<index>(m_storage);
		}

		// Accessors
		template<meta::static_string key_v>
		constexpr decltype(auto) get() {
			return storage<key_v>();
		}
		template<meta::static_string key_v>
		constexpr decltype(auto) get() const {
			return storage<key_v>();
		}

		// Setters
		template<meta::static_string key_v>
		constexpr void set(auto&& value) {
			static_assert(std::is_assignable_v<decltype(storage<key_v>()), decltype(value)>, "Cannot assign value at key with given value");
			storage<key_v>() = std::forward<decltype(value)>(value);
		}

		storage_type m_storage{};

	private:
		template<typename>
		static constexpr bool invalid_entry_v = false;

		template<typename pair_t>
		struct entry_check {
			static_assert(invalid_entry_v<pair_t>, "static_dict keys must be meta::type_value<meta::static_string>");
		};

		template<meta::static_string key_v, typename value_t>
		struct entry_check<meta::type_pair<meta::type_value<key_v>, value_t>> {
			static_assert(std::is_default_constructible_v<value_t>, "static_dict value type must be default-constructible");
			using type = void;
		};

		using entry_checks = typename pairs_type::template fold<[]<typename state_t, typename pair_t>() consteval {
			using check_t = entry_check<pair_t>;

			// Forces validation of this key/value pair.
			static_assert(std::is_same_v<typename check_t::type, void>);

			return std::type_identity<state_t>{};
		}, meta::type_list<>>;
		static_assert(std::is_same_v<entry_checks, meta::type_list<>>);
	};
}

#endif // MGMAKE_META_STATIC_DICT_HXX// ===== end include/mgmake/meta/static_dict.hxx =====


// ===== begin include/mgmake/meta/value_list.hxx =====
#pragma once

#ifndef MGMAKE_META_VALUE_LIST_HXX
#define MGMAKE_META_VALUE_LIST_HXX

// skipped duplicate include: include/mgmake/meta/type_list.hxx
// skipped duplicate include: include/mgmake/meta/type_value.hxx

#include <cstddef>
#include <type_traits>
#include <utility>

namespace mgmake::meta {
	// Compile-time list of values.
	//
	// Values are stored internally as:
	//
	//     type_list<type_value<value_vs>...>
	//
	// Public operations automatically wrap and unwrap values.
	template<auto... value_vs>
	struct value_list {
		using underlying_type = type_list<type_value<value_vs>...>;

		static consteval std::size_t size() {
			return underlying_type::size();
		}

		static consteval bool empty() {
			return underlying_type::empty();
		}

		template<std::size_t index>
		using type_at = typename underlying_type::template type_at<index>;
		template<std::size_t index>
		static inline constexpr auto value_at = [] {
			using wrapped_t = type_at<index>;
			return wrapped_t::value;
		}();

		template<auto value_v>
		static consteval std::size_t count() {
			return underlying_type::template count<type_value<value_v>>();
		}

		template<auto value_v>
		static consteval bool has() {
			return underlying_type::template has<type_value<value_v>>();
		}

		template<auto value_v>
		static consteval bool unique() {
			return underlying_type::template unique<type_value<value_v>>();
		}

		template<auto value_v>
		static consteval std::size_t index() {
			return underlying_type::template index<type_value<value_v>>();
		}

		template<auto... other_value_vs>
		using append_values = value_list<value_vs..., other_value_vs...>;

		template<auto value_v>
		using append = append_values<value_v>;

		// Convert a type_list of type_value wrappers into a value_list.
		template<typename list_t>
		using unwrap_list = typename list_t::template fold<
			[]<typename result_t, typename wrapped_t>() consteval {
				return std::type_identity<typename result_t::template append<wrapped_t::value>>{};
			},
			value_list<>
		>;

		template<auto... other_value_vs>
		using append_values_unique = unwrap_list<
			typename underlying_type::template append_types_unique<
				type_value<other_value_vs>...
			>
		>;

		template<auto value_v, bool check = true>
		struct append_unique_type {
			static_assert((not check) or (not has<value_v>()), "value_list::append_unique cannot append a duplicate value.");

			using type = append_values_unique<value_v>;
		};

		template<auto value_v, bool check = true>
		using append_unique = typename append_unique_type<value_v, check>::type;

		template<auto... other_value_vs>
		using prepend_values = value_list<other_value_vs..., value_vs...>;

		template<auto value_v>
		using prepend = prepend_values<value_v>;

		template<auto... other_value_vs>
		using prepend_values_unique = unwrap_list<
			typename underlying_type::template prepend_types_unique<
				type_value<other_value_vs>...
			>
		>;

		template<auto value_v, bool check = true>
		struct prepend_unique_type {
			static_assert((not check) or (not has<value_v>()), "value_list::prepend_unique cannot prepend a duplicate value.");

			using type = prepend_values_unique<value_v>;
		};

		template<auto value_v, bool check = true>
		using prepend_unique = typename prepend_unique_type<value_v, check>::type;

		template<typename other_list_t>
		using append_list = typename other_list_t::template apply<append_values>;

		template<typename other_list_t>
		using append_list_unique = typename other_list_t::template apply<append_values_unique>;

		template<typename other_list_t>
		using prepend_list = typename other_list_t::template apply<prepend_values>;

		template<typename other_list_t>
		using prepend_list_unique = typename other_list_t::template apply<prepend_values_unique>;

		// Invoke a variadic template with the stored value pack.
		template<template<auto...> typename pack_t>
		using apply = pack_t<value_vs...>;

		// Fold the stored values from left to right into an accumulated type.
		//
		// The operation should be callable as:
		//
		//     operation.template operator()<state_t, value_v>()
		//
		// and return:
		//
		//     std::type_identity<next_state_t>
		template<auto operation, typename initial_t>
		using fold = typename underlying_type::template fold<
			[]<typename state_t, typename wrapped_t>() consteval {
				return operation.template operator()<state_t, wrapped_t::value>();
			},
			initial_t
		>;

		// Keep the values for which the consteval NTTP predicate returns true.
		//
		// The predicate should be callable as:
		//
		//     predicate.template operator()<value_v>()
		template<auto predicate>
		using filter = unwrap_list<
			typename underlying_type::template filter<
				[]<typename wrapped_t>() consteval {
					return static_cast<bool>(predicate.template operator()<wrapped_t::value>());
				}
			>
		>;

		// Sort the values using a consteval NTTP comparator.
		//
		// The comparator should be callable as:
		//
		//     compare.template operator()<left_v, right_v>()
		//
		// and return true when left_v should appear before right_v.
		template<auto compare>
		using sort = unwrap_list<
			typename underlying_type::template sort<
				[]<typename left_t, typename right_t>() consteval {
					return static_cast<bool>(compare.template operator()<left_t::value, right_t::value>());
				}
			>
		>;

		template<typename callable_t>
		static constexpr void for_each(callable_t&& callable) {
			(callable.template operator()<value_vs>(), ...);
		}

		// Invoke the callable with the value at the runtime-selected index.
		template<typename callable_t>
		static constexpr decltype(auto) value_switch(callable_t&& callable, std::size_t index) {
			return underlying_type::type_switch(
				[&callable]<typename wrapped_t>() -> decltype(auto) {
					return std::forward<callable_t>(callable).template operator()<wrapped_t::value>();
				},
				index
			);
		}
	};
}

#endif // MGMAKE_META_VALUE_LIST_HXX// ===== end include/mgmake/meta/value_list.hxx =====


#include <utility>

namespace mgmake::cli {
	// Type list of all options for the build program
	template<typename opts_t = meta::value_list<>>
	struct option_storage {
		using list_type = opts_t;

	private:
		// When emplacing a key: this ensures if the key already exists, the types match
		// If they don't, a static_assert is thrown
		// If they do, the key was already added and can be skipped
		template<typename key_t, typename value_t, typename map_t>
		struct emplace_key_checked {
			using type = std::invoke_result_t<decltype([] consteval {
				// If the key exists
				if constexpr (map_t::template has<key_t>()) {
					// Get the type
					using in_map_type = typename map_t::template at<key_t>;
					// Assert they are the same
					static_assert(std::is_same_v<value_t, in_map_type>, "option storage key value type mismatch!");
					return std::type_identity<map_t>{};
				} else {
					return std::type_identity<typename map_t::template emplace_unique<key_t, value_t>>{};
				}
			})>::type;
		};
		//template<typename key_t, typename value_t, typename map_t>
		//using emplace_key_checked_t = emplace_key_checked<key_t, value_t, map_t>::type;

	public:
		// The type_map for the option storage key/value pairs
		using storage_map_type = typename list_type::template fold<[]<typename state_t, auto opt_v>() consteval {
			// If the option uses storage, it has a storage pair
			if constexpr (opt_v.has_storage) {
				// Get the pair
				using pair_type = decltype(opt_v.storage_pair());
				// Get the key
				using key_type = pair_type::key_type;
				// Get the value type
				using value_type = pair_type::value_type;

				// If the value_type is void, then it is determined by another option (typically a storage option)
				if constexpr (not std::is_same_v<value_type, void>) {
					// Check the key value & emplace it
					return std::type_identity<typename emplace_key_checked<key_type, value_type, state_t>::type>{};
				} else {
					// TODO: assert the key exists after constructing the full map
					return std::type_identity<state_t>{};
				}

			} else {
				// The option doesn't have storage, ignore it
				return std::type_identity<state_t>{};
			}
		}, meta::type_map<>>;

		// Ensure deferred `void` storage pairs resolve to a concrete key supplied by another option.
		using storage_validation = typename list_type::template fold<[]<typename state_t, auto opt_v>() consteval {
			if constexpr (opt_v.has_storage) {
				using pair_type = decltype(opt_v.storage_pair());
				if constexpr (std::is_same_v<typename pair_type::value_type, void>) {
					static_assert(storage_map_type::template has<typename pair_type::key_type>(), "void option storage value requires another option to define the key's value type");
				}
			}
			return std::type_identity<state_t>{};
		}, meta::type_list<>>;
		static_assert(std::is_same_v<storage_validation, meta::type_list<>>);

		// The storage type for the option values
		using storage_type = meta::static_dict<storage_map_type>;

		template<meta::static_string key_v>
		static consteval decltype(auto) has() {
			return storage_type::template has<key_v>();
		}

		template<meta::static_string key_v>
		constexpr decltype(auto) get() {
			return m_storage.template get<key_v>();
		}
		template<meta::static_string key_v>
		constexpr decltype(auto) get() const {
			return m_storage.template get<key_v>();
		}
		template<meta::static_string key_v>
		constexpr void set(auto&& value) {
			return m_storage.template set<key_v>(std::forward<decltype(value)>(value));
		}

		// Overloads for option types
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		static inline consteval decltype(auto) has() {
			return has<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr decltype(auto) get() {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template get<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr decltype(auto) get() const {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template get<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr void set(auto&& value) {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template set<opt_v.storage_key()>(std::forward<decltype(value)>(value));
		}

		// Value storage
		storage_type m_storage{};
	};
}

#endif // MGMAKE_CLI_OPTION_STORAGE_HXX// ===== end include/mgmake/cli/option_storage.hxx =====



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

// ===== begin include/mgmake/meta/type_traits.hxx =====
#pragma once

#ifndef MGMAKE_META_TYPE_TRAITS_HXX
#define MGMAKE_META_TYPE_TRAITS_HXX

#include <array>
#include <type_traits>
#include <vector>

namespace mgmake::meta {
	template<typename T>
	struct array_traits : std::false_type {};

	template<typename T, std::size_t size_v>
	struct array_traits<std::array<T, size_v>> : std::true_type {
		using type = std::array<T, size_v>;
		using value_type = T;

		static inline constexpr std::size_t size = size_v;

		using size_type = typename type::size_type;
		using difference_type = typename type::difference_type;
		using reference = typename type::reference;
		using const_reference = typename type::const_reference;
		using pointer = typename type::pointer;
		using const_pointer = typename type::const_pointer;
		using iterator = typename type::iterator;
		using const_iterator = typename type::const_iterator;
	};

	template<typename T>
	using array_traits_t = array_traits<std::remove_cvref_t<T>>;

	template<typename T>
	concept array_type = array_traits_t<T>::value;

	template<typename T>
	inline constexpr bool is_array_v = array_traits<std::remove_cvref_t<T>>::value;

	template<typename T>
	struct vector_traits : std::false_type {};

	template<typename T, typename allocator_t>
	struct vector_traits<std::vector<T, allocator_t>> : std::true_type {
		using type = std::vector<T, allocator_t>;
		using value_type = T;
		using allocator_type = allocator_t;

		using size_type = typename type::size_type;
		using difference_type = typename type::difference_type;
		using reference = typename type::reference;
		using const_reference = typename type::const_reference;
		using pointer = typename type::pointer;
		using const_pointer = typename type::const_pointer;
		using iterator = typename type::iterator;
		using const_iterator = typename type::const_iterator;
	};

	template<typename T>
	using vector_traits_t = vector_traits<std::remove_cvref_t<T>>;

	template<typename T>
	concept vector_type = vector_traits_t<T>::value;

	template<typename T>
	inline constexpr bool is_vector_v = vector_traits<std::remove_cvref_t<T>>::value;
}

#endif // MGMAKE_META_TYPE_TRAITS_HXX// ===== end include/mgmake/meta/type_traits.hxx =====


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
    template<typename option_storage_t = option_storage<>>
    struct parser {
		using option_storage_type = option_storage_t;
		using list_type = option_storage_type::list_type;

		// Task options (first arg, no - or --)
		using tasks_type = typename list_type::template filter<[]<auto opt_v> -> bool {
			return opt_v.task();
		}>;
		// Switch option (- or -- prefix)
		using switches_type = typename list_type::template filter<[]<auto opt_v> -> bool {
			return opt_v.flag();
		}>;

		template<typename dispatcher_t>
        static inline constexpr std::expected<option_storage_type, std::string> parse(const sys::shell& cmd) {
			// The resulting options
			option_storage_type opts{};

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
						auto matches = match<list_type>(std::format("-{}", arg));
						if (matches.any()) {
							error_hint = std::format("Did you mean '-{}'?", arg);
						}
					}

					// 2) See if it could have been a long switch
					if (error_hint.empty()) {
						auto matches = match<list_type>(std::format("--{}", arg));
						if (matches.any()) {
							error_hint = std::format("Did you mean '--{}'?", arg);
						}
					}

					// 3) See if the arg is meant to be used as an task
					if (error_hint.empty()) {
						auto matches = match<tasks_type>(arg);
						if (matches.any()) {
							auto corrected = std::format("{} {} ...", cmd.program_name(), arg);
							error_hint = std::format("'{}' is a task, did you mean '{}'?", arg, corrected);
						}
					}

					if (not error_hint.empty()) {
						return std::unexpected(std::format("Invalid argument: {} ({})", arg, error_hint));
					}
					return std::unexpected(std::format("Invalid argument: '{}'", arg));
				}
				mgmkassert(is_task or is_switch, "Values for switches should be skipped/parsed by the switch needing it");

				// Shrimply doesn't exit?
				auto matches = match<list_type>(arg);
				if (not matches.any()) {
					std::string error_hint = "";

					// Trim the arg
					std::string_view arg_prefix = "";
					std::string_view arg_name = arg;
					if (is_long) {
						arg_prefix = arg.substr(0, 2);
						arg_name = arg.substr(2);
					} else if (is_short) {
						arg_prefix = arg.substr(0, 1);
						arg_name = arg.substr(1);
					}					

					if (is_long) {
						// 1) Use string distance algorithm to find the closest
						std::size_t dist = -1;
						if (error_hint.empty()) {
							std::string closest = "";
							switches_type::for_each([&]<auto opt_v>{
								constexpr auto name = opt_v.name();
								if (not name.empty()) {
									auto name_dist = name.distance(arg_name);
									if (name_dist < dist) {
										closest = name;
										dist = name_dist;
									}
								}

								constexpr auto alias = opt_v.alias();
								if (not alias.empty()) {
									auto alias_dist = alias.distance(arg_name);
									if (alias_dist < dist) {
										closest = alias;
										dist = alias_dist;
									}
								}
							});
							if (dist < arg.size()) {
								error_hint = std::format("Did you mean '{}{}'?", arg_prefix, closest);
							}
						}

						// 2) Check if its contained in a name
						if (error_hint.empty() or (dist > 1)) {
							switches_type::for_each([&]<auto opt_v>{
								constexpr auto name = opt_v.name();
								// If arg is in name
								if (name.contains(arg_name)) {
									error_hint = std::format("Did you mean '{}{}'?", arg_prefix, name);
								}

								// or the alias
								constexpr auto alias = opt_v.alias();
								if (alias.contains(arg_name)) {
									error_hint = std::format("Did you mean '{}{}'?", arg_prefix, alias);
								}
							});
						}
					}

					if (error_hint.empty()) {
						return std::unexpected(std::format("Unknown argument: '{}'", arg));
					} else {
						return std::unexpected(std::format("Unknown argument: '{}' ({})", arg, error_hint));
					}
				}
				// If this happens, there's a conflict with option names (either long or short)
				mgmkassert(matches.count() == 1, "Matched arg to more than one option?");

				auto index = detail::index_bit(matches);
				auto result = list_type::value_switch([&]<auto opt_v> -> std::expected<bool, std::string> {
					// If the option expects a value
					if constexpr (opt_v.parses()) {
						// What is the expected value type?
						// TODO: If value_type is a std::vector or other container,
						// we need to keep reading each arg, parse them, and store...
						using value_type = decltype(opt_v)::storage_value_type;

						// Is it `--switch=value` or `--switch value`?
						if (const auto seperator = arg.find_first_of("="); seperator != std::string_view::npos) {
							std::string_view value_text = arg.substr(seperator+1);
							arg = arg.substr(0, seperator);

							// assign
							auto result = opt_v.handle_parse(opts, arg, value_text);
							if (not result) {
								return std::unexpected(std::format("opt_t::handle_assign failed: {}", result.error()));
							}
						} else {
							if constexpr (meta::is_vector_v<value_type>) {
								std::size_t parsed_values = 0;
								for (auto next_it = std::next(it); next_it != args.end(); ++next_it) {
									std::string_view value_text = *next_it;
									if (match<switches_type>(value_text).any()) {
										break;
									}

									auto result = opt_v.handle_parse(opts, arg, value_text);
									if (not result) {
										return std::unexpected(std::format("opt_t::handle_assign failed: {}", result.error()));
									}

									++parsed_values;
									it = next_it;
								}

								if (parsed_values == 0) {
									return std::unexpected(std::format("argument '{}' expects at least one value", arg));
								}
							} else {
								// Get the next arg
								it = std::next(it);
								if (it == args.end()) {
									return std::unexpected(std::format("argument '{}' expects a value", arg));
								}

								std::string_view value_text = *it;
								// assign
								auto result = opt_v.handle_parse(opts, arg, value_text);
								if (not result) {
									return std::unexpected(std::format("opt_t::handle_assign failed: {}", result.error()));
								}
							}
						}

						return true;
					}
					
					// Tasks must be handled before callbacks because task options may also define callbacks.
					if constexpr (opt_v.task()) {
						auto result = opt_v.template handle_task<dispatcher_t>(opts, arg);
						if (not result) {
							return std::unexpected(std::format("opt_t::handle_task failed: {}", result.error()));
						}

						return true;
					}

					// If the option invokes a callback
					if constexpr (opt_v.is_callback) {
						auto result = opt_v.handle_callback(opts, arg);
						if (not result) {
							return std::unexpected(std::format("opt_v.handle_callback failed: {}", result.error()));
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

		template<typename opts_t>
		using matches_type = std::bitset<opts_t::size()>;
		template<typename opts_t>
		static inline constexpr matches_type<opts_t> match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type<opts_t> matches{};
				(matches.set(Is, opts_t::template value_at<Is>.match(arg)), ...);
				return matches;
			}(std::make_index_sequence<opts_t::size()>{}, arg);
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
        task_failure,
        usage_error
    };
}

#endif// ===== end include/mgmake/sys/exit_code.hxx =====

// skipped duplicate include: include/mgmake/sys/shell.hxx

// ===== begin include/mgmake/task/dispatcher.hxx =====
#pragma once

#ifndef MGMAKE_TASK_DISPATCHER_HXX
#define MGMAKE_TASK_DISPATCHER_HXX


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

// skipped duplicate include: include/mgmake/meta/static_string.hxx

#include <charconv>
#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <vector>

// Value parsers convert one option argument string into a typed destination value.

namespace mgmake::cli {
	template<typename type_t>
	struct value_parser {
		// Hint for the value type in help menu
		static inline constexpr meta::static_string help_hint = "value";
	};

	template<> struct value_parser<std::string> {
		static inline constexpr meta::static_string help_hint = "string";

		[[nodiscard]] static std::expected<std::string, std::string> parse(std::string_view text) {
			return std::string{ text };
		}
	};

	template<> struct value_parser<int> {
		static inline constexpr meta::static_string help_hint = "integer";

		[[nodiscard]] static std::expected<int, std::string> parse(std::string_view text) {
			if (text.empty()) {
				return std::unexpected(std::format("invalid integer value '{}' (empty)", text));
			}

			int value = 0;
			auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
			if (error != std::errc{} or end != text.data() + text.size()) {
				return std::unexpected(std::format("invalid integer value '{}'", text));
			}
			return value;
		}
	};

	template<> struct value_parser<std::filesystem::path> {
		static inline constexpr meta::static_string help_hint = "path";

		[[nodiscard]] static std::expected<std::filesystem::path, std::string> parse(std::string_view text) {
			std::filesystem::path path{ text };
			if (path.is_absolute()) {
				return path;
			}
			return std::filesystem::current_path() / path;
		}
	};

	template<typename type_t> struct value_parser<std::vector<type_t>> {
		using item_parser = value_parser<type_t>;
		static inline constexpr meta::static_string help_hint = item_parser::help_hint + meta::static_string{"s..."};

		[[nodiscard]] static std::expected<type_t, std::string> parse(std::string_view text) {
			return item_parser::parse(text);
		}
	};
}

#endif // MGMAKE_CLI_VALUE_PARSER_HXX
// ===== end include/mgmake/cli/value_parser.hxx =====


// skipped duplicate include: include/mgmake/detail/assert.hxx
// skipped duplicate include: include/mgmake/detail/index_bit.hxx

// ===== begin include/mgmake/meta/builder_mixin.hxx =====
#pragma once

#ifndef MGMAKE_META_BUILDER_MIXIN_HXX
#define MGMAKE_META_BUILDER_MIXIN_HXX

// skipped duplicate include: include/mgmake/meta/static_string.hxx

namespace mgmake::meta {
	struct named {
		template<meta::static_string value_v>
		consteval auto name(this auto const& self) {
			return self.template set_str<"name", value_v>();
		}
		consteval auto name(this auto const& self) {
			return self.template get_str<"name">();
		}
	};
}

#endif // MGMAKE_META_BUILDER_MIXIN_HXX// ===== end include/mgmake/meta/builder_mixin.hxx =====


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

// skipped duplicate include: include/mgmake/meta/type_list.hxx

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

// skipped duplicate include: include/mgmake/meta/static_string.hxx
// skipped duplicate include: include/mgmake/meta/type_map.hxx

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

// skipped duplicate include: include/mgmake/meta/type_value.hxx

namespace mgmake::meta {
    template<template<typename = type_map<>> typename impl_t, typename storage_t = type_map<>>
    struct type_builder {
		using storage_type = storage_t;

		template<static_string key_v>
		static consteval bool has() {
			return storage_type::template has<type_value<key_v>>();
		}

        template<static_string key_v, bool check_v = true>
        using get_type = typename storage_t::template at<type_value<key_v>, check_v>;
		template<static_string key_v, typename default_t>
		using get_type_or = type_or_t<get_type<key_v, false>, default_t>;
        template<static_string key_v, bool check_v = true>
		static consteval auto get_value() {
			return get_type<key_v, check_v>::value;
		}
		template<static_string key_v, auto default_v = nullptr>
		static consteval auto get_value_or() {
			if constexpr (has<key_v>()) {
				return get_value<key_v>();
			} else {
				return default_v;
			}
		}
		template<static_string key_v>
		static consteval auto get_str() {
			return get_value_or<key_v, static_string{ "" }>();
		}

        template<static_string key_v, typename value_t>
        using set_type = impl_t<typename storage_t::template emplace<type_value<key_v>, value_t>>;
		template<static_string key_v, auto value_v>
		static consteval auto set_value() {
			return set_type<key_v, type_value<value_v>>{};
		}
		template<static_string key_v, static_string str_v>
		static consteval auto set_str() {
			return set_type<key_v, type_value<str_v>>{};
		}
    };
}

#endif // MGMAKE_META_TYPE_BUILDER_HXX// ===== end include/mgmake/meta/type_builder.hxx =====

// skipped duplicate include: include/mgmake/meta/type_traits.hxx

#include <expected>

namespace mgmake::cli {
    // Actual configurable option impl
    template<typename storage_t = meta::type_map<>>
    struct option_impl : public meta::type_builder<option_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<option_impl, storage_t>;

		using builder_type::get_type_or;

		template<meta::static_string value_v>
		static consteval auto alias() {
			return builder_type::template set_str<"alias", value_v>();
		}
		static consteval auto alias() {
			return builder_type::template get_str<"alias">();
		}

		template<meta::static_string value_v>
		static consteval auto description() {
			return builder_type::template set_str<"description", value_v>();
		}
		static consteval auto description() {
			return builder_type::template get_str<"description">();
		}

		template<char value_v = '\0'>
		static consteval auto short_name() {
			return builder_type::template set_value<"short_name", value_v>();
		}
		static consteval char short_name() {
			return builder_type::template get_value_or<"short_name", '\0'>();
		}

		template<auto value_v = nullptr>
		static consteval auto callback() {
			return builder_type::template set_value<"callback", value_v>();
		}
		static consteval auto callback() {
			return builder_type::template get_value_or<"callback", nullptr>();
		}

		// Takes a `meta::type_pair<meta::static_string, value_type>` for the option value storage
		// This is what adds the key and value type to the `option_storage`
		template<typename pair_t>
		static consteval auto storage_pair() -> typename builder_type::template set_type<"storage_pair", pair_t> {
			return {};
		}
		static consteval auto storage_pair() -> typename builder_type::template get_type<"storage_pair", false> {
			return {};
		}

		// Set the option to set a specific value & assigns the storage pair
		template<meta::static_string key_v, auto value_v = std::nullopt>
        static consteval auto set() {
			return callback<[](auto& opts) {
				static_assert(not std::is_same_v<decltype(value_v), std::nullopt_t>, "No value passed to `option::set<>` (Do we actually need to set the value to nullopt?)");
				opts.template set<key_v>(value_v);
			}>().template storage_pair<typename meta::type_pair<meta::type_value<key_v>, std::remove_cvref_t<decltype(value_v)>>>();
		}

		// If the option parses a value (`--switch=value` or `--switch value`) and stores it
		template<bool value_v = true>
		static consteval auto parses() {
			return builder_type::template set_value<"parses", value_v>();
		}
		static consteval bool parses() {
			return builder_type::template get_value_or<"parses", false>();
		}

		// Set the option to parse a value & assigns the storage pair
		template<meta::static_string key_v, typename parse_t>
		static consteval auto parse() {
			return parses<true>().template storage_pair<typename meta::type_pair<meta::type_value<key_v>, parse_t>>();
		}

		template<bool value_v = true>
		static consteval auto task() {
			return builder_type::template set_value<"task", value_v>();
		}
		static consteval bool task() {
			return builder_type::template get_value_or<"task", false>();
		}

		// aka switch
		template<bool value_v = true>
		static consteval auto flag() {
			return builder_type::template set_value<"flag", value_v>();
		}
		static consteval bool flag() {
			return builder_type::template get_value_or<"flag", true>();
		}

		// The option is only for reserving a key/value in storage
		// this disables task and flag
		template<meta::static_string key_v, typename value_t>
		static consteval auto storage() {
			return task<false>()
				.template flag<false>()
				.template storage_pair<typename meta::type_pair<meta::type_value<key_v>, value_t>>();
		}

		static inline constexpr bool has_storage = not std::is_same_v<decltype(storage_pair()), void>;
		// The key for the storage value, else the option name
		static inline constexpr decltype(auto) storage_key() {
			if constexpr (has_storage) {
				return decltype(storage_pair())::key_type::value;
			} else {
				return option_impl{}.name();
			}
		}
		// The value for the storage, else void
		using storage_value_type = std::invoke_result_t<decltype([] consteval {
			if constexpr (has_storage) {
				return std::type_identity<typename decltype(storage_pair())::value_type>{};
			} else {
				return std::type_identity<void>{};
			}
		})>::type;

		static inline constexpr bool match(std::string_view arg) {
			if (arg.empty()) {
				return false;
			}

			// If the option is a task
			if constexpr (task()) {
				if (arg == option_impl{}.name()) {
					return true;
				}
			}

			// Parse as switch
			if constexpr (flag()) {
				if (arg.starts_with("--")) {
					return match_long(arg.substr(2));
				} else if (arg.starts_with("-")) {
					return match_short(arg.substr(1));
				}
			}
			return false;
		}

		static inline constexpr bool match_long_name(std::string_view arg, std::string_view name) {
			return arg == name or (arg.size() > name.size() and arg.starts_with(name) and arg[name.size()] == '=');
		}

		static inline constexpr bool match_long(std::string_view arg) {
			if (match_long_name(arg, option_impl{}.name())) {
				return true;
			}

			if constexpr (not alias().empty()) {
				return match_long_name(arg, alias());
			}
			return false;
		}

		static inline constexpr bool match_short(std::string_view arg) {
			// handle short = val (e.g. -g=ninja/-g ninja or smth)
			return arg.size() == 1 and arg.front() == short_name();
		}

		static inline constexpr bool is_callback = not std::is_same_v<std::decay_t<decltype(callback())>, std::nullptr_t>;
		static inline constexpr std::expected<void, std::string> handle_callback(auto& opts, std::string_view arg) {
			mgmkassert(is_callback, "option_impl::handle_callback called for non-callback option");
			mgmkassert(match(arg), "handling a callback for the incorrect arg");

			if constexpr (is_callback) {
				callback()(opts);
				return {};
			} else {
				return std::unexpected("option_impl::handle_callback called for an option with no callback");
			}
		}
 
		static inline constexpr std::expected<void, std::string> handle_parse(auto& opts, std::string_view arg, std::string_view value) {
			mgmkassert(match(arg), "handling a switch with the incorrect arg");
			mgmkassert(parses(), "handling a normal switch as a value parse switch");

			if constexpr (parses()) {
				// parse the storage_value_type
				using vp = value_parser<storage_value_type>;
				auto result = vp::parse(value);
				if (not result.has_value()) {
					return std::unexpected(std::format("Error parsing value for arg '{}': {}", arg, result.error()));
				}

				// If its a vector, emplace back
				if constexpr (meta::is_vector_v<storage_value_type>) {
					opts.template get<storage_key()>().emplace_back(result.value());
				} else {
					// otherwise just assign
					opts.template set<storage_key()>(result.value());
				}
			}
			return {};
		}

		template<typename dispatcher_t>
		static inline constexpr std::expected<void, std::string> handle_task(auto& opts, std::string_view arg) {
			mgmkassert(match(arg), "handling a task with the incorrect arg");
			mgmkassert(task(), "handling a normal switch as a task");

			auto matches = dispatcher_t::match(arg);
			if (not matches.any()) {
				return std::unexpected(std::format("Unknown task '{}' (cli::option_impl::handle_task no match from dispatcher_t::match for arg)", arg));
			}
			opts.template set<"task">(detail::index_bit(matches));
			return {};
		}
    };
    // default builder alias
    static constexpr auto option = option_impl<>{};
}

#endif // MGMAKE_CLI_OPTION_HXX// ===== end include/mgmake/cli/option.hxx =====


// skipped duplicate include: include/mgmake/meta/value_list.hxx

#include <print>
#include <string>
#include <vector>

namespace mgmake::cli {
	static constexpr auto task_option = option
		.name<"task">()
		.description<"Decides which task should run.">()
		.storage<"task", std::size_t>();

	static constexpr auto verbose_option = option
		.name<"verbose">().short_name<'v'>()
		.description<"Print commands before executing them.">()
		.set<"verbose", true>();
	
	static constexpr auto dry_run_option = option
		.name<"dry-run">().short_name<'d'>()
		.description<"Print commands without executing them.">()
		.set<"dry_run", true>();

	static constexpr auto build_dir_option = option
		.name<"build-dir">().short_name<'b'>()
		.description<"Set the build directory.">()
		.parse<"build_dir", std::filesystem::path>();

	static constexpr auto targets_option = option
		.name<"targets">().alias<"target">().short_name<'t'>()
		.description<"Build a specific target. May be passed multiple times.">()
		.parse<"targets", std::vector<std::string>>();
	
    // Type list of default options
	//
    // this way you can add your own options to 
    // default_options before passing the list 
    // to your mgmake config for your own CLI
    static constexpr auto default_options = meta::value_list<
		task_option,
		verbose_option,
		dry_run_option,
		build_dir_option,
		targets_option
	>{};
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX// ===== end include/mgmake/cli/default_options.hxx =====


// ===== begin include/mgmake/task/task_traits.hxx =====
#pragma once

#ifndef MGMAKE_TASK_TASK_TRAITS_HXX
#define MGMAKE_TASK_TASK_TRAITS_HXX

// skipped duplicate include: include/mgmake/sys/exit_code.hxx

#include <concepts>
#include <expected>
#include <string>

namespace mgmake::sys {
	struct shell;
}

namespace mgmake::task {
	template<typename task_t, auto config_v>
	concept task_handler = requires(const sys::shell& cmd, const typename decltype(config_v.option_storage())::type& opts) {
		{
			task_t::template handle<config_v>(cmd, opts)
		} -> std::same_as<std::expected<sys::exit_code, std::string>>;
	};

	template<typename task_t>
	struct task_traits {
		using task_type = task_t;
		static constexpr auto option = task_type::option;

		template<auto config_v>
		static constexpr bool valid_handler = task_handler<task_type, config_v>;

		static constexpr auto name() {
			return option.name();
		}
		static constexpr auto description() {
			return option.description();
		}

		static constexpr bool match(std::string_view arg) {
			return option.match(arg);
		}
	};
}

#endif // MGMAKE_TASK_TASK_HXX// ===== end include/mgmake/task/task_traits.hxx =====


// skipped duplicate include: include/mgmake/sys/exit_code.hxx
// skipped duplicate include: include/mgmake/sys/shell.hxx

// task::dispatcher
// consumes the cli::options and executes the required task(s)

namespace mgmake::task {
	// The mgmake config
    template<auto config_v>
	struct dispatcher {
		using list_type = decltype(config_v.tasks());

		static inline constexpr std::expected<sys::exit_code, std::string> invoke(const sys::shell& cmd, const auto& opts) {
			if constexpr (not opts.template has<cli::task_option>()) {
				return std::unexpected("cli::dispatcher::invoke cannot invoke without a task!");
			} else {
				return list_type::type_switch([&]<typename task_t> -> std::expected<sys::exit_code, std::string> {
					using traits_type = task_traits<task_t>;
					static_assert(traits_type::template valid_handler<config_v>, "task is missing a handle function");

					return task_t::template handle<config_v>(cmd, opts);
				}, opts.template get<cli::task_option>());
			}
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

// skipped duplicate include: include/mgmake/cli/option_storage.hxx
// skipped duplicate include: include/mgmake/cli/default_options.hxx
// skipped duplicate include: include/mgmake/meta/type_builder.hxx
// skipped duplicate include: include/mgmake/meta/type_or.hxx

// ===== begin include/mgmake/task/default_tasks.hxx =====
#pragma once

#ifndef MGMAKE_CLI_DEFAULT_TASKS_HXX
#define MGMAKE_CLI_DEFAULT_TASKS_HXX


// ===== begin include/mgmake/task/clean.hxx =====
#pragma once

#ifndef MGMAKE_TASK_CLEAN_HXX
#define MGMAKE_TASK_CLEAN_HXX

// skipped duplicate include: include/mgmake/cli/default_options.hxx
// skipped duplicate include: include/mgmake/sys/exit_code.hxx

#include <print>

namespace mgmake::task {
	struct clean {
		static constexpr auto option = cli::option
			.name<"clean">()
			.description<"Delete all build files.">()
			.task<true>().flag<false>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			// TODO: This would be the entrypoint/root for clean
			std::println("Clean task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_CLEAN_HXX// ===== end include/mgmake/task/clean.hxx =====


// ===== begin include/mgmake/task/build.hxx =====
#pragma once

#ifndef MGMAKE_TASK_BUILD_HXX
#define MGMAKE_TASK_BUILD_HXX

// skipped duplicate include: include/mgmake/cli/default_options.hxx

// ===== begin include/mgmake/spec/traits.hxx =====
#pragma once

#ifndef MGMAKE_SPEC_TRAITS_HXX
#define MGMAKE_SPEC_TRAITS_HXX

namespace mgmake::spec {
	template<typename spec_t>
	concept has_links = requires {
		typename spec_t::links;
	};

	template<auto spec_v>
	concept collects_targets = requires {
		{ spec_v.collect_targets() };
	};
}

#endif // MGMAKE_SPEC_TRAITS_HXX// ===== end include/mgmake/spec/traits.hxx =====

// skipped duplicate include: include/mgmake/sys/exit_code.hxx

#include <print>
#include <type_traits>

namespace mgmake::task {
	struct build {
		static constexpr auto option = cli::option
			.name<"build">()
			.description<"Build the project.">()
			.task<true>().flag<false>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			// TODO: This would be the entrypoint/root for build
			std::println("Build task");
			std::println("Build dir: {}", opts.template get<cli::build_dir_option>().string());
			std::println("Verbose: {}", opts.template get<cli::verbose_option>());
			std::print("Target(s): ");
			auto& targets = opts.template get<cli::targets_option>();
			for (auto it = targets.begin(); it != targets.end(); ++it) {
				std::print("{}", *it);
				if (std::next(it) != targets.end()) {
					std::print(", ");
				}
			}
			std::println("");

			constexpr auto project = config_v.project();
			if constexpr (not std::is_same_v<std::remove_cvref_t<decltype(project)>, std::nullptr_t>) {
				constexpr auto targets = project.all_targets();

				targets.for_each([]<auto target_v>{
					if constexpr (spec::collects_targets<target_v>) {
						std::println("PROJ LINKS TARGET: '{}'", target_v.name());
					} else {
						std::println("PROJ TARGET: '{}'", target_v.name());
					}
				});
			}
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_BUILD_HXX// ===== end include/mgmake/task/build.hxx =====


// ===== begin include/mgmake/task/fetch.hxx =====
#pragma once

#ifndef MGMAKE_TASK_FETCH_HXX
#define MGMAKE_TASK_FETCH_HXX

// skipped duplicate include: include/mgmake/cli/default_options.hxx
// skipped duplicate include: include/mgmake/sys/exit_code.hxx

#include <print>

namespace mgmake::task {
	struct fetch {
		static constexpr auto option = cli::option
			.name<"fetch">()
			.description<"Fetch (download, find, etc) all project dependencies.">()
			.task<true>().flag<false>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			// TODO: This would be the entrypoint/root for fetch
			std::println("Fetch task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_FETCH_HXX// ===== end include/mgmake/task/fetch.hxx =====


// ===== begin include/mgmake/task/help.hxx =====
#pragma once

#ifndef MGMAKE_TASK_HELP_HXX
#define MGMAKE_TASK_HELP_HXX

// skipped duplicate include: include/mgmake/task/task_traits.hxx

// skipped duplicate include: include/mgmake/cli/option.hxx
// skipped duplicate include: include/mgmake/sys/exit_code.hxx

#include <print>
#include <sstream>
#include <string>

namespace mgmake::task {
	struct help {
		static constexpr auto option = cli::option
			.name<"help">().short_name<'h'>()
			.description<"Show help.">()
			.task<true>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, auto& opts) {
			std::println("Usage:");
			std::println("\t{} [task] [options]", cmd.program_name());
			using tasks_type = decltype(config_v.tasks());
			using options_type = decltype(config_v.option_storage())::type::list_type;
			
			std::println("\nTasks:");
			static constexpr auto task_help = []<typename task_t>(auto& cmd){
				using traits_type = task_traits<task_t>;
				std::println("\t{:<10} {}", traits_type::name(), traits_type::description());
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>, auto& cmd) {
				(task_help.template operator()<typename tasks_type::template type_at<Is>>(cmd), ...);
			}(std::make_index_sequence<tasks_type::size()>{}, cmd);

			std::println("\nOptions:");
			static constexpr auto option_help = []<auto opt_v>{
				// Only print switches, tasks will be shown first
				if constexpr (opt_v.flag()) {
					std::stringstream ss;
					if constexpr (opt_v.short_name() != '\0') {
						std::print(ss, "-{}, ", opt_v.short_name());
					}
					std::print(ss, "--{}", opt_v.name().view());
					if constexpr (opt_v.parses()) {
						using vp = cli::value_parser<typename decltype(opt_v)::storage_value_type>;
						std::print(ss, "=<{}>", vp::help_hint);
					}
					std::println("\t{:<24} {}", ss.str(), opt_v.description().view());
				}
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>) {
				(option_help.template operator()<options_type::template value_at<Is>>(), ...);
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
		task::build, // The default task when none is specified
		task::help,
		task::clean,
		task::fetch
	>;
}

#endif // MGMAKE_CLI_DEFAULT_TASKS_HXX// ===== end include/mgmake/task/default_tasks.hxx =====


namespace mgmake {
	template<typename storage_t = meta::type_map<>>
    struct config_impl : public meta::type_builder<config_impl, storage_t> {
		using builder_type = meta::type_builder<config_impl, storage_t>;

		template<auto project_v>
		static consteval auto project() {
			return builder_type::template set_value<"project", project_v>();
		}
		static consteval auto project() {
			return builder_type::template get_value_or<"project", nullptr>();
		}
		template<auto toolchains_v>
		static consteval auto toolchains() {
			return builder_type::template set_value<"toolchains", toolchains_v>();
		}
		template<typename tasks_t>
		static consteval auto tasks() -> builder_type::template set_type<"tasks", tasks_t> {
			return {};
		}
		static consteval auto tasks() -> builder_type::template get_type_or<"tasks", task::default_tasks> {
			return {};
		}
		template<auto options_v>
		static consteval auto options() {
			return builder_type::template set_value<"options", options_v>();
		}
		static consteval auto options() {
			return builder_type::template get_value_or<"options", cli::default_options>();
		}

		static consteval auto option_storage() {
			/* Automatically add task options to options */

			// Collect the option associated with every task.
			using task_options = meta::value_list<>::unwrap_list<typename decltype(tasks())::template fold<[]<typename state_t, typename task_t>() consteval {
				return std::type_identity<typename state_t::template append<meta::type_value<task_t::option>>>{};
			}, meta::type_list<>>>;

			// Append the contents of task_options, not task_options itself.
			using full_options_list = decltype(options())::template prepend_list<task_options>;

			// IMPORTANT: wrap the list in option_storage
			using option_storage_type = cli::option_storage<full_options_list>;

			return std::type_identity<option_storage_type>{};
		};

		static consteval auto full_options() {
			return typename decltype(option_storage())::type{};
		}
	};
	static constexpr auto config = config_impl<>{}.template tasks<task::default_tasks>().template options<cli::default_options>();
}

#endif // MGMAKE_CONFIG_HXX// ===== end include/mgmake/config.hxx =====


#include <print>
#include <utility>

namespace mgmake::cli {
    template<auto config_v = config>
    inline sys::exit_code entry(sys::shell cmd) {
		using opt_storage_type = decltype(config_v.option_storage())::type;

        // construct the parser & dispatcher at compile time :)
		using d = task::dispatcher<config_v>;
        using p = parser<opt_storage_type>;

        // parse cmd at runtime
        if (auto parse_result = p::template parse<d>(cmd)) {
			auto opts = std::move(parse_result).value();

			if (auto dispatch_result = d::invoke(cmd, opts)) {
                return dispatch_result.value();
            } else {
                std::println(stderr, "{}", dispatch_result.error());
                return sys::exit_code::task_failure;
            }
        } else {
            std::println(stderr, "{}", parse_result.error());
            return sys::exit_code::usage_error;
        }
    }

    template<auto config_v = config>
    inline sys::exit_code entry(int argc, char* argv[]) {
        return entry<config_v>(sys::shell::from_args(argc, argv));
    }
}

// Pass a `cli::config` type with your project
#define MGMK_ENTRY(...) \
int main(int argc, char* argv[]) { \
    return std::to_underlying(::mgmake::cli::entry<__VA_ARGS__>(argc, argv)); \
}

#endif // MGMAKE_CLI_ENTRY_HXX// ===== end include/mgmake/cli/entry.hxx =====


// ===== begin include/mgmake/spec/cmake.hxx =====
#pragma once

#ifndef MGMAKE_SPEC_CMAKE_HXX
#define MGMAKE_SPEC_CMAKE_HXX

// skipped duplicate include: include/mgmake/meta/builder_mixin.hxx
// skipped duplicate include: include/mgmake/meta/type_builder.hxx

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct cmake_target_impl :public meta::type_builder<cmake_target_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<cmake_target_impl, storage_t>;

		template<auto cmake_v>
		static consteval auto project() {
			return builder_type::template set_value<"project", cmake_v>();
		}
		static consteval auto project() {
			return builder_type::template get_value<"project">();
		}
	};

	template<typename storage_t = meta::type_map<>>
	struct cmake_impl : public meta::type_builder<cmake_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<cmake_impl, storage_t>;

		template<auto fetch_v>
		static consteval auto fetch() {
			return builder_type::template set_value<"fetch", fetch_v>();
		}
		static consteval auto fetch() {
			if constexpr (builder_type::template has<"fetch">()) {
				return builder_type::template get_value<"fetch">();
			}
		}

		template<typename cmake_vars_t = meta::type_map<>>
		static consteval auto set_cmake_vars() -> builder_type::template set_type<"cmake_vars", cmake_vars_t> {
			return {};
		}
		template<meta::static_string var_v, meta::static_string val_v>
		static consteval auto define() -> decltype(set_cmake_vars<typename meta::type_or_t<
				typename builder_type::template get_type<"cmake_vars", false>,
				meta::type_map<>
			>
			::template emplace_unique<meta::type_value<var_v>, meta::type_value<val_v>>>()) {
			return {};
		}

		template<bool install_v = true>
		static consteval auto set_install() {
			return builder_type::template set_value<"install", install_v>();
		}
		static consteval auto install() {
			return set_install<true>();
		}

		static consteval auto target() {
			return cmake_target_impl<>{}.project<cmake_impl{}>();
		}
		static consteval auto library() {
			return target();
		}
	};

	static constexpr auto cmake = cmake_impl<>{};
}

#endif // MGMAKE_SPEC_CMAKE_HXX// ===== end include/mgmake/spec/cmake.hxx =====


// ===== begin include/mgmake/spec/fetch.hxx =====
#pragma once

#ifndef MGMAKE_SPEC_FETCH_HXX
#define MGMAKE_SPEC_FETCH_HXX

// skipped duplicate include: include/mgmake/meta/builder_mixin.hxx
// skipped duplicate include: include/mgmake/meta/type_builder.hxx

namespace mgmake::spec {
	enum struct fetch_type {
		git,
		archive,
		local
	};

	enum struct archive_format {
		auto_detect,
		zip,
		tar,
		tar_gz,
		tar_xz
	};

	template<typename storage_t = meta::type_map<>>
	struct git_fetch_impl : public meta::type_builder<git_fetch_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<git_fetch_impl, storage_t>;

		template<meta::static_string url_v>
		static consteval auto url() {
			return builder_type::template set_str<"url", url_v>();
		}
		static consteval auto url() {
			return builder_type::template get_str<"url">();
		}

		template<meta::static_string tag_v>
		static consteval auto tag() {
			return builder_type::template set_str<"tag", tag_v>();
		}
		static consteval auto tag() {
			return builder_type::template get_str<"tag">();
		}
	};

	template<typename storage_t = meta::type_map<>>
	struct archive_fetch_impl : public meta::type_builder<archive_fetch_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<archive_fetch_impl, storage_t>;

		template<meta::static_string url_v>
		static consteval auto url() {
			return builder_type::template set_str<"url", url_v>();
		}
		static consteval auto url() {
			return builder_type::template get_str<"url">();
		}

		template<meta::static_string checksum_v>
		static consteval auto checksum() {
			return builder_type::template set_str<"checksum", checksum_v>();
		}
		static consteval auto checksum() {
			return builder_type::template get_str<"checksum">();
		}

		template<archive_format format_v>
		static consteval auto format() {
			return builder_type::template set_value<"format", format_v>();
		}
		static consteval archive_format format() {
			if constexpr (builder_type::template has<"format">()) {
				return builder_type::template get_value<"format">();
			} else {
				return archive_format::auto_detect;
			}
		}
	};

	template<typename storage_t = meta::type_map<>>
	struct local_fetch_impl : public meta::type_builder<local_fetch_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<local_fetch_impl, storage_t>;

		template<meta::static_string path_v>
		static consteval auto path() {
			return builder_type::template set_str<"path", path_v>();
		}
		static consteval auto path() {
			return builder_type::template get_str<"path">();
		}
	};

	template<typename storage_t = meta::type_map<>>
	struct fetch_impl : public meta::type_builder<fetch_impl, storage_t>, meta::named {
		static consteval auto git() {
			return git_fetch_impl<>{}.name<fetch_impl{}.name()>();
		}
		static consteval auto archive() {
			return archive_fetch_impl<>{}.name<fetch_impl{}.name()>();
		}
		static consteval auto local() {
			return local_fetch_impl<>{}.name<fetch_impl{}.name()>();
		}
	};
	static constexpr auto fetch = fetch_impl<>{};
}

#endif // MGMAKE_SPEC_FETCH_HXX// ===== end include/mgmake/spec/fetch.hxx =====


// ===== begin include/mgmake/spec/project.hxx =====
#pragma once

#ifndef MGMAKE_SPEC_PROJECT_HXX
#define MGMAKE_SPEC_PROJECT_HXX

// skipped duplicate include: include/mgmake/spec/traits.hxx

// skipped duplicate include: include/mgmake/meta/builder_mixin.hxx
// skipped duplicate include: include/mgmake/meta/type_builder.hxx
// skipped duplicate include: include/mgmake/meta/value_list.hxx

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct project_impl : public meta::type_builder<project_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<project_impl, storage_t>;

		// Targets directly given to the project
		template<typename targets_t = meta::value_list<>>
		static consteval auto set_targets() -> builder_type::template set_type<"targets", targets_t> {
			return {};
		}
		template<auto... target_vs>
		static consteval auto targets() {
			using targets_type = builder_type::template get_type_or<"targets", meta::value_list<>>;
			return set_targets<typename targets_type::template append_values_unique<target_vs...>>();
		}
		static consteval auto targets() -> builder_type::template get_type_or<"targets", meta::value_list<>> {
			return {};
		}
		template<auto target_v>
		static consteval auto target() {
			return targets<target_v>();
		}

		// Uses collect_targets to recursively get all targets for the project
		static consteval auto all_targets() -> decltype(targets())::template fold<[]<typename state_t, auto target_v>{
				// If the target can also collect targets
				if constexpr (collects_targets<target_v>) {
					// Collect them
					static constexpr auto children_v = target_v.collect_targets();
					// Append the collected
					return std::type_identity<typename state_t::template append_list_unique<decltype(children_v)>::template append_values_unique<target_v>>{};
				} else {
					// Doesn't collect, just append
					return std::type_identity<typename state_t::template append_values_unique<target_v>>{};
				}
			}, meta::value_list<>> {
			return {};
		}
	};
	static constexpr auto project = project_impl<>{};
}

#endif// ===== end include/mgmake/spec/project.hxx =====


// ===== begin include/mgmake/spec/target.hxx =====
#pragma once

#ifndef MGMAKE_SPEC_TARGET_HXX
#define MGMAKE_SPEC_TARGET_HXX

// skipped duplicate include: include/mgmake/spec/traits.hxx

// skipped duplicate include: include/mgmake/meta/builder_mixin.hxx
// skipped duplicate include: include/mgmake/meta/type_builder.hxx
// skipped duplicate include: include/mgmake/meta/value_list.hxx

namespace mgmake::spec {
	enum struct target_type {
		none,
		executable,
		library
	};

	enum struct library_type {
		none,
		static_lib,
		shared_lib,
		interface
	};

	template<typename storage_t = meta::type_map<>>
	struct target_impl : public meta::type_builder<target_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<target_impl, storage_t>;

		template<target_type type_v>
		static consteval auto set_target_type() {
			return builder_type::template set_value<"target_type", type_v>();
		}
		template<library_type type_v>
		static consteval auto set_library_type() {
			return builder_type::template set_value<"library_type", type_v>();
		}
		template<auto type_v>
		static consteval auto type() {
			using type_t = decltype(type_v);
			static_assert(std::is_same_v<type_t, target_type> or std::is_same_v<type_t, library_type>, "type_v must be a target_type or library_type");

			if constexpr (std::is_same_v<type_t, target_type>) {
				return set_target_type<type_v>();
			}
			if constexpr (std::is_same_v<type_t, library_type>) {
				return set_library_type<type_v>();
			}
		}

		template<typename sources_t = meta::type_list<>>
		static consteval auto set_sources() -> builder_type::template set_type<"sources", sources_t> {
			return {};
		}
		template<meta::static_string... source_vs>
		static consteval auto sources() {
			using sources_type = builder_type::template get_type_or<"sources", meta::type_list<>>;
			return set_sources<typename sources_type::template append_types_unique<meta::type_value<source_vs>...>>();
		}
		template<meta::static_string source_v>
		static consteval auto source() {
			return sources<source_v>();
		}

		template<typename includes_t = meta::type_list<>>
		static consteval auto set_includes() -> builder_type::template set_type<"includes", includes_t> {
			return {};
		}
		template<meta::static_string... include_vs>
		static consteval auto includes() {
			using includes_type = builder_type::template get_type_or<"includes", meta::type_list<>>;
			return set_includes<typename includes_type::template append_types_unique<meta::type_value<include_vs>...>>();
		}
		template<meta::static_string... include_vs>
		static consteval auto include_dirs() {
			return includes<include_vs...>();
		}
		template<meta::static_string include_v>
		static consteval auto include() {
			return includes<include_v>();
		}
		template<meta::static_string include_v>
		static consteval auto include_dir() {
			return include<include_v>();
		}

		template<typename links_t = meta::value_list<>>
		static consteval auto set_links() -> builder_type::template set_type<"links", links_t> {
			return {};
		}
		template<auto... link_vs>
		static consteval auto links() {
			using links_type = builder_type::template get_type_or<"links", meta::value_list<>>;
			return set_links<typename links_type::template append_values_unique<link_vs...>>();
		}
		static consteval auto links() -> builder_type::template get_type_or<"links", meta::value_list<>> {
			return {};
		}
		template<auto link_v>
		static consteval auto link() {
			return links<link_v>();
		}

		// Recursively collect all used targets
		static consteval auto collect_targets() -> decltype(links())::template fold<[]<typename state_t, auto link_v>{
				// If the link can also collect targets
				if constexpr (collects_targets<link_v>) {
					// Collect them
					static constexpr auto children_v = link_v.collect_targets();
					// Append the collected
					return std::type_identity<typename state_t::template append_list_unique<decltype(children_v)>::template append_values_unique<link_v>>{};
				} else {
					// Doesn't collect, just append
					return std::type_identity<typename state_t::template append_values_unique<link_v>>{};
				}
			}, meta::value_list<>> {
			return {};
		}
	};
	static constexpr auto target = target_impl<>{};

	static constexpr auto library = target.type<target_type::library>();

	static constexpr auto static_lib = library.type<library_type::static_lib>();
	static constexpr auto shared_lib = library.type<library_type::shared_lib>();
	static constexpr auto interface_lib = library.type<library_type::interface>();

	static constexpr auto executable = target.type<target_type::executable>();
}

#endif // MGMAKE_SPEC_TARGET_HXX// ===== end include/mgmake/spec/target.hxx =====

// skipped duplicate include: include/mgmake/config.hxx

namespace mgmk = mgmake;

#endif // MGMAKE_MGMAKE_HXX
// ===== end include/mgmake/mgmake.hxx =====


#endif // MGMAKE_SINGLE_HEADER_HXX
