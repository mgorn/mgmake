#pragma once

#ifndef MGMK_EXT_CMAKE_TARGET_HXX
#define MGMK_EXT_CMAKE_TARGET_HXX

#include "../json.hxx"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

// CMake target metadata is parsed from File API target-*.json replies and used to locate artifacts and link usage.

namespace mgmake::ext::cmake {
	enum struct link_entry_kind {
		fragment,
		target_id
	};

	struct link_entry {
		// CMake keeps linkLibraries/interfaceLinkLibraries as ordered arrays whose entries can be
		// raw linker fragments or references to other CMake targets. Keep the kind next to the
		// value so mgmake can replay that order when lowering provider target usage.
		link_entry_kind m_kind = link_entry_kind::fragment;
		std::string m_value;
	};

	struct target {
		std::string m_name;
		std::string m_id;
		std::string m_type;
		std::vector<std::filesystem::path> m_artifacts;
		std::vector<link_entry> m_link_entries;
		std::vector<link_entry> m_interface_link_entries;

		[[nodiscard]] std::filesystem::path primary_artifact() const {
			if (m_artifacts.empty()) {
				return {};
			}

			return m_artifacts.front();
		}

		[[nodiscard]] static std::expected<target, std::string> from_file_api_json(
			const ext::json& parsed,
			const std::filesystem::path& build_dir,
			const std::filesystem::path& source_file = {}
		) {
			if (!parsed.is_object()) {
				return std::unexpected{target_parse_error(source_file, "target JSON root is not an object")};
			}

			const auto name = parsed.get("name");

			if (!name.has_value()) {
				return std::unexpected{target_parse_error(source_file, "missing string member 'name'")};
			}

			const auto name_text = name->as_string();

			if (!name_text.has_value() || name_text->empty()) {
				return std::unexpected{target_parse_error(source_file, "member 'name' is not a non-empty string")};
			}

			target result{};
			result.m_name = *name_text;

			if (const auto id = parsed.get("id")) {
				const auto id_text = id->as_string();

				if (!id_text.has_value()) {
					return std::unexpected{target_parse_error(source_file, "member 'id' is present but is not a string")};
				}

				result.m_id = *id_text;
			}

			if (const auto type = parsed.get("type")) {
				const auto type_text = type->as_string();

				if (!type_text.has_value()) {
					return std::unexpected{target_parse_error(source_file, "member 'type' is present but is not a string")};
				}

				result.m_type = *type_text;
			}

			for (const auto& artifact : parsed.array("artifacts")) {
				const auto path = artifact.get("path");

				if (!path.has_value()) {
					continue;
				}

				const auto path_text = path->as_string();

				if (!path_text.has_value() || path_text->empty()) {
					return std::unexpected{target_parse_error(source_file, "artifact path is not a non-empty string")};
				}

				std::filesystem::path artifact_path{*path_text};

				if (artifact_path.is_relative()) {
					artifact_path = build_dir / artifact_path;
				}

				result.m_artifacts.emplace_back(std::move(artifact_path));
			}

			const auto links = append_file_api_link_entries(
				parsed,
				"linkLibraries",
				result.m_link_entries,
				source_file
			);

			if (!links.has_value()) {
				return std::unexpected{links.error()};
			}

			const auto interface_links = append_file_api_link_entries(
				parsed,
				"interfaceLinkLibraries",
				result.m_interface_link_entries,
				source_file
			);

			if (!interface_links.has_value()) {
				return std::unexpected{interface_links.error()};
			}

			return result;
		}

	private:
		[[nodiscard]] static std::string target_parse_error(
			const std::filesystem::path& source_file,
			std::string_view message
		) {
			if (source_file.empty()) {
				return std::string{message};
			}

			return "failed to parse CMake File API target '" + source_file.string() + "': " + std::string{message};
		}

		[[nodiscard]] static std::expected<void, std::string> append_file_api_link_entries(
			const ext::json& parsed,
			std::string_view key,
			std::vector<link_entry>& out,
			const std::filesystem::path& source_file
		) {
			for (const auto& item : parsed.array(key)) {
				if (const auto fragment = item.get("fragment")) {
					const auto text = fragment->as_string();

					if (!text.has_value() || text->empty()) {
						return std::unexpected{target_parse_error(source_file, "link fragment is not a non-empty string")};
					}

					out.emplace_back(link_entry{
						.m_kind = link_entry_kind::fragment,
						.m_value = *text
					});
				}

				if (const auto id = item.get("id")) {
					const auto text = id->as_string();

					if (!text.has_value() || text->empty()) {
						return std::unexpected{target_parse_error(source_file, "link target id is not a non-empty string")};
					}

					out.emplace_back(link_entry{
						.m_kind = link_entry_kind::target_id,
						.m_value = *text
					});
				}
			}

			return {};
		}
	};
}

#endif // MGMK_EXT_CMAKE_TARGET_HXX
