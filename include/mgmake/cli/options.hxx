#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "default_actions.hxx"

#include <filesystem>
#include <optional>

namespace mgmake::cli {
	// Store parsed CLI options
	struct options {
		std::optional<std::size_t> m_action = default_actions::index<build_action>();

		bool m_verbose = false;
		bool m_dry_run = false;

		std::filesystem::path m_build_dir = std::filesystem::current_path() / ".build";

		inline constexpr auto action() const {
			return m_action;
		}
	};
}

#endif // MGMAKE_CLI_OPTIONS_HXX