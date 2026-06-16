#pragma once

#ifndef MGMAKE_DISCOVERY_CONTEXT_HXX
#define MGMAKE_DISCOVERY_CONTEXT_HXX

#include "../build/request.hxx"
#include "../cli/options.hxx"
#include "cache.hxx"
#include "mode.hxx"
#include "resolved_tool.hxx"

#include <filesystem>
#include <string>
#include <vector>

namespace mgmake::spec { struct project; }

namespace mgmake::discovery {
	struct context {
		const cli::options* m_options = nullptr;
		const spec::project* m_project = nullptr;
		const build::request* m_request = nullptr;
		cache m_cache{};
		std::vector<resolved_tool> m_resolved_tools{};
		std::vector<rejected_tool_candidate> m_rejected{};
		std::vector<std::string> m_notes{};
		mode m_mode = mode::exact;
		bool m_use_cache = true;
		bool m_refresh_cache = false;
		bool m_verbose = false;
		bool m_show_search = false;

		[[nodiscard]] inline const cli::options& options() const {
			return *m_options;
		}

		[[nodiscard]] inline const build::request& request() const {
			return *m_request;
		}

		[[nodiscard]] inline const build::toolchain& toolchain() const {
			return request().toolchain();
		}
	};
}

#endif
