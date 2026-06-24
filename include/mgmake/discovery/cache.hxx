#pragma once

#ifndef MGMAKE_DISCOVERY_CACHE_HXX
#define MGMAKE_DISCOVERY_CACHE_HXX

#include "../build/request.hxx"
#include "../sys/platform.hxx"
#include "filesystem.hxx"
#include "resolved_tool.hxx"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The discovery cache records validated tools per host, target, toolchain, and role to avoid repeated searches.

namespace mgmake::discovery {
	struct cache_entry {
		std::string m_toolchain{};
		std::string m_host_key{};
		std::string m_target_key{};
		sys::target m_host{};
		sys::target m_target{};
		tool_role m_role{};
		std::string m_logical_name{};
		std::filesystem::path m_path{};
		tool_provider m_provider{};
		std::string m_version{};
	};

	[[nodiscard]] inline std::string target_key(const sys::target& target) {
		std::string result;
		result += std::string{sys::name(target.m_arch)};
		result += '-';
		result += std::string{sys::name(target.m_platform)};
		result += '-';
		result += std::string{sys::name(target.m_abi)};

		if (!target.m_triple.empty()) {
			result += '-';
			result += target.m_triple;
		}

		return result;
	}

	struct cache {
		std::vector<cache_entry> m_entries{};

		[[nodiscard]] std::optional<cache_entry> find(
			std::string_view toolchain,
			const sys::target& host,
			const sys::target& target,
			tool_role role,
			std::string_view logical_name
		) const {
			const auto host_key = target_key(host);
			const auto target_key_value = target_key(target);

			for (const auto& entry : m_entries) {
				const auto entry_host_key = entry.m_host_key.empty()
					? target_key(entry.m_host)
					: entry.m_host_key;
				const auto entry_target_key = entry.m_target_key.empty()
					? target_key(entry.m_target)
					: entry.m_target_key;

				if (entry.m_toolchain == toolchain
					&& entry_host_key == host_key
					&& entry_target_key == target_key_value
					&& entry.m_role == role
					&& entry.m_logical_name == logical_name
					&& is_launchable_file(entry.m_path)) {
					return entry;
				}
			}

			return std::nullopt;
		}

		void put(cache_entry entry) {
			if (entry.m_host_key.empty()) {
				entry.m_host_key = target_key(entry.m_host);
			}

			if (entry.m_target_key.empty()) {
				entry.m_target_key = target_key(entry.m_target);
			}

			for (auto& existing : m_entries) {
				if (existing.m_toolchain == entry.m_toolchain
					&& (existing.m_host_key.empty() ? target_key(existing.m_host) : existing.m_host_key) == entry.m_host_key
					&& (existing.m_target_key.empty() ? target_key(existing.m_target) : existing.m_target_key) == entry.m_target_key
					&& existing.m_role == entry.m_role
					&& existing.m_logical_name == entry.m_logical_name) {
					existing = std::move(entry);
					return;
				}
			}

			m_entries.emplace_back(std::move(entry));
		}
	};

	[[nodiscard]] inline std::filesystem::path cache_path(const build::request& req) {
		return req.build_dir() / "mgmake" / "tool-cache.txt";
	}

	[[nodiscard]] inline std::optional<std::string> value_after(
		std::string_view line,
		std::string_view prefix
	) {
		if (!line.starts_with(prefix)) {
			return std::nullopt;
		}

		return std::string{line.substr(prefix.size())};
	}

	[[nodiscard]] inline cache load_cache(const build::request& req) {
		cache result;
		std::ifstream in(cache_path(req));

		if (!in.is_open()) {
			return result;
		}

		std::string line;
		cache_entry current{};
		bool active = false;

		auto flush = [&] {
			if (active && !current.m_path.empty()) {
				result.m_entries.emplace_back(std::move(current));
			}

			current = cache_entry{};
			active = false;
		};

		while (std::getline(in, line)) {
			if (line.empty()) {
				flush();
				continue;
			}

			if (line == "mgmake-tool-cache-v1") {
				continue;
			}

			active = true;

			if (auto value = value_after(line, "toolchain=")) current.m_toolchain = *value;
			else if (auto value = value_after(line, "host=")) current.m_host_key = *value;
			else if (auto value = value_after(line, "target=")) current.m_target_key = *value;
			else if (auto value = value_after(line, "tool.role=")) {
				if (auto role = tool_role_names::from_string(*value)) current.m_role = *role;
			} else if (auto value = value_after(line, "tool.logical=")) current.m_logical_name = *value;
			else if (auto value = value_after(line, "tool.path=")) current.m_path = *value;
			else if (auto value = value_after(line, "tool.provider=")) {
				if (auto provider = tool_provider_names::from_string(*value)) current.m_provider = *provider;
			} else if (auto value = value_after(line, "tool.version=")) current.m_version = *value;
		}

		flush();
		return result;
	}

	inline void save_cache(const build::request& req, const cache& cache_data) {
		const auto path = cache_path(req);

		if (path.has_parent_path()) {
			std::filesystem::create_directories(path.parent_path());
		}

		std::ofstream out(path);

		if (!out.is_open()) {
			return;
		}

		out << "mgmake-tool-cache-v1\n\n";

		for (const auto& entry : cache_data.m_entries) {
			const auto host_key = entry.m_host_key.empty()
				? target_key(entry.m_host)
				: entry.m_host_key;
			const auto target_key_value = entry.m_target_key.empty()
				? target_key(entry.m_target)
				: entry.m_target_key;

			out << "toolchain=" << entry.m_toolchain << "\n";
			out << "host=" << host_key << "\n";
			out << "target=" << target_key_value << "\n";
			out << "tool.role=" << name(entry.m_role) << "\n";
			out << "tool.logical=" << entry.m_logical_name << "\n";
			out << "tool.path=" << entry.m_path.generic_string() << "\n";
			out << "tool.provider=" << name(entry.m_provider) << "\n";
			out << "tool.version=" << entry.m_version << "\n\n";
		}
	}
}

#endif // MGMAKE_DISCOVERY_CACHE_HXX
