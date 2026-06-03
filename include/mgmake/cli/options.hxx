#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "action.hxx"
#include "backend.hxx"

#include <string>
#include <vector>

namespace mgmake::cli {
	struct options {
		action_kind m_action = action_kind::build;
		backend_kind m_backend = backend_kind::automatic;

		std::string m_build_dir = ".build";

		std::vector<std::string> m_targets;
		std::vector<std::string> m_passthrough_args;

		int m_jobs = 0;

		bool m_verbose = false;
		bool m_dry_run = false;
		bool m_show_help = false;
		bool m_show_version = false;
	};
}

#endif