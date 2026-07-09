#pragma once

#include "extract_type.hxx"

namespace mgmake::detail {
    template<auto member_ptr>
    struct member_traits {
        static constexpr auto valid = true;

		template<auto new_member_ptr>
		using var = member_traits<new_member_ptr>;

		using binding_type = decltype(member_ptr);
		using extracted_type = extract_type<binding_type>;
		using class_type = extracted_type::class_type;
		using value_type = extracted_type::member_type;

		static_assert(extracted_type::is_member_variable, "Member pointer binding with incorrect binding type...");
		static constexpr value_type type::* bound_member = member_ptr;

		static value_type& get(type& obj) {
			return obj.*bound_member;
		}
		static void set(type& obj, auto value) {
			obj.*bound_member = static_cast<value_type>(value);
		}
    };

    template<>
	struct member_traits<nullptr> {
		static constexpr auto valid = false;

		using value_type = void;

		template<auto new_member_ptr>
		using var = member_traits<new_member_ptr>;
	};

	using member = member_traits<nullptr>;
}