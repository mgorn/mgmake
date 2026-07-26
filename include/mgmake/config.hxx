#pragma once

#ifndef MGMAKE_CONFIG_HXX
#define MGMAKE_CONFIG_HXX

#include "cli/default_options.hxx"
#include "cli/options.hxx"
#include "find/default_toolchains.hxx"
#include "find/default_tools.hxx"
#include "find/tools.hxx"
#include "meta/type_builder.hxx"
#include "meta/value_list.hxx"
#include "task/default_tasks.hxx"

#include <type_traits>

namespace mgmake {
	template<typename storage_t = meta::type_map<>>
    struct config_impl : public meta::type_builder<config_impl, storage_t> {
		using builder_type = meta::type_builder<config_impl, storage_t>;

		template<auto project_v>
		[[nodiscard]] static consteval auto project() {
			return builder_type::template set_value<"project", project_v>();
		}
		static consteval auto project() {
			return builder_type::template get_value_or<"project", nullptr>();
		}

		template<typename tools_t>
		[[nodiscard]] static consteval auto tools_list() -> builder_type::template set_type<"tools_list", tools_t> {
			return {};
		}
		static consteval auto tools_list() -> builder_type::template get_type_or<"tools_list", find::default_tools> {
			return {};
		}
		template<auto tool_v>
		[[nodiscard]] static consteval auto add_tool() {
			return tools_list<typename decltype(tools_list())::template append<tool_v>>();
		}
		template<typename toolchains_t>
		[[nodiscard]] static consteval auto toolchains_list() -> builder_type::template set_type<"toolchains_list", toolchains_t> {
			return {};
		}
		static consteval auto toolchains_list() -> builder_type::template get_type_or<"toolchains_list", find::default_toolchains> {
			return {};
		}
		template<auto toolchain_v>
		[[nodiscard]] static consteval auto add_toolchain() {
			return toolchains_list<typename decltype(toolchains_list())::template append<toolchain_v>>();
		}

		template<typename tasks_t>
		[[nodiscard]] static consteval auto tasks_list() -> builder_type::template set_type<"tasks_list", tasks_t> {
			return {};
		}
		static consteval auto tasks_list() -> builder_type::template get_type_or<"tasks_list", task::default_tasks> {
			return {};
		}
		template<typename task_t>
		[[nodiscard]] static consteval auto add_task() {
			return tasks_list<typename decltype(tasks_list())::template append<task_t>>();
		}

		template<typename options_t>
		[[nodiscard]] static consteval auto options_list() -> builder_type::template set_type<"options_list", options_t> {
			return {};
		}
		static consteval auto options_list() -> builder_type::template get_type_or<"options_list", cli::default_options> {
			return {};
		}
		template<auto option_v>
		[[nodiscard]] static consteval auto add_option() {
			return options_list<typename decltype(options_list())::template append<option_v>>();
		}

		static consteval auto full_options_list() {
			// Collect the option associated with every task.
			using task_options = meta::value_list<>::unwrap_list<typename decltype(tasks_list())::template fold<[]<typename state_t, typename task_t>() consteval {
				return std::type_identity<typename state_t::template append<meta::type_value<task_t::option>>>{};
			}, meta::type_list<>>>;

			// Collect the option for each tool override
			using tool_options = decltype(tools_list())::template fold<[]<typename state_t, auto tool_v> consteval {
				return std::type_identity<typename state_t::template append<tool_v.option()>>{};
			}, meta::value_list<>>;

			// Append the contents of task_options, not task_options itself.
			using full_options_list = decltype(options_list())::template prepend_list<task_options>::template append_list<tool_options>;
			return full_options_list{};
		}

		static consteval auto options() {
			return cli::options_impl<decltype(full_options_list())>{};
		}
		static consteval auto tools() {
			return find::tools_impl<decltype(tools_list()), decltype(toolchains_list())>{};
		}
	};
	static constexpr auto config = config_impl<>{};
}

#endif // MGMAKE_CONFIG_HXX
