#pragma once

#ifndef MGMK_EXT_CMAKE_PROJECT_HXX
#define MGMK_EXT_CMAKE_PROJECT_HXX

#include "../fetch.hxx"
#include "../provided_target_ref.hxx"
#include "../../detail/assert.hxx"
#include "../../spec/executable.hxx"
#include "../../spec/library.hxx"

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The CMake project spec is the user-facing declaration of an external CMake project.

namespace mgmake::ext::cmake {
	struct project {
		using id = std::vector<project>::size_type;

		std::string m_name;
		std::optional<ext::fetch> m_source;
		std::vector<std::string> m_args;
		std::vector<std::pair<std::string, std::string>> m_defines;
		bool m_install = false;
		std::string m_install_target = "install";
		std::string m_generator;
		std::string m_build_config;

		explicit project(std::string_view name)
			: m_name{name} {
			mgmkassert(!m_name.empty(), "mgmake ext: CMake project has no name");
		}

		project& source(const ext::fetch& fetch) {
			m_source = fetch;
			return *this;
		}

		project& arg(std::string_view value) {
			mgmkassert(!value.empty(), "mgmake ext: CMake argument is empty");
			m_args.emplace_back(value);
			return *this;
		}

		project& define(std::string_view key, std::string_view value) {
			mgmkassert(!key.empty(), "mgmake ext: CMake define key is empty");
			m_defines.emplace_back(std::string{key}, std::string{value});
			return *this;
		}

		project& install(bool enabled = true) {
			m_install = enabled;
			return *this;
		}

		project& install_target(std::string_view target) {
			mgmkassert(!target.empty(), "mgmake ext: CMake install target is empty");
			m_install_target = std::string{target};
			m_install = true;
			return *this;
		}

		project& generator(std::string_view value) {
			mgmkassert(!value.empty(), "mgmake ext: CMake generator is empty");
			m_generator = std::string{value};
			return *this;
		}

		project& build_config(std::string_view value) {
			mgmkassert(!value.empty(), "mgmake ext: CMake build configuration is empty");
			m_build_config = std::string{value};
			return *this;
		}

		[[nodiscard]] ext::provided_target_ref target(std::string_view cmake_target) const {
			mgmkassert(!cmake_target.empty(), "mgmake ext: CMake target is empty");
			return ext::provided_target_ref{
				.m_kind = ext::provider_kind::cmake,
				.m_project = m_name,
				.m_target = std::string{cmake_target}
			};
		}

		[[nodiscard]] spec::library library(
			std::string_view cmake_target,
			spec::library::kind kind
		) const {
			return library(cmake_target, cmake_target, kind);
		}

		[[nodiscard]] spec::library library(
			std::string_view name,
			std::string_view cmake_target,
			spec::library::kind kind
		) const {
			mgmkassert(!name.empty(), "mgmake ext: CMake library name is empty");
			auto result = spec::library{name, kind};
			result.from(target(cmake_target));
			return result;
		}

		[[nodiscard]] spec::executable executable(std::string_view cmake_target) const {
			return executable(cmake_target, cmake_target);
		}

		[[nodiscard]] spec::executable executable(
			std::string_view name,
			std::string_view cmake_target
		) const {
			mgmkassert(!name.empty(), "mgmake ext: CMake executable name is empty");
			auto result = spec::executable{name};
			result.from(target(cmake_target));
			return result;
		}
	};
}

#endif // MGMK_EXT_CMAKE_PROJECT_HXX
