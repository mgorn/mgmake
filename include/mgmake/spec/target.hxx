#pragma once

#ifndef MGMK_SPEC_TARGET_HXX
#define MGMK_SPEC_TARGET_HXX

#include "../detail/assert.hxx"

#include <filesystem>
#include <set>
#include <string>
#include <string_view>

namespace mgmake::spec {
	template<typename target_t>
	struct target {
		std::string m_name;
		std::set<std::filesystem::path> m_sources;
		std::set<std::filesystem::path> m_include_dirs;
		std::set<std::string> m_linked_libraries;

		inline constexpr auto& name() const {
			return m_name;
		}

		inline constexpr auto& add_source(const std::filesystem::path& file) {
			mgmkassert(
				not file.empty(),
				"mgmake spec: target '" + m_name + "' cannot add an empty source path"
			);

			m_sources.emplace(file);
			return self();
		}
		inline constexpr auto& sources() const {
			return m_sources;
		}

		inline constexpr auto& add_include_dir(const std::filesystem::path& file) {
			mgmkassert(
				not file.empty(),
				"mgmake spec: target '" + m_name + "' cannot add an empty include directory"
			);

			m_include_dirs.emplace(file);
			return self();
		}
		inline constexpr auto& include_dir(const std::filesystem::path& file) {
			return add_include_dir(file);
		}

		inline constexpr auto& include_dirs() const {
			return m_include_dirs;
		}

		inline constexpr auto& link(std::string_view lib) {
			mgmkassert(
				not lib.empty(),
				"mgmake spec: target '" + m_name + "' cannot link an empty library name"
			);
			mgmkassert(
				lib != m_name,
				"mgmake spec: target '" + m_name + "' cannot link itself"
			);

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

#endif // MGMK_SPEC_TARGET_HXX
