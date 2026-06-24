#pragma once

#ifndef MGMAKE_BUILD_REQUEST_FROM_OPTIONS_HXX
#define MGMAKE_BUILD_REQUEST_FROM_OPTIONS_HXX

#include "request.hxx"
#include "toolchain_registry.hxx"
#include "../cli/options.hxx"

#include <expected>
#include <filesystem>
#include <format>
#include <string>

// Converts parsed CLI options into a build request while keeping toolchain selection errors near the CLI boundary.

namespace mgmake::build {
	template <toolchain_registry_like Toolchains>
	[[nodiscard]] inline std::expected<request, std::string> request_from_options(
		const cli::options& opts,
		const Toolchains& toolchains
	) {
		const auto* selected_toolchain = toolchains.find(opts.m_toolchain);

		if (selected_toolchain == nullptr) {
			return std::unexpected{
				std::format(
					"mgmake: unknown toolchain '{}'; expected one of: {}",
					opts.m_toolchain,
					toolchains.choices_string()
				)
			};
		}

		request result{};

		result.m_tc = *selected_toolchain;

		if (!opts.m_target_triple.empty()) {
			result.m_tc.target_triple(opts.m_target_triple);
		}

		result.m_build_dir = std::filesystem::path{ opts.m_build_dir };
		result.m_targets = opts.m_targets;
		result.m_target = opts.target_platform();

		return result;
	}
}

#endif // MGMAKE_BUILD_REQUEST_FROM_OPTIONS_HXX
