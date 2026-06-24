#pragma once

#ifndef MGMAKE_DEP_PARSER_HXX
#define MGMAKE_DEP_PARSER_HXX

#include "../ext/json.hxx"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Parsers intentionally target compiler-generated dependency files, not general Makefiles or arbitrary JSON.

namespace mgmake::dep {
	namespace detail {
		[[nodiscard]] inline std::string collapse_make_continuations(
			std::string text
		) {
			std::string result;

			for (std::size_t i = 0; i < text.size(); ++i) {
				if (text[i] == '\\' && i + 1 < text.size() && text[i + 1] == '\n') {
					result.push_back(' ');
					++i;
					continue;
				}

				if (
					text[i] == '\\' &&
					i + 2 < text.size() &&
					text[i + 1] == '\r' &&
					text[i + 2] == '\n'
				) {
					result.push_back(' ');
					i += 2;
					continue;
				}

				result.push_back(text[i]);
			}

			return result;
		}

		[[nodiscard]] inline std::size_t find_first_unescaped_colon(
			std::string_view text
		) {
			bool escaped = false;

			for (std::size_t i = 0; i < text.size(); ++i) {
				const char ch = text[i];

				if (escaped) {
					escaped = false;
					continue;
				}

				if (ch == '\\') {
					escaped = true;
					continue;
				}

				if (ch == ':') {
					return i;
				}
			}

			return std::string_view::npos;
		}

		[[nodiscard]] inline std::vector<std::string> split_make_words(
			std::string_view text
		) {
			std::vector<std::string> result;
			std::string current;
			bool escaped = false;

			for (const char ch : text) {
				if (escaped) {
					current.push_back(ch);
					escaped = false;
					continue;
				}

				if (ch == '\\') {
					escaped = true;
					continue;
				}

				if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
					if (!current.empty()) {
						result.emplace_back(std::move(current));
						current.clear();
					}

					continue;
				}

				current.push_back(ch);
			}

			if (!current.empty()) {
				result.emplace_back(std::move(current));
			}

			return result;
		}
	}

	[[nodiscard]] inline std::optional<std::string> read_text_file(
		const std::filesystem::path& path
	) {
		std::ifstream in(path, std::ios::binary);

		if (!in.is_open()) {
			return std::nullopt;
		}

		return std::string{
			std::istreambuf_iterator<char>{in},
			std::istreambuf_iterator<char>{}
		};
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> parse_gcc_make_depfile(
		std::string_view content
	) {
		std::vector<std::filesystem::path> result;

		const auto flat = detail::collapse_make_continuations(std::string{content});

		// This parser is intentionally scoped to compiler-generated depfiles.
		// GCC/Clang usually escape Windows drive colons as C\:/..., which makes
		// this target/dependency separator search work for normal mgmake output.
		// It is not intended to be a complete Makefile parser.
		const auto colon = detail::find_first_unescaped_colon(flat);

		if (colon == std::string_view::npos) {
			return result;
		}

		const auto deps_text = std::string_view{flat}.substr(colon + 1);

		for (const auto& word : detail::split_make_words(deps_text)) {
			if (word.empty()) {
				continue;
			}

			// GCC/Clang -MP emits empty dummy rules like:
			// include/foo.hxx:
			// Those are useful for Make/Ninja behavior, but they are not dependency
			// entries mgmake should materialize as header artifacts.
			if (word.ends_with(":")) {
				continue;
			}

			result.emplace_back(word);
		}

		return result;
	}

	[[nodiscard]] inline std::vector<std::filesystem::path>
	parse_msvc_source_dependencies(std::string_view content) {
		std::vector<std::filesystem::path> result;

		const auto parsed = ext::json::parse(content);

		if (!parsed.has_value()) {
			return result;
		}

		const auto data = parsed->get("Data");

		if (!data.has_value()) {
			return result;
		}

		// MSVC /sourceDependencies records the primary source separately.
		// Lowering filters already-explicit action inputs after parsing.
		if (const auto source = data->get("Source")) {
			if (const auto path = source->as_string()) {
				result.emplace_back(*path);
			}
		}

		// Ordinary header dependencies.
		for (const auto& include : data->array("Includes")) {
			if (const auto path = include.as_string()) {
				result.emplace_back(*path);
			}
		}

		// Header units can name real header files as well. Capture the known
		// Header field, but avoid recursively treating arbitrary strings as paths.
		for (const auto& header_unit : data->array("ImportedHeaderUnits")) {
			const auto header = header_unit.get("Header");

			if (!header.has_value()) {
				continue;
			}

			if (const auto path = header->as_string()) {
				result.emplace_back(*path);
			}
		}

		return result;
	}
}

#endif // MGMAKE_DEP_PARSER_HXX
