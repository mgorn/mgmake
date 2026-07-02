#pragma once

#ifndef MGMK_SPEC_LIBRARY_HXX
#define MGMK_SPEC_LIBRARY_HXX

#include "target.hxx"
#include "../ext/provided_target_ref.hxx"
#include "../ext/rooted_path.hxx"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Library specs may be interface, static, shared, or provider-backed, and propagate usage to dependents.

namespace mgmake::spec {
	struct project;

	struct library : public target<library> {
		using id = std::vector<library>::size_type;

		enum struct kind {
			static_lib,
			shared_lib,
			interface
		} m_kind;

		std::optional<ext::provided_target_ref> m_provider;
		std::optional<ext::rooted_path> m_artifact;
		std::vector<ext::rooted_path> m_external_include_dirs;

		inline constexpr library(std::string_view name, kind k)
			: target<library>{ std::string{ name } }, m_kind{k} {
			mgmkassert(not m_name.empty(), "mgmake spec: library target has no name");
			mgmkassert(
				m_kind == kind::static_lib ||
					m_kind == kind::shared_lib ||
					m_kind == kind::interface,
				"mgmake spec: invalid library kind"
			);
		}

		inline constexpr auto& add_source(const std::filesystem::path& file) {
			mgmkassert(
				m_kind != kind::interface,
				"mgmake spec: interface library '" + m_name + "' cannot have sources"
			);

			target<library>::add_source(file);
			return *this;
		}

		inline library& from(const ext::provided_target_ref& provider) {
			mgmkassert(!provider.m_project.empty(), "mgmake spec: provider-backed library has no provider project");
			mgmkassert(!provider.m_target.empty(), "mgmake spec: provider-backed library has no provider target");
			m_provider = provider;
			return *this;
		}

		[[nodiscard]] inline bool provider_backed() const noexcept {
			return m_provider.has_value();
		}

		inline library& artifact(const std::filesystem::path& path) {
			return artifact(ext::path_root::usage, path);
		}

		inline library& artifact(
			ext::path_root root,
			const std::filesystem::path& path
		) {
			mgmkassert(!path.empty(), "mgmake spec: library '" + m_name + "' has an empty external artifact path");
			m_artifact = ext::rooted_path{root, path};
			return *this;
		}

		inline library& include_dir(const std::filesystem::path& path) {
			if (provider_backed()) {
				return include_dir(ext::path_root::usage, path);
			}

			add_include_dir(path);
			return *this;
		}

		inline library& include_dir(
			ext::path_root root,
			const std::filesystem::path& path
		) {
			mgmkassert(!path.empty(), "mgmake spec: library '" + m_name + "' has an empty external include directory");
			m_external_include_dirs.push_back(ext::rooted_path{root, path});
			return *this;
		}
	};
}

#endif // MGMK_SPEC_LIBRARY_HXX
