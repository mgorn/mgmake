#pragma once

#ifndef MGMAKE_FIND_TOOL_CANDIDATE_HXX
#define MGMAKE_FIND_TOOL_CANDIDATE_HXX

#include <filesystem>

namespace mgmake::find {
	struct tool_candidate {
		// Full path to found tool executable
		std::filesystem::path m_executable{};
		// What searcher found it
		std::string m_searcher{};
	};
}

#endif // MGMAKE_FIND_TOOL_CANDIDATE_HXX