#pragma once

#ifndef MGMK_EXT_CMAKE_CODEMODEL_HXX
#define MGMK_EXT_CMAKE_CODEMODEL_HXX

#include "target.hxx"

#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <utility>

// A CMake codemodel stores the target metadata mgmake needs after CMake configuration.

namespace mgmake::ext::cmake {
	struct codemodel {
		std::map<std::string, target> m_targets;
		std::map<std::string, std::string> m_target_names_by_id;
		std::map<std::string, std::string> m_target_names_by_artifact_filename;

		[[nodiscard]] bool empty() const noexcept {
			return m_targets.empty();
		}

		[[nodiscard]] std::size_t size() const noexcept {
			return m_targets.size();
		}

		[[nodiscard]] const target* find_target(std::string_view name) const {
			const auto found = m_targets.find(std::string{name});

			if (found != m_targets.end()) {
				return &found->second;
			}

			const auto text = std::string{name};
			const auto namespace_pos = text.rfind("::");

			if (namespace_pos != std::string::npos) {
				const auto unqualified = text.substr(namespace_pos + 2);
				const auto unqualified_found = m_targets.find(unqualified);

				if (unqualified_found != m_targets.end()) {
					return &unqualified_found->second;
				}
			}

			return nullptr;
		}

		[[nodiscard]] const target* find_target_id(std::string_view id) const {
			const auto found = m_target_names_by_id.find(std::string{id});

			if (found == m_target_names_by_id.end()) {
				return nullptr;
			}

			return find_target(found->second);
		}

		[[nodiscard]] const target* find_target_artifact(const std::filesystem::path& artifact) const {
			const auto filename = artifact.filename().string();

			if (filename.empty()) {
				return nullptr;
			}

			const auto found = m_target_names_by_artifact_filename.find(filename);

			if (found == m_target_names_by_artifact_filename.end()) {
				return nullptr;
			}

			return find_target(found->second);
		}

		void add_target(target value) {
			const auto name = value.m_name;

			if (!value.m_id.empty()) {
				m_target_names_by_id.insert_or_assign(value.m_id, name);
			}

			for (const auto& artifact : value.m_artifacts) {
				const auto filename = artifact.filename().string();

				if (!filename.empty()) {
					m_target_names_by_artifact_filename.insert_or_assign(filename, name);
				}
			}

			m_targets.insert_or_assign(name, std::move(value));
		}
	};
}

#endif // MGMK_EXT_CMAKE_CODEMODEL_HXX
