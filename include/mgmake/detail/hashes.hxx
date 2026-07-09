#pragma once

#ifndef MGMK_DETAIL_HASHES_HXX
#define MGMK_DETAIL_HASHES_HXX

#include "../build/request.hxx"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>

// Artifact hashes are stored in the build directory and used to detect changed inputs across invocations.

namespace mgmake::detail {
	struct hashes {
		using hash_type = std::uint64_t;
		using file_type = std::filesystem::path;
		using map_type = std::unordered_map<file_type, hash_type>;

		map_type m_cache;
		map_type m_live;

		[[nodiscard]] inline static std::filesystem::path cache_path(
			const build::request& req
		) {
			return req.build_dir() / "mgmake" / "hashes.tsv";
		}

		[[nodiscard]] inline static hashes load(const build::request& req) {
			hashes result{};
			result.m_cache = load_file(cache_path(req));
			return result;
		}

		// A missing cached hash means mgmake has not seen this artifact before.
		[[nodiscard]] inline bool is_dirty(const file_type& file) const {
			if (!exists(file)) {
				return true;
			}

			if (!m_cache.contains(file)) {
				return true;
			}

			const auto hash = hash_file(file);

			if (!hash) {
				return true;
			}

			const auto cached = m_cache.at(file);
			return cached != hash.value();
		}

		[[nodiscard]] inline bool check(const file_type& file) {
			if (!exists(file)) {
				m_live.erase(file);
				return true;
			}

			const auto hash = hash_file(file);

			if (!hash) {
				m_live.erase(file);
				return true;
			}

			m_live[file] = hash.value();

			if (!m_cache.contains(file)) {
				return true;
			}

			const auto cached = m_cache.at(file);
			return cached != hash.value();
		}

		inline void update(const file_type& file) {
			if (!exists(file)) {
				m_live.erase(file);
				return;
			}

			const auto hash = hash_file(file);

			if (!hash) {
				m_live.erase(file);
				return;
			}

			m_live[file] = hash.value();
		}

		inline void store(const build::request& req) const {
			store_file(cache_path(req), m_live);
		}

		[[nodiscard]] inline static std::optional<hash_type> hash_file(
			const file_type& file
		) {
			auto file = fs::open<std::ios::binary>(file);

			return file.transform([&](auto& input) {
				constexpr hash_type offset_basis = 14695981039346656037ull;
				constexpr hash_type prime = 1099511628211ull;

				hash_type result = offset_basis;
				std::array<char, 64 * 1024> buffer{};

				while (input) {
					input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
					const auto count = input.gcount();

					for (std::streamsize i = 0; i < count; ++i) {
						result ^= static_cast<unsigned char>(buffer[static_cast<std::size_t>(i)]);
						result *= prime;
					}
				}

				return result;
			});
		}

		[[nodiscard]] inline static map_type load_file(
			const std::filesystem::path& path
		) {
			map_type result{};
			std::ifstream input{path};

			if (!input) {
				return result;
			}

			std::string tag;
			input >> tag;

			if (tag != "mgmake-hashes") {
				return result;
			}

			int version = 0;
			input >> version;

			if (version != 1) {
				return result;
			}

			hash_type hash{};
			std::string path_string;

			while (input >> hash >> std::quoted(path_string)) {
				result.emplace(file_type{path_string}, hash);
			}

			return result;
		}

		inline static void store_file(
			const std::filesystem::path& path,
			const map_type& hashes
		) {
			const auto parent = path.parent_path();

			if (!parent.empty()) {
				std::filesystem::create_directories(parent);
			}

			std::ofstream output{path, std::ios::trunc};

			if (!output) {
				return;
			}

			output << "mgmake-hashes\t1\n";

			for (const auto& [file, hash] : hashes) {
				output << hash << '\t' << std::quoted(file.string()) << '\n';
			}
		}

	private:
		[[nodiscard]] inline static bool exists(const file_type& file) {
			std::error_code ec;
			return std::filesystem::exists(file, ec);
		}
	};
}

#endif // MGMK_DETAIL_HASHES_HXX
