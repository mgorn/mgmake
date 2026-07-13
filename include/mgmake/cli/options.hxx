#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "action.hxx"

#include <filesystem>

namespace mgmake::cli {
	// Store parsed CLI options
	struct options {
		// Build action by default
		std::size_t m_action_id = 0;//meta::static_string{"build"}.hash();

		bool m_verbose = false;
		bool m_dry_run = false;

		std::filesystem::path m_build_dir = std::filesystem::current_path() / ".build";
	};
}

#endif // MGMAKE_CLI_OPTIONS_HXX