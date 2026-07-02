#pragma once

#ifndef MGMK_EXT_CMAKE_TARGET_HXX
#define MGMK_EXT_CMAKE_TARGET_HXX

#include <filesystem>
#include <string>
#include <vector>

// CMake target metadata is parsed from File API replies and used to locate build-tree artifacts and link usage.

namespace mgmake::ext::cmake {
	enum struct link_entry_kind {
		fragment,
		target_id
	};

	struct link_entry {
		// CMake keeps linkLibraries/interfaceLinkLibraries as ordered arrays whose entries can be
		// raw linker fragments or references to other CMake targets. Keep the kind next to the
		// value so mgmake can replay that order when lowering provider target usage.
		link_entry_kind m_kind = link_entry_kind::fragment;
		std::string m_value;
	};

	struct target {
		std::string m_name;
		std::string m_id;
		std::string m_type;
		std::vector<std::filesystem::path> m_artifacts;
		std::vector<link_entry> m_link_entries;
		std::vector<link_entry> m_interface_link_entries;

		[[nodiscard]] std::filesystem::path primary_artifact() const {
			if (m_artifacts.empty()) {
				return {};
			}

			return m_artifacts.front();
		}

		[[nodiscard]] bool has_link_usage() const noexcept {
			return !m_link_entries.empty() || !m_interface_link_entries.empty();
		}
	};
}

#endif // MGMK_EXT_CMAKE_TARGET_HXX
