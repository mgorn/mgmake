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
    struct config_impl : public meta::type_builder<config_impl, storage_t> {
		using builder_type = meta::type_builder<config_impl, storage_t>;

		template<auto project_v>
		static consteval auto project() {
			return builder_type::template set_value<"project", project_v>();
		}
		static consteval auto project() {
			return builder_type::template get_value_or<"project", nullptr>();
		}
		template<auto toolchains_v>
		static consteval auto toolchains() {
			return builder_type::template set_value<"toolchains", toolchains_v>();
		}
		template<typename tasks_t>
		static consteval auto tasks() -> builder_type::template set_type<"tasks", tasks_t> {
			return {};
		}
		static consteval auto tasks() -> builder_type::template get_type_or<"tasks", task::default_tasks> {
			return {};
		}
		template<auto options_v>
		static consteval auto options() {
			return builder_type::template set_value<"options", options_v>();
		}
		static consteval auto options() {
			return builder_type::template get_value_or<"options", cli::default_options>();
		}

		static consteval auto option_storage() {
			/* Automatically add task options to options */

			// Collect the option associated with every task.
			using task_options = meta::value_list<>::unwrap_list<typename decltype(tasks())::template fold<[]<typename state_t, typename task_t>() consteval {
				return std::type_identity<typename state_t::template append<meta::type_value<task_t::option>>>{};
			}, meta::type_list<>>>;

			// Append the contents of task_options, not task_options itself.
			using full_options_list = decltype(options())::template prepend_list<task_options>;

			// IMPORTANT: wrap the list in option_storage
			using option_storage_type = cli::option_storage<full_options_list>;

			return std::type_identity<option_storage_type>{};
		};

		static consteval auto full_options() {
			return typename decltype(option_storage())::type{};
		}
	};
	static constexpr auto config = config_impl<>{}.template tasks<task::default_tasks>().template options<cli::default_options>();
}

#endif // MGMAKE_CONFIG_HXX