#pragma once

#ifndef MGMAKE_CLI_CONFIG_HXX
#define MGMAKE_CLI_CONFIG_HXX

#include "default_actions.hxx"
#include "default_options.hxx"

namespace mgmake::cli {
	template<auto project_v = nullptr, auto toolchains_v = nullptr, typename actions_t = default_actions, typename options_t = default_options>
	struct config_impl {
		static inline constexpr auto project_value = project_v;
		static inline constexpr auto toolchains_value = toolchains_v;
		using actions_type = actions_t;
		using options_type = options_t;

		template<auto new_project_v>
		using project = config_impl<new_project_v, toolchains_value, actions_type, options_type>;

		template<auto new_toolchains_v>
		using toolchains = config_impl<project_value, new_toolchains_v, actions_type, options_type>;

		template<typename new_actions_t>
		using actions = config_impl<project_value, toolchains_value, new_actions_t, options_type>;

		template<typename new_options_t>
		using options = config_impl<project_value, toolchains_value, actions_type, new_options_t>;
	};
	using config = config_impl<>;
}

#endif // MGMAKE_CLI_CONFIG_HXX