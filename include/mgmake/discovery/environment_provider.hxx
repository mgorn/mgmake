#pragma once

#ifndef MGMAKE_DISCOVERY_ENVIRONMENT_PROVIDER_HXX
#define MGMAKE_DISCOVERY_ENVIRONMENT_PROVIDER_HXX

#include "../build/request.hxx"
#include "../cli/options.hxx"
#include "../spec/project.hxx"
#include "environment.hxx"
#include "tool_environment.hxx"
#include "windows/visual_studio.hxx"

namespace mgmake::discovery {
	[[nodiscard]] inline tool_environment discover_tool_environment(
		const cli::options&,
		const build::request& req,
		const spec::project&
	) {
		tool_environment env;

#if defined(_WIN32)
		if (req.toolchain().dialect() == build::toolchain::dialect::msvc) {
			if (getenv_string("VCINSTALLDIR") && getenv_string("WindowsSdkDir")) {
				return env;
			}

			if (auto root = windows::visual_studio_root_from_tools(req)) {
				windows::visual_studio_instance instance{};
				instance.m_root = *root;

				auto candidate = windows::visual_studio_environment(instance, req);
				if (!candidate.empty()) {
					return candidate;
				}
			}

			for (const auto& vs : windows::visual_studio_instances()) {
				auto candidate = windows::visual_studio_environment(vs, req);
				if (!candidate.empty()) {
					return candidate;
				}
			}
		}
#endif // defined(_WIN32)

		return env;
	}
}

#endif // MGMAKE_DISCOVERY_ENVIRONMENT_PROVIDER_HXX
