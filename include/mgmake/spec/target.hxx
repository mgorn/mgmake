#pragma once

#ifndef MGMK_SPEC_TARGET_HXX
#define MGMK_SPEC_TARGET_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

#include <filesystem>

namespace mgmake::spec {
	struct target {
		std::string m_name;
		std::vector<std::filesystem::path> m_sources;

		inline constexpr auto name(const std::filesystem::path& file) {
			m_name = file;
			return *this;
		}
		inline constexpr auto add_source(const std::filesystem::path& file) {
			m_sources.emplace_back(file);
			return *this;
		}
	};
}

#endif