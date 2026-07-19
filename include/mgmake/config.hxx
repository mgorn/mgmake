#pragma once

#ifndef MGMAKE_CONFIG_HXX
#define MGMAKE_CONFIG_HXX

#include "cli/option_storage.hxx"
#include "cli/default_options.hxx"
#include "meta/type_builder.hxx"
#include "meta/type_or.hxx"
#include "task/default_tasks.hxx"

namespace mgmake {
	template<typename storage_t = meta::type_map<>>
    struct config_impl {
		using builder_type = meta::type_builder<storage_t>;

		template<auto builder_v, meta::static_string key_v>
		consteval auto set_builder() const {
			if constexpr (meta::has_builder_fn<builder_v>) {
				return config_impl<builder_type::set<key_v, builder_v.build()>>{};
			} else {
				return config_impl<builder_type::set<key_v, builder_v>>{};
			}
		}
		template<auto project_v>
		consteval auto project() const {
			return set_builder<project_v, "project">();
		}
		template<auto toolchains_v>
		consteval auto toolchains() const {
			return set_builder<toolchains_v, "toolchains">();
		}
		template<auto tasks_v>
		consteval auto tasks() const {
			return set_builder<tasks_v, "tasks">();
		}
		template<auto options_v>
		consteval auto options() const {
			return set_builder<options_v, "options">();
		}

		consteval auto option_storage() const {
			/* Automatically add task options to options */
			// Get the tasks
			using tasks_type = meta::type_or_t<typename builder_type::template get<"tasks", false>, task::default_tasks>;
			// Get the options
			using options_type = meta::type_or_t<typename builder_type::template get<"options", false>, cli::default_options>;

			// Collect the option associated with every task.
			using task_options = typename tasks_type::template fold<[]<typename state_t, typename task_t>() consteval {
				return std::type_identity<typename state_t::template append<typename task_t::option_type>>{};
			}, meta::type_list<>>;

			// Append the contents of task_options, not task_options itself.
			using full_options_list = typename options_type::template prepend_list<task_options>;

			// IMPORTANT: wrap the list in option_storage
			using options_storage_type = cli::option_storage<full_options_list>;

			return std::type_identity<option_storage_type>{};
		};

		consteval auto options() const {
			return decltype(option_storage())::type{};
		}
	};
	static constexpr auto config = config_impl<>{}.tasks<task::default_tasks>().options<cli::default_options>();
}

#endif // MGMAKE_CONFIG_HXX