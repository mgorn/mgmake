#pragma once

#ifndef MGMAKE_CLI_DEFAULT_ACTIONS_HXX
#define MGMAKE_CLI_DEFAULT_ACTIONS_HXX

#include "action.hxx"

#include "../meta/type_list.hxx"
#include "../sys/exit_code.hxx"

#include <sstream>

namespace mgmake::cli {
	using help_action = action
		::option<option
			::name<"help">::short_name<'h'>
			::description<"Show help.">
			::action<true>
			::callback<[](auto& opts) {
				// This overrides the action with the help action
				opts.m_action = 0;
			}>
			::build>
		::handler<[]<typename config_t>(auto& cmd, auto& opts) -> std::expected<sys::exit_code, std::string> {
			using config_type = config_t;

			std::println("Usage:");
			std::println("\t{} [action] [options]", cmd.program_name());
			using actions_type = config_type::actions_type;
			using options_type = config_type::options_type;
			
			std::println("\nActions:");
			static constexpr auto action_help = []<typename act_t>(auto& cmd){
				std::println("\t{:<10} {}", act_t::name().view(), act_t::description().view());
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>, auto& cmd) {
				(action_help.template operator()<typename actions_type::template type_at<Is>>(cmd), ...);
			}(std::make_index_sequence<actions_type::size()>{}, cmd);

			std::println("\nOptions:");
			static constexpr auto option_help = []<typename opt_t>{
				// Only print switches, actions will be shown first
				if constexpr (opt_t::flag_value) {
					std::stringstream ss;
					if constexpr (opt_t::short_name_value != '\0') {
						std::print(ss, "-{}, ", opt_t::short_name_value);
					}
					std::print(ss, "--{}", opt_t::name_value.view());
					if constexpr (opt_t::is_assign) {
						using vp = value_parser<typename opt_t::assign_type::value_type>;
						std::print(ss, "=<{}>", vp::help_hint);
					}
					std::println("\t{:<24} {}", ss.str(), opt_t::description_value.view());
				}
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>) {
				(option_help.template operator()<typename options_type::template type_at<Is>>(), ...);
			}(std::make_index_sequence<options_type::size()>{});

			return sys::exit_code::success;
		}>
		::build;
	
	using build_action = action
		::option<option
			::name<"build">
			::description<"Build the project.">
			::action<true>::flag<false>
			::build>
		::handler<[]<typename config_t>(auto& cmd, auto& opts) -> std::expected<sys::exit_code, std::string> {
			// TODO: This would be the entrypoint/root for build
			std::println("Build action");
			return sys::exit_code::success;
		}>
		::build;
	
	using default_actions = meta::type_list<
		help_action,
		build_action
	>;
}

#endif // MGMAKE_CLI_DEFAULT_ACTIONS_HXX