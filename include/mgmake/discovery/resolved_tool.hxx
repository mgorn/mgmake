#pragma once

#ifndef MGMAKE_DISCOVERY_RESOLVED_TOOL_HXX
#define MGMAKE_DISCOVERY_RESOLVED_TOOL_HXX

#include "tool_family.hxx"
#include "tool_provider.hxx"
#include "tool_role.hxx"

#include <filesystem>
#include <optional>
#include <string>

namespace mgmake::discovery {
	struct tool_candidate {
		tool_role m_role{};
		std::string m_logical_name{};
		std::filesystem::path m_path{};
		tool_provider m_provider{};
		std::string m_reason{};
		int m_priority = 0;
		bool m_authoritative = false;
		std::optional<std::filesystem::path> m_provider_root{};
	};

	struct rejected_tool_candidate {
		tool_candidate m_candidate{};
		std::string m_reason{};
	};

	struct searched_tool_candidate {
		tool_candidate m_candidate{};
		std::string m_status{};
	};

	struct resolved_tool {
		tool_role m_role{};
		std::string m_logical_name{};
		std::filesystem::path m_path{};
		tool_provider m_provider{};
		tool_family m_family = tool_family::unknown;
		linker_flavor m_linker_flavor = linker_flavor::unknown;
		object_format m_object_format = object_format::unknown;
		std::string m_version{};
		std::string m_target_triple{};
		std::string m_reason{};
		std::optional<std::filesystem::path> m_provider_root{};

		[[nodiscard]] inline const std::filesystem::path& path() const noexcept {
			return m_path;
		}

		[[nodiscard]] inline std::string path_string() const {
#if defined(MGMK_PLATFORM_WINDOWS)
			return m_path.generic_string();
#else
			return m_path.string();
#endif // defined(MGMK_PLATFORM_WINDOWS)
		}
	};
}

#endif // MGMAKE_DISCOVERY_RESOLVED_TOOL_HXX
