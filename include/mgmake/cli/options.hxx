#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "action.hxx"
#include "backend.hxx"

#include <string>
#include <vector>

namespace mgmake::cli {
	struct options {
		action_kind action = action_kind::build;
		backend_kind backend = backend_kind::automatic;

		std::string build_dir = "build";

		std::vector<std::string> targets;
		std::vector<std::string> passthrough_args;

		int jobs = 0;

		bool verbose = false;
		bool dry_run = false;
		bool show_help = false;
		bool show_version = false;
	};
}

#endif