#pragma once

#ifndef MGMK_EXT_CMAKE_TARGET_HXX
#define MGMK_EXT_CMAKE_TARGET_HXX

#include <filesystem>
#include <string>
#include <vector>

// CMake target metadata is parsed from File API replies and used to locate build-tree artifacts.

namespace mgmake::ext::cmake {
	struct target {
		std::string m_name;
		std::string m_type;
		std::vector<std::filesystem::path> m_artifacts;

		[[nodiscard]] std::filesystem::path primary_artifact() const {
			if (m_artifacts.empty()) {
				return {};
			}

			return m_artifacts.front();
		}
	};
}

#endif // MGMK_EXT_CMAKE_TARGET_HXX
