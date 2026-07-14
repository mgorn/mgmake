#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "../task/build.hxx"
#include "../task/default_tasks.hxx"

#include <filesystem>
#include <optional>

namespace mgmake::cli {
	// Store parsed CLI options
	struct options {
		std::optional<std::size_t> m_task = task::default_tasks::index<task::build>();

		bool m_verbose = false;
		bool m_dry_run = false;

		std::filesystem::path m_build_dir = std::filesystem::current_path() / ".build";

		inline constexpr auto task() const {
			return m_task;
		}
	};
}

#endif // MGMAKE_CLI_OPTIONS_HXX