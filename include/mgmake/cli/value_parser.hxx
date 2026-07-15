#pragma once

#ifndef MGMAKE_CLI_VALUE_PARSER_HXX
#define MGMAKE_CLI_VALUE_PARSER_HXX

#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>

// Value parsers convert one option argument string into a typed destination value.

namespace mgmake::cli {
	template<typename type_t>
	struct value_parser {
		// Hint for the value type in help menu
		static inline constexpr std::string_view help_hint = "value";
	};

	template<> struct value_parser<std::string> {
		static inline constexpr std::string_view help_hint = "text";

		[[nodiscard]] static std::expected<std::string, std::string> parse(std::string_view text) {
			return std::string{ text };
		}
	};

	template<> struct value_parser<int> {
		static inline constexpr std::string_view help_hint = "integer";

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
		static inline constexpr std::string_view help_hint = "path";

		[[nodiscard]] static std::expected<std::filesystem::path, std::string> parse(std::string_view text) {
			std::filesystem::path path{ text };
			if (path.is_absolute()) {
				return path;
			}
			return std::filesystem::current_path() / path;
		}
	};
}

#endif // MGMAKE_CLI_VALUE_PARSER_HXX
