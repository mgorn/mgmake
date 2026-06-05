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
		std::set<std::filesystem::path> m_sources;
		std::set<std::filesystem::path> m_include_dirs;
		std::set<std::string> m_linked_libraries;

		inline constexpr auto& add_source(const std::filesystem::path& file) {
			m_sources.emplace(file);
			return self();
		}

		inline constexpr auto& add_include_dir(const std::filesystem::path& file) {
			m_include_dirs.emplace(file);
			return self();
		}
		inline constexpr auto& include_dirs() const {
			return m_include_dirs;
		}

		inline constexpr auto& link(std::string_view lib) {
			m_linked_libraries.emplace(lib);
			return self();
		}
		inline constexpr auto& link(const std::string& lib) {
			return link(std::string_view{ lib });
		}
		inline constexpr auto& linked_libraries() const {
			return m_linked_libraries;
		}

		// Implicit cast to std::string_view for when the target needs to be identified by name
		operator std::string_view() const {
			return m_name;
		}
	private:
		inline constexpr target_t& self() {
			return *static_cast<target_t*>(this);
		}
	};
}

#endif