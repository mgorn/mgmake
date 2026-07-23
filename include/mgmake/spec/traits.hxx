#pragma once

#ifndef MGMAKE_SPEC_TRAITS_HXX
#define MGMAKE_SPEC_TRAITS_HXX

#include "../meta/value_list.hxx"

#include <type_traits>

namespace mgmake::spec {
	template<typename spec_t>
	concept has_links = requires {
		typename spec_t::links;
	};

	template<typename type_t>
	struct is_value_list : std::false_type {};

	template<auto... value_vs>
	struct is_value_list<meta::value_list<value_vs...>> : std::true_type {};

	template<typename type_t>
	concept value_list_type = is_value_list<std::remove_cvref_t<type_t>>::value;

	template<auto spec_v, typename visited_t>
	concept collects_targets_with = value_list_type<visited_t> and requires {
		requires value_list_type<decltype(spec_v.template collect_targets<visited_t>())>;
	};

	template<auto spec_v>
	concept collects_targets = requires {
		requires value_list_type<decltype(spec_v.collect_targets())>;
	};
}

#endif // MGMAKE_SPEC_TRAITS_HXX
