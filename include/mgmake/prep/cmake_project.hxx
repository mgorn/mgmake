#pragma once

#ifndef MGMK_PREP_CMAKE_PROJECT_HXX
#define MGMK_PREP_CMAKE_PROJECT_HXX

#include "../ext/cmake/codemodel.hxx"
#include "../ext/path_root.hxx"
#include "../ext/rooted_path.hxx"

#include <filesystem>
#include <string>
#include <string_view>

// Prepared CMake projects store mgmake's resolved external project directories and parsed CMake metadata.

namespace mgmake::prep {
	struct cmake_project {
		std::string m_name;
		std::filesystem::path m_source_dir;
		std::filesystem::path m_build_dir;
		std::filesystem::path m_install_dir;
		ext::path_root m_usage_root = ext::path_root::build;
		ext::cmake::codemodel m_codemodel;

		[[nodiscard]] std::filesystem::path root(ext::path_root root) const {
			switch (root) {
				case ext::path_root::usage: return this->root(m_usage_root);
				case ext::path_root::source: return m_source_dir;
				case ext::path_root::build: return m_build_dir;
				case ext::path_root::install: return m_install_dir;
			}

			return m_build_dir;
		}

		[[nodiscard]] std::filesystem::path resolve(const ext::rooted_path& path) const {
			if (path.m_path.is_absolute()) {
				return path.m_path;
			}

			return root(path.m_root) / path.m_path;
		}

		[[nodiscard]] const ext::cmake::target* find_target(std::string_view name) const {
			return m_codemodel.find_target(name);
		}
	};
}

#endif // MGMK_PREP_CMAKE_PROJECT_HXX
