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

			for (const auto& vs : windows::visual_studio_instances()) {
				auto candidate = windows::visual_studio_environment(vs, req);
				if (!candidate.empty()) {
					return candidate;
				}
			}
		}
#endif

		return env;
	}
}

#endif
