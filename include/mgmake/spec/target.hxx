#pragma once

#ifndef MGMK_SPEC_TARGET_HXX
#define MGMK_SPEC_TARGET_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

#include <filesystem>

namespace mgmake::spec {
	template<typename target_t>
	struct target {
		std::string m_name;
		std::vector<std::filesystem::path> m_sources;

		inline constexpr auto& add_source(const std::filesystem::path& file) {
			m_sources.emplace_back(file);
			return self();
		}

	private:
		inline constexpr target_t& self() {
			return *static_cast<target_t*>(this);
		}
	};
}

#endif