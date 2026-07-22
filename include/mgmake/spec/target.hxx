#pragma once

#ifndef MGMAKE_SPEC_TARGET_HXX
#define MGMAKE_SPEC_TARGET_HXX

#include "traits.hxx"

#include "../meta/type_builder.hxx"

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
	struct target_impl {
		using storage_type = storage_t;

		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(name, meta::static_string{ "" });
		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(target_type, target_type::none);
		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(library_type, library_type::none);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(sources, meta::type_list<>);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(include_dirs, meta::type_list<>);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(links, meta::type_list<>);

		// Recursively collect all used targets
		using collect_targets = typename links::template fold<[]<typename state_t, typename link_t>{
			// If the link can also collect targets
			if constexpr (collects_targets<link_t>) {
				// Collect them
				using children_t = link_t::collect_targets;
				// Append the collected
				return std::type_identity<typename state_t::template append_list_unique<children_t>::template append_types_unique<link_t>>{};
			} else {
				// Doesn't collect, just append
				return std::type_identity<typename state_t::template append_types_unique<link_t>>{};
			}
		}, meta::type_list<>>;
	};

	template<typename builder_t = meta::type_builder<>>
	struct target_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(target_builder, name, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD_AS(target_builder, unchecked_set_target_type, target_type, "target_type");
		MGMAKE_TYPE_BUILDER_VALUE_FIELD_AS(target_builder, unchecked_set_library_type, library_type, "library_type");
		// Takes a `meta::type_list` of `meta::type_value`s of `meta::static_string`s for sources
		MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(target_builder, set_sources, "sources");
		// Takes a `meta::type_list` of `meta::type_value`s of `meta::static_string`s for include dirs
		MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(target_builder, set_include_dirs, "include_dirs");
		// Takes a `meta::type_list` of link dependencies
		MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(target_builder, set_links_impl, "links");

		template<target_type type_v>
		using set_target_type = std::remove_cvref_t<std::invoke_result_t<decltype([] consteval {
			static constexpr target_type old_type = meta::type_or_t<typename builder_type::template get<"target_type", false>, meta::type_value<target_type::none>>::value;
			//static_assert(old_type == target_type::none or ((old_type == target_type::executable) != (type_v == target_type::library)), "target type changed from executable to library");
			//static_assert(old_type == target_type::none or ((old_type == target_type::library) != (type_v == target_type::executable)), "target type changed from library to executable");

			using changed_type = unchecked_set_target_type<type_v>;
			return std::type_identity<changed_type>{};
		})>>::type;
		template<library_type type_v>
		using set_library_type = std::remove_cvref_t<decltype([] consteval {
			static constexpr target_type target_type_v = meta::type_or_t<typename builder_type::template get<"target_type", false>, meta::type_value<target_type::none>>::value;
			//static_assert((target_type_v == target_type::none) or (target_type_v == target_type::library), "Cannot change library type of non-library target");
			if constexpr (target_type_v == target_type::none) {
				return std::type_identity<typename set_target_type<target_type::library>::template set_library_type<type_v>>{};
			} else {
				using changed_type = unchecked_set_library_type<type_v>;
				return std::type_identity<changed_type>{};
			}
		}())>::type;
		template<auto type_v>
		using type = std::remove_cvref_t<std::invoke_result_t<decltype([] consteval {
			using type_t = decltype(type_v);
			static_assert(std::is_same_v<type_t, target_type> or std::is_same_v<type_t, library_type>, "type_v must be a target_type or library_type");

			if constexpr (std::is_same_v<type_t, target_type>) {
				return std::type_identity<target_builder::template set_target_type<type_v>>{};
			}
			if constexpr (std::is_same_v<type_t, library_type>) {
				return std::type_identity<target_builder::template set_library_type<type_v>>{};
			}
		})>>::type;

		// Add multiple sources
		template<meta::static_string... source_vs>
		using sources = set_sources<typename meta::type_or_t<typename builder_t::template get<"sources", false>, meta::type_list<>>::template append_types_unique<meta::type_value<source_vs>...>>;
		// Add one source
		template<meta::static_string source_v>
		using source = sources<source_v>;

		// Add multiple include dirs
		template<meta::static_string... include_vs>
		using include_dirs = set_include_dirs<typename meta::type_or_t<typename builder_t::template get<"include_dirs", false>, meta::type_list<>>::template append_types_unique<meta::type_value<include_vs>...>>;
		// Add one include dir
		template<meta::static_string include_v>
		using include_dir = include_dirs<include_v>;

		// Check if any links are builders and build them
		template<typename links_t = meta::type_list<>>
		using set_links = set_links_impl<typename links_t::template fold<[]<typename list_t, typename link_t> consteval {
			if constexpr (meta::has_builder_alias<link_t>) {
				return std::type_identity<typename list_t::template append_unique<typename link_t::build>>{};
			} else {
				return std::type_identity<typename list_t::template append_unique<link_t>>{};
			}
		}, meta::type_list<>>>;
		// Add multiple links
		template<typename... link_ts>
		using links = set_links<typename meta::type_or_t<typename builder_t::template get<"links", false>, meta::type_list<>>::template append_types_unique<link_ts...>>;
		// Add one source
		template<typename link_t>
		using link = links<link_t>;

		using build = typename builder_type::template build<target_impl>;
	};
	using target = target_builder<>;

	using library = target::type<target_type::library>;

	using static_lib = library::type<library_type::static_lib>;
	using shared_lib = library::type<library_type::shared_lib>;
	using interface_lib = library::type<library_type::interface>;

	using executable = target::type<target_type::executable>;
}

#endif // MGMAKE_SPEC_TARGET_HXX