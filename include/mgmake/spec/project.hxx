#pragma once

#ifndef MGMAKE_SPEC_PROJECT_HXX
#define MGMAKE_SPEC_PROJECT_HXX

#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct project_impl {
		using storage_type = storage_t;

		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(name, meta::static_string{ "" });
		// Targets directly given to the project
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(targets, meta::type_list<>);
	};

	template<typename builder_t = meta::type_builder<>>
	struct project_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(project_builder, name, meta::static_string);
		// Takes a `meta::type_list` of your target types
		MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(project_builder, set_targets_impl, "targets");

		// Check if any targets are builders and build them
		template<typename targets_t = meta::type_list<>>
		using set_targets = set_targets_impl<typename targets_t::template fold<[]<typename list_t, typename target_t> consteval {
			if constexpr (meta::is_builder<target_t>) {
				return std::type_identity<typename list_t::template append_unique<typename target_t::build>>{};
			} else {
				return std::type_identity<typename list_t::template append_unique<target_t>>{};
			}
		}, meta::type_list<>>>;

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