#pragma once

#ifndef MGMAKE_SPEC_TARGET_HXX
#define MGMAKE_SPEC_TARGET_HXX

#include "traits.hxx"

#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/value_list.hxx"

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
	struct target_impl : public meta::type_builder<target_impl, storage_t>, public meta::named<target_impl<storage_t>> {
		using builder_type = meta::type_builder<target_impl, storage_t>;

		template<target_type type_v>
		[[nodiscard]] static consteval auto set_target_type() {
			return builder_type::template set_value<"target_type", type_v>();
		}
		template<library_type type_v>
		[[nodiscard]] static consteval auto set_library_type() {
			return builder_type::template set_value<"library_type", type_v>();
		}
		template<auto type_v>
		[[nodiscard]] static consteval auto type() {
			using type_t = decltype(type_v);
			static_assert(std::is_same_v<type_t, target_type> or std::is_same_v<type_t, library_type>, "type_v must be a target_type or library_type");

			if constexpr (std::is_same_v<type_t, target_type>) {
				return set_target_type<type_v>();
			} else {
				return set_library_type<type_v>();
			}
		}
		static consteval auto type() {
			if constexpr (builder_type::template has<"library_type">()) {
				return builder_type::template get_value<"library_type">();
			} else {
				return builder_type::template get_value_or<"target_type", target_type::none>();
			}
		}

		template<typename sources_t = meta::value_list<>>
		[[nodiscard]] static consteval auto set_sources() -> builder_type::template set_type<"sources", sources_t> {
			return {};
		}
		template<meta::static_string... source_vs>
		[[nodiscard]] static consteval auto sources() {
			using sources_type = builder_type::template get_type_or<"sources", meta::value_list<>>;
			return set_sources<typename sources_type::template append_values_unique<source_vs...>>();
		}
		static consteval auto sources() -> builder_type::template get_type_or<"sources", meta::value_list<>> {
			return {};
		}
		template<meta::static_string source_v>
		[[nodiscard]] static consteval auto source() {
			return sources<source_v>();
		}

		template<typename includes_t = meta::value_list<>>
		[[nodiscard]] static consteval auto set_includes() -> builder_type::template set_type<"includes", includes_t> {
			return {};
		}
		template<meta::static_string... include_vs>
		[[nodiscard]] static consteval auto includes() {
			using includes_type = builder_type::template get_type_or<"includes", meta::value_list<>>;
			return set_includes<typename includes_type::template append_values_unique<include_vs...>>();
		}
		static consteval auto includes() -> builder_type::template get_type_or<"includes", meta::value_list<>> {
			return {};
		}
		template<meta::static_string... include_vs>
		[[nodiscard]] static consteval auto include_dirs() {
			return includes<include_vs...>();
		}
		template<meta::static_string include_v>
		[[nodiscard]] static consteval auto include() {
			return includes<include_v>();
		}
		template<meta::static_string include_v>
		[[nodiscard]] static consteval auto include_dir() {
			return include<include_v>();
		}

		template<typename links_t = meta::value_list<>>
		[[nodiscard]] static consteval auto set_links() -> builder_type::template set_type<"links", links_t> {
			return {};
		}
		template<auto... link_vs>
		[[nodiscard]] static consteval auto links() {
			using links_type = builder_type::template get_type_or<"links", meta::value_list<>>;
			return set_links<typename links_type::template append_values_unique<link_vs...>>();
		}
		static consteval auto links() -> builder_type::template get_type_or<"links", meta::value_list<>> {
			return {};
		}
		template<auto link_v>
		[[nodiscard]] static consteval auto link() {
			return links<link_v>();
		}

		// Recursively collect all used targets.
		template<typename visited_t = meta::value_list<>> requires value_list_type<visited_t>
		static consteval auto collect_targets() {
			static_assert(not visited_t::template has<target_impl<storage_t>{}>(), "target dependency cycle detected while collecting linked targets");
			using path_t = typename visited_t::template append<target_impl<storage_t>{}>;

			using collected_t = typename decltype(links())::template fold<[]<typename state_t, auto link_v>() consteval {
				// Prefer the cycle-aware collector when it is available.
				if constexpr (collects_targets_with<link_v, path_t>) {
					using children_t = decltype(link_v.template collect_targets<path_t>());
					return std::type_identity<typename state_t::template append_list_unique<children_t>::template append_values_unique<link_v>>{};
				} else if constexpr (collects_targets<link_v>) {
					using children_t = decltype(link_v.collect_targets());
					return std::type_identity<typename state_t::template append_list_unique<children_t>::template append_values_unique<link_v>>{};
				} else {
					return std::type_identity<typename state_t::template append_values_unique<link_v>>{};
				}
			}, meta::value_list<>>;
			return collected_t{};
		}
	};
	static constexpr auto target = target_impl<>{};

	static constexpr auto library = target.type<target_type::library>();

	static constexpr auto static_lib = library.type<library_type::static_lib>();
	static constexpr auto shared_lib = library.type<library_type::shared_lib>();
	static constexpr auto interface_lib = library.type<library_type::interface>();

	static constexpr auto executable = target.type<target_type::executable>();
}

#endif // MGMAKE_SPEC_TARGET_HXX
