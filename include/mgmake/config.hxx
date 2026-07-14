#pragma once

#ifndef MGMAKE_CONFIG_HXX
#define MGMAKE_CONFIG_HXX

#include "cli/default_options.hxx"
#include "meta/type_builder.hxx"
#include "task/default_tasks.hxx"

namespace mgmake::cli {
	template<typename storage_t = meta::type_map<>>
	struct config_impl {
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(project, nullptr);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(toolchains, nullptr);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(tasks, void);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(options, void);
	};

	template<typename builder_t = meta::type_builder<>>
    struct config_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(config_builder, project);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(config_builder, toolchains);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(config_builder, tasks);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(config_builder, options);

		using build = std::decay_t<std::invoke_result_t<[] {
			/* Automatically add task options to options */
			// Get the tasks
			using tasks_type = builder_type::get<"tasks">;
			// Get the options
			using options_type = builder_type::get<"options">;

			// Get the list of task options
			using task_options = tasks_type::template fold<[]<typename state_t, typename task_t>() consteval {
				return std::type_identity<typename state_t::template append<typename task_t::option_type>>{};
			}, meta::type_list<>>;

			// Create a new config builder with the task options appended
			using actual_config_builder = config_builder::options<options_type::append<task_options>>;

			// Use the builder type from that instead
			using result_type = typename actual_config_builder::builder_type::template build<config_impl>;

			// Now we have a builder where options has options from tasks appended
			return result_type{};
		}>>;
	}
	using config = config_builder<>::tasks<default_tasks>::options<default_options>;
}

#endif // MGMAKE_CONFIG_HXX