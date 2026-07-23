#pragma once

#ifndef MGMAKE_SPEC_PROJECT_HXX
#define MGMAKE_SPEC_PROJECT_HXX

#include "traits.hxx"

#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/value_list.hxx"

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct project_impl : public meta::type_builder<project_impl, storage_t>, public meta::named<project_impl<storage_t>> {
		using builder_type = meta::type_builder<project_impl, storage_t>;

		// Targets directly given to the project
		template<typename targets_t = meta::value_list<>>
		[[nodiscard]] static consteval auto set_targets() -> builder_type::template set_type<"targets", targets_t> {
			return {};
		}
		template<auto... target_vs>
		[[nodiscard]] static consteval auto targets() {
			using targets_type = builder_type::template get_type_or<"targets", meta::value_list<>>;
			return set_targets<typename targets_type::template append_values_unique<target_vs...>>();
		}
		static consteval auto targets() -> builder_type::template get_type_or<"targets", meta::value_list<>> {
			return {};
		}
		template<auto target_v>
		[[nodiscard]] static consteval auto target() {
			return targets<target_v>();
		}

		// Uses collect_targets to recursively get all targets for the project
		static consteval auto all_targets() {
			using collected_t = typename decltype(targets())::template fold<[]<typename state_t, auto target_v>() consteval {
				// Prefer the cycle-aware collector when it is available.
				if constexpr (collects_targets_with<target_v, meta::value_list<>>) {
					using children_t = decltype(target_v.template collect_targets<meta::value_list<>>());
					return std::type_identity<typename state_t::template append_list_unique<children_t>::template append_values_unique<target_v>>{};
				} else if constexpr (collects_targets<target_v>) {
					using children_t = decltype(target_v.collect_targets());
					return std::type_identity<typename state_t::template append_list_unique<children_t>::template append_values_unique<target_v>>{};
				} else {
					return std::type_identity<typename state_t::template append_values_unique<target_v>>{};
				}
			}, meta::value_list<>>;
			return collected_t{};
		}
	};
	static constexpr auto project = project_impl<>{};
}

#endif