#pragma once

#ifndef MGMAKE_SPEC_PROJECT_HXX
#define MGMAKE_SPEC_PROJECT_HXX

#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct project_impl {

	};

	template<typename builder_t = meta::type_builder<>>
	struct project_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(project_builder, name, meta::static_string);
		// Takes a `meta::type_list` of your target types
		MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(project_builder, set_targets, "targets");

		// Add targets
		template<typename... target_ts>
		using targets = set_targets<typename meta::type_or_t<typename builder_t::template get<"targets", false>, meta::type_list<>>::template append_types_unique<target_ts...>>;
		// Add one target
		template<typename target_t>
		using target = targets<target_t>;

		using build = typename builder_type::template build<project_impl>;
	};
	using project = project_builder<>;
}

#endif