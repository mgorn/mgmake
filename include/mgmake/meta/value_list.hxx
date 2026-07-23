#pragma once

#ifndef MGMAKE_META_VALUE_LIST_HXX
#define MGMAKE_META_VALUE_LIST_HXX

#include "type_list.hxx"
#include "type_value.hxx"

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

#endif // MGMAKE_META_VALUE_LIST_HXX