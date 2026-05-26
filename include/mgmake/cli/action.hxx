#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

#include <string_view>

namespace mgmake::cli {
	enum struct action_kind {
		build,
		generate,
		clean,
		run,
		help,
		version
	};

	[[nodiscard]] inline constexpr std::string_view action_name(action_kind action) {
		switch (action) {
			case action_kind::build:
				return "build";
			case action_kind::generate:
				return "generate";
			case action_kind::clean:
				return "clean";
			case action_kind::run:
				return "run";
			case action_kind::help:
				return "help";
			case action_kind::version:
				return "version";
		}

		return "unknown";
	}

	[[nodiscard]] inline constexpr bool parse_action(std::string_view text, action_kind& out) {
		if (text == "build") {
			out = action_kind::build;
			return true;
		}

		if (text == "generate" || text == "gen") {
			out = action_kind::generate;
			return true;
		}

		if (text == "clean") {
			out = action_kind::clean;
			return true;
		}

		if (text == "run") {
			out = action_kind::run;
			return true;
		}

		if (text == "help") {
			out = action_kind::help;
			return true;
		}

		if (text == "version") {
			out = action_kind::version;
			return true;
		}

		return false;
	}
}

#endif