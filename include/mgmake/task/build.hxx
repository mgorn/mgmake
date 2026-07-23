#pragma once

#ifndef MGMAKE_TASK_BUILD_HXX
#define MGMAKE_TASK_BUILD_HXX

#include "../cli/default_options.hxx"
#include "../spec/traits.hxx"
#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct build {
		static constexpr auto option = cli::option
			.name<"build">()
			.description<"Build the project.">()
			.set<"task", std::size_t{0}>()
			.task<true>().flag<false>();
		
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

			constexpr auto project = config_v.project();
			if constexpr (not std::is_same_v<decltype(project), std::nullptr_t>) {
				constexpr auto targets = project.all_targets();

				targets.for_each([]<auto target_v>{
					if constexpr (spec::collects_targets<target_v>) {
						std::println("PROJ LINKS TARGET: '{}'", target_v.name());
					} else {
						std::println("PROJ TARGET: '{}'", target_v.name());
					}
				});
			}
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_BUILD_HXX