#pragma once

#ifndef MGMAKE_DISCOVERY_PRINT_TOOLS_HXX
#define MGMAKE_DISCOVERY_PRINT_TOOLS_HXX

#include "resolve.hxx"

#include <expected>
#include <print>
#include <string>

namespace mgmake::discovery {
	inline void print_resolved_tool(const resolved_tool& tool) {
		std::println("  {}", name(tool.m_role));
		std::println("    logical: {}", tool.m_logical_name);
		std::println("    path: {}", tool.m_path.string());
		std::println("    provider: {}", name(tool.m_provider));

		if (tool.m_family != tool_family::unknown) {
			std::println("    family: {}", discovery::name(tool.m_family));
		}

		if (!tool.m_version.empty()) {
			std::println("    version: {}", tool.m_version.substr(0, tool.m_version.find('\n')));
		}
	}

	[[nodiscard]] inline std::expected<void, std::string> print_tools(
		const cli::options& opts,
		const build::request& req,
		const spec::project& project
	) {
		auto resolved_tc = resolve_toolchain(opts, req, project);

		if (!resolved_tc) {
			return std::unexpected{resolved_tc.error()};
		}

		build::request resolved_req = req;
		apply_resolved_toolchain(resolved_req, *resolved_tc);

		std::println("mgmake tools\n");
		std::println("request:");
		std::println("  host: {}", target_key(resolved_tc->m_host));
		std::println("  target: {}", target_key(resolved_tc->m_target));
		std::println("  toolchain: {}", resolved_tc->m_name);
		std::println("  backend: {}", cli::backend_name(opts.m_backend));
		std::println("  discovery mode: {}\n", discovery::name(resolved_tc->m_mode));

		std::println("target tools:");
		for (const auto& tool : resolved_tc->m_tools) {
			print_resolved_tool(tool);
		}

		std::println("\nbackend tools:");

		if (opts.m_backend == cli::backend_kind::automatic || opts.m_backend == cli::backend_kind::ninja) {
			auto ninja = resolve_backend_tool(
				opts,
				resolved_req,
				backend_tool_requirement{
					.m_role = tool_role::generator_ninja,
					.m_logical_name = opts.m_ninja.empty() ? "ninja" : opts.m_ninja,
					.m_needed_because = "the selected backend is ninja"
				}
			);

			if (!ninja) return std::unexpected{ninja.error()};
			print_resolved_tool(*ninja);
		}

		if (opts.m_show_tool_search) {
			std::println("\nsearch trace:");

			for (const auto& item : resolved_tc->m_searched) {
				std::println(
					"  [{}] {}: {} ({})",
					item.m_status,
					name(item.m_candidate.m_role),
					item.m_candidate.m_path.string(),
					name(item.m_candidate.m_provider)
				);
			}

			if (!resolved_tc->m_rejected.empty()) {
				std::println("\nrejected:");

				for (const auto& rejected : resolved_tc->m_rejected) {
					std::println(
						"  {}: {}",
						rejected.m_candidate.m_path.string(),
						rejected.m_reason
					);
				}
			}
		}

		std::println("\nenvironment:");
		if (resolved_tc->m_environment.empty()) {
			std::println("  none");
		} else if (resolved_tc->m_environment.m_setup_script.has_value()) {
			std::println("  setup script: {}", resolved_tc->m_environment.m_setup_script->string());
		}

		std::println("\ncache:");
		std::println("  {}", cache_path(resolved_req).string());
		return {};
	}
}

#endif
