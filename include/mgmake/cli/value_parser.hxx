#pragma once

#ifndef MGMAKE_CLI_VALUE_PARSER_HXX
#define MGMAKE_CLI_VALUE_PARSER_HXX

#include "../meta/static_string.hxx"

#include <charconv>
#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <vector>

// Value parsers convert one option argument string into a typed destination value.

namespace mgmake::cli {
	template<typename type_t>
	struct value_parser {
		// Hint for the value type in help menu
		static inline constexpr meta::static_string help_hint = "value";
	};

	template<> struct value_parser<std::string> {
		static inline constexpr meta::static_string help_hint = "string";

		[[nodiscard]] static std::expected<std::string, std::string> parse(std::string_view text) {
			return std::string{ text };
		}
	};

	template<> struct value_parser<int> {
		static inline constexpr meta::static_string help_hint = "integer";

		[[nodiscard]] static std::expected<int, std::string> parse(std::string_view text) {
			if (text.empty()) {
				return std::unexpected(std::format("invalid integer value '{}' (empty)", text));
			}

			int value = 0;
			auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
			if (error != std::errc{} or end != text.data() + text.size()) {
				return std::unexpected(std::format("invalid integer value '{}'", text));
			}
			return value;
		}
	};

	template<> struct value_parser<std::filesystem::path> {
		static inline constexpr meta::static_string help_hint = "path";

		[[nodiscard]] static std::expected<std::filesystem::path, std::string> parse(std::string_view text) {
			std::filesystem::path path{ text };
			if (path.is_absolute()) {
				return path;
			}
			return std::filesystem::current_path() / path;
		}
	};

	template<typename type_t> struct value_parser<std::vector<type_t>> {
		using item_parser = value_parser<type_t>;
		static inline constexpr meta::static_string help_hint = item_parser::help_hint + meta::static_string{"s..."};

		[[nodiscard]] static std::expected<type_t, std::string> parse(std::string_view text) {
			return item_parser::parse(text);
		}
	};
}

#endif // MGMAKE_CLI_VALUE_PARSER_HXX
