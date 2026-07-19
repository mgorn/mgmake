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
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(project, void);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(toolchains, void);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(tasks, void);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(option_storage, cli::option_storage<>);
	};

	template<typename builder_t = meta::type_builder<>>
    struct config_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(config_builder, project_impl, "project");
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(config_builder, toolchains);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(config_builder, tasks);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(config_builder, options);

		template<typename project_t>
		using project = project_impl<typename std::invoke_result_t<decltype([]{
			if constexpr (meta::is_builder<project_t>) {
				return std::type_identity<typename project_t::build>{};
			} else {
				return std::type_identity<project_t>{};
			}
		})>::type>;

		using build = std::decay_t<std::invoke_result_t<decltype([] consteval {
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

			// Create a new config builder with the task options appended
			// Update the existing builder directly. Creating another config_builder
			// specialization here would recursively instantiate its build alias.
			// assign to option_storage key instead as well
			using actual_builder_type = typename builder_type::template set<"option_storage", options_storage_type>;

			// Use the builder type from that instead
			using result_type = typename actual_builder_type::template build<config_impl>;

			// Now we have a config where options has task options appended
			return result_type{};
		})>>;
	};
	using config = config_builder<>::tasks<task::default_tasks>::options<cli::default_options>;
}

#endif // MGMAKE_CONFIG_HXX