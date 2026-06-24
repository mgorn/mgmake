#pragma once

#ifndef MGMAKE_DEP_DATABASE_HXX
#define MGMAKE_DEP_DATABASE_HXX

#include "file.hxx"
#include "parser.hxx"

#include <filesystem>
#include <map>
#include <set>
#include <span>
#include <system_error>
#include <utility>
#include <vector>

// The dependency database is populated from depfiles from previous builds and feeds discovered headers back into the next DAG.

namespace mgmake::dep {
	struct database {
		std::set<std::filesystem::path> m_consumed;

		std::map<
			std::filesystem::path,
			std::vector<std::filesystem::path>
		> m_dependencies_by_target;

		// Missing depfiles are normal on the first build, so consume() quietly treats them as no data yet.
		void consume(const dep::file& depfile) {
			if (depfile.m_path.empty()) {
				return;
			}

			if (m_consumed.contains(depfile.m_path)) {
				return;
			}

			std::error_code ec;

			if (!std::filesystem::exists(depfile.m_path, ec) || ec) {
				return;
			}

			m_consumed.emplace(depfile.m_path);

			const auto content = read_text_file(depfile.m_path);

			if (!content.has_value()) {
				return;
			}

			std::vector<std::filesystem::path> dependencies{};

			if (depfile.m_format.has(format_bits::gcc)) {
				dependencies = parse_gcc_make_depfile(*content);
			} else if (depfile.m_format.has(format_bits::msvc_source_dependencies)) {
				dependencies = parse_msvc_source_dependencies(*content);
			}

			if (!dependencies.empty()) {
				m_dependencies_by_target.insert_or_assign(
					depfile.m_target,
					std::move(dependencies)
				);
			}
		}

		[[nodiscard]] std::span<const std::filesystem::path> dependencies_for(
			const std::filesystem::path& target
		) const {
			static const std::vector<std::filesystem::path> empty{};

			const auto found = m_dependencies_by_target.find(target);

			if (found == m_dependencies_by_target.end()) {
				return std::span<const std::filesystem::path>{
					empty.data(),
					empty.size()
				};
			}

			return std::span<const std::filesystem::path>{
				found->second.data(),
				found->second.size()
			};
		}
	};
}

#endif // MGMAKE_DEP_DATABASE_HXX
