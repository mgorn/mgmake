#pragma once

#ifndef MGMAKE_CLI_VALUE_PARSER_HXX
#define MGMAKE_CLI_VALUE_PARSER_HXX

#include <expected>
#include <format>
#include <string>
#include <string_view>

// Value parsers convert one option argument string into a typed destination value.

namespace mgmake::cli {
	template<typename type_t>
	struct value_parser;

	template<> struct value_parser<std::string> {
		[[nodiscard]] static std::expected<std::string, std::string> parse(std::string_view text) {
			return std::string{ text };
		}
	};

	template<> struct value_parser<int> {
		[[nodiscard]] static std::expected<int, std::string> parse(std::string_view text) {
			if (text.empty()) {
				return std::unexpected(std::format("invalid integer value '{}' (empty)", text));
			}

			try {
				// Why can't std::stoi take a string_view???
				return std::stoi(std::string{ text });
				// Why does the alternative `std::from_chars` return a `std::from_chars_result` instead of a `std::expected` or something??
			} catch (...) {}
			return std::unexpected(std::format("invalid integer value '{}'", text));
		}
	};

	template<> struct value_parser<std::filesystem::path> {
		[[nodiscard]] static std::expected<std::filesystem::path, std::string> parse(std::string_view text) {
			return std::filesystem::path{ text };
		}
	};
}

#endif // MGMAKE_CLI_VALUE_PARSER_HXX
