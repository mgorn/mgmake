#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX

#include "target.hxx"
#include "../ext/provider_ref.hxx"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct executable : public target<executable> {
		using id = std::vector<executable>::size_type;

		std::optional<ext::provider_ref> m_provider;
		std::optional<ext::rooted_path> m_artifact;

		inline constexpr executable(std::string_view name)
			: target<executable>{ std::string{ name } } {
			mgmkassert(not m_name.empty(), "mgmake spec: executable target has no name");
		}

		inline executable& from(const ext::provider_ref& provider) {
			mgmkassert(!provider.m_project.empty(), "mgmake spec: provider-backed executable has no provider project");
			mgmkassert(!provider.m_target.empty(), "mgmake spec: provider-backed executable has no provider target");
			m_provider = provider;
			return *this;
		}

		[[nodiscard]] inline bool provider_backed() const noexcept {
			return m_provider.has_value();
		}

		inline executable& artifact(const std::filesystem::path& path) {
			return artifact(ext::output_root::install_dir, path);
		}

		inline executable& artifact(
			ext::output_root root,
			const std::filesystem::path& path
		) {
			mgmkassert(!path.empty(), "mgmake spec: executable '" + m_name + "' has an empty external artifact path");
			m_artifact = ext::rooted_path{root, path};
			return *this;
		}
	};
}

#endif
