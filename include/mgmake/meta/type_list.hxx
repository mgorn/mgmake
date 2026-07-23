#pragma once

#ifndef MGMAKE_META_TYPE_LIST_HXX
#define MGMAKE_META_TYPE_LIST_HXX

#include "../detail/assert.hxx"

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
	struct type_list;

	namespace type_list_detail {
		template<typename current_list_t, typename... other_type_ts>
		struct append_types_unique_type {
			using type = current_list_t;
		};

		template<typename current_list_t, typename first_t, typename... rest_t>
		struct append_types_unique_type<current_list_t, first_t, rest_t...> {
			using next_list_t = std::conditional_t<current_list_t::template has<first_t>(), current_list_t, typename current_list_t::template append<first_t>>;
			using type = typename append_types_unique_type<next_list_t, rest_t...>::type;
		};

		template<typename current_list_t, typename prefix_t, typename... other_type_ts>
		struct prepend_types_unique_type;

		template<typename current_list_t, typename prefix_t>
		struct prepend_types_unique_type<current_list_t, prefix_t> {
			using type = typename prefix_t::template append_list<current_list_t>;
		};

		template<typename current_list_t, typename prefix_t, typename first_t, typename... rest_t>
		struct prepend_types_unique_type<current_list_t, prefix_t, first_t, rest_t...> {
			using next_prefix_t = std::conditional_t<current_list_t::template has<first_t>() or prefix_t::template has<first_t>(), prefix_t, typename prefix_t::template append<first_t>>;
			using type = typename prepend_types_unique_type<current_list_t, next_prefix_t, rest_t...>::type;
		};

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
				typename insert_sorted_type<compare, type_list<rest_t...>, type_t>::type::template prepend<first_t>
			>;
		};
	}

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

		template<typename... other_type_ts>
		using append_types_unique = typename type_list_detail::append_types_unique_type<type_list<type_ts...>, other_type_ts...>::type;

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
		using prepend_types_unique = typename type_list_detail::prepend_types_unique_type<type_list<type_ts...>, type_list<>, other_type_ts...>::type;

		template<typename type_t, bool check = true>
		struct prepend_unique_type {
			static_assert((not check) or (not has<type_t>()), "type_list::prepend_unique cannot prepend a duplicate type.");

			using type = std::conditional_t<has<type_t>(), type_list<type_ts...>, prepend<type_t>>;
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

		// Fold the stored types from left to right into an accumulated type.
		//
		// The operation should be callable as:
		//     `operation.template operator()<state_t, type_t>()`
		// and return `std::type_identity<next_state_t>`.
		template<auto operation, typename initial_t>
		using fold = typename type_list_detail::fold_type<operation, initial_t, type_ts...>::type;

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

		// Sort this `type_list` using a consteval NTTP comparator.
		//
		// The comparator should be callable as:
		//     `compare.template operator()<left_t, right_t>()`
		// and return true when `left_t` should appear before `right_t`.
		template<auto compare>
		using sort = fold<
			[]<typename sorted_t, typename type_t>() consteval {
				return std::type_identity<typename type_list_detail::insert_sorted_type<compare, sorted_t, type_t>::type>{};
			},
			type_list<>
		>;

		template<typename callable_t>
		static constexpr void for_each(callable_t& callable) {
			(callable.template operator()<type_ts>(), ...);
		}

		template<typename callable_t> requires (not std::is_lvalue_reference_v<callable_t>)
		static constexpr void for_each(callable_t&& callable) {
			for_each(callable);
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

#endif // MGMAKE_META_TYPE_LIST_HXX
