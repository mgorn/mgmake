#pragma once

#ifndef MGMK_EXT_CMAKE_HXX
#define MGMK_EXT_CMAKE_HXX

#include "provider_ref.hxx"
#include "fetch.hxx"
#include "../spec/executable.hxx"
#include "../spec/library.hxx"

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::ext {
	struct cmake {
		using id = std::vector<cmake>::size_type;

		std::string m_name;
		std::optional<ext::fetch> m_source;
		std::vector<std::string> m_args;
		std::vector<std::pair<std::string, std::string>> m_defines;
		std::vector<std::string> m_build_targets;
		bool m_install = false;
		std::string m_install_target = "install";
		std::string m_generator;
		std::string m_build_config;

		explicit cmake(std::string_view name)
			: m_name{name} {
			mgmkassert(!m_name.empty(), "mgmake ext: CMake project has no name");
		}

		cmake& source(const ext::fetch& fetch) {
			m_source = fetch;
			return *this;
		}

		cmake& arg(std::string_view value) {
			mgmkassert(!value.empty(), "mgmake ext: CMake argument is empty");
			m_args.emplace_back(value);
			return *this;
		}

		cmake& define(std::string_view key, std::string_view value) {
			mgmkassert(!key.empty(), "mgmake ext: CMake define key is empty");
			m_defines.emplace_back(std::string{key}, std::string{value});
			return *this;
		}

		cmake& build_target(std::string_view target) {
			mgmkassert(!target.empty(), "mgmake ext: CMake build target is empty");
			m_build_targets.emplace_back(target);
			return *this;
		}

		cmake& install(bool enabled = true) {
			m_install = enabled;
			return *this;
		}

		cmake& install_target(std::string_view target) {
			mgmkassert(!target.empty(), "mgmake ext: CMake install target is empty");
			m_install_target = std::string{target};
			m_install = true;
			return *this;
		}

		cmake& generator(std::string_view value) {
			mgmkassert(!value.empty(), "mgmake ext: CMake generator is empty");
			m_generator = std::string{value};
			return *this;
		}

		cmake& build_config(std::string_view value) {
			mgmkassert(!value.empty(), "mgmake ext: CMake build configuration is empty");
			m_build_config = std::string{value};
			return *this;
		}

		[[nodiscard]] spec::library library(
			std::string_view target,
			spec::library::kind kind
		) const {
			mgmkassert(!target.empty(), "mgmake ext: CMake library target is empty");
			auto result = spec::library{target, kind};
			result.from(ext::provider_ref{
				.m_kind = ext::provider_kind::cmake,
				.m_project = m_name,
				.m_target = std::string{target},
				.m_usage_root = ext::output_root::install_dir
			});
			return result;
		}

		[[nodiscard]] spec::executable executable(std::string_view target) const {
			mgmkassert(!target.empty(), "mgmake ext: CMake executable target is empty");
			auto result = spec::executable{target};
			result.from(ext::provider_ref{
				.m_kind = ext::provider_kind::cmake,
				.m_project = m_name,
				.m_target = std::string{target},
				.m_usage_root = ext::output_root::install_dir
			});
			return result;
		}
	};
}

#endif
