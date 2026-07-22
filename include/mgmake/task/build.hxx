#pragma once

#ifndef MGMAKE_TASK_BUILD_HXX
#define MGMAKE_TASK_BUILD_HXX

#include "../cli/default_options.hxx"
#include "../spec/traits.hxx"
#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct build {
		using option_type = cli::option
			::name<"build">
			::description<"Build the project.">
			::set<"task", std::size_t{0}>
			::task<true>::flag<false>
			::build;
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			// TODO: This would be the entrypoint/root for build
			std::println("Build task");
			std::println("Build dir: {}", opts.template get<cli::build_dir_option>().string());
			std::println("Verbose: {}", opts.template get<cli::verbose_option>());
			std::print("Target(s): ");
			auto& targets = opts.template get<cli::targets_option>();
			for (auto it = targets.begin(); it != targets.end(); ++it) {
				std::print("{}", *it);
				if (std::next(it) != targets.end()) {
					std::print(", ");
				}
			}
			std::println("");

			using project_type = decltype(config_v.project())::type;
			if constexpr (not std::is_same_v<project_type, void>) {
				using targets_type = project_type::all_targets;

				targets_type::for_each([]<typename target_t>{
					if constexpr (spec::collects_targets<target_t>) {
						std::println("PROJ LINKS TARGET: '{}'", target_t::name_value);
					} else {
						std::println("PROJ TARGET: '{}'", target_t::name_value);
					}
				});
			}
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_BUILD_HXX