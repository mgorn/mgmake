#pragma once

#ifndef MGMK_EXT_CMAKE_CODEMODEL_HXX
#define MGMK_EXT_CMAKE_CODEMODEL_HXX

#include "target.hxx"

#include <cstddef>
#include <expected>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// A CMake codemodel owns the assembled target metadata mgmake needs after CMake configuration.

namespace mgmake::ext::cmake {
	struct codemodel {
		std::map<std::string, target> m_targets_by_id;
		std::map<std::string, std::string> m_target_ids_by_name;
		std::map<std::string, std::string> m_target_ids_by_artifact_filename;

		[[nodiscard]] bool empty() const noexcept {
			return m_targets_by_id.empty();
		}

		[[nodiscard]] std::size_t size() const noexcept {
			return m_targets_by_id.size();
		}

		[[nodiscard]] const target* find_target(std::string_view name) const {
			const auto found = m_target_ids_by_name.find(std::string{name});

			if (found == m_target_ids_by_name.end()) {
				return nullptr;
			}

			return find_target_id(found->second);
		}

		[[nodiscard]] const target* find_target_name_or_unqualified_alias(std::string_view name) const {
			if (const auto* found = find_target(name)) {
				return found;
			}

			const auto text = std::string{name};
			const auto namespace_pos = text.rfind("::");

			if (namespace_pos == std::string::npos) {
				return nullptr;
			}

			return find_target(text.substr(namespace_pos + 2));
		}

		[[nodiscard]] const target* find_target_id(std::string_view id) const {
			const auto found = m_targets_by_id.find(std::string{id});

			if (found == m_targets_by_id.end()) {
				return nullptr;
			}

			return &found->second;
		}

		[[nodiscard]] const target* find_target_artifact(const std::filesystem::path& artifact) const {
			const auto filename = artifact.filename().string();

			if (filename.empty()) {
				return nullptr;
			}

			const auto found = m_target_ids_by_artifact_filename.find(filename);

			if (found == m_target_ids_by_artifact_filename.end()) {
				return nullptr;
			}

			return find_target_id(found->second);
		}

		void add_target(target value) {
			const auto key = value.m_id.empty()
				? value.m_name
				: value.m_id;

			if (!value.m_name.empty()) {
				m_target_ids_by_name.insert_or_assign(value.m_name, key);
			}

			for (const auto& artifact : value.m_artifacts) {
				const auto filename = artifact.filename().string();

				if (!filename.empty()) {
					m_target_ids_by_artifact_filename.insert_or_assign(filename, key);
				}
			}

			m_targets_by_id.insert_or_assign(key, std::move(value));
		}

		[[nodiscard]] static std::expected<codemodel, std::string> from_file_api_reply(
			const std::filesystem::path& build_dir
		);

		[[nodiscard]] static std::expected<codemodel, std::string> from_target_files(
			const std::filesystem::path& reply_dir,
			const std::filesystem::path& build_dir,
			const std::vector<std::filesystem::path>& target_files
		);
	};
}

#endif // MGMK_EXT_CMAKE_CODEMODEL_HXX
