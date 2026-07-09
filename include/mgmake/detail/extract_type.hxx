#pragma once

#include "type_list.hxx"

// Utilities for peeling a class type out of a member pointer or member-function pointer.
// These are useful in bind/member-traits code where the original declaration syntax is noisy.

namespace mgmake::detail {
	template<typename class_t = std::false_type>
	struct extract_type {
		using class_type = class_t;

		static constexpr auto is_member_function = false;
		static constexpr auto is_member_variable = false;
	};

	template<typename return_t, typename class_t, typename... args_t>
	struct extract_type<return_t (class_t::*)(args_t...)> {
		using class_type = class_t;
		using return_type = return_t;
		using arg_types = type_list<args_t...>;

		static constexpr auto is_member_function = true;
		static constexpr auto is_member_variable = false;
	};

	template<typename return_t, typename class_t, typename... args_t>
	struct extract_type<return_t (class_t::*)(args_t...)const> {
		using class_type = class_t;
		using return_type = return_t;
		using arg_types = type_list<args_t...>;

		static constexpr auto is_member_function = true;
		static constexpr auto is_member_variable = false;
	};

	template<typename class_t, typename member_t>
	struct extract_type<member_t class_t::*> {
		using class_type = class_t;
		using member_type = member_t;

		static constexpr auto is_member_function = false;
		static constexpr auto is_member_variable = true;
	};
}