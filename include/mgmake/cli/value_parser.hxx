#pragma once

#ifndef MGMAKE_CLI_VALUE_PARSER_HXX
#define MGMAKE_CLI_VALUE_PARSER_HXX

#include "backend.hxx"
#include "util.hxx"

#include <format>
#include <string>
#include <string_view>

// Value parsers convert one option argument string into a typed destination value.

namespace mgmake::cli {
	template <typename T>
	struct value_parser;

	template <>
	struct value_parser<std::string> {
		[[nodiscard]] static bool parse(std::string_view text, std::string& out) {
			out = std::string{ text };
			return true;
		}

		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format("invalid string value '{}'", text);
		}
	};

	template <>
	struct value_parser<int> {
		[[nodiscard]] static bool parse(std::string_view text, int& out) {
			return parse_positive_int(text, out);
		}

		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format("invalid integer value '{}'", text);
		}
	};
}

#endif // MGMAKE_CLI_VALUE_PARSER_HXX
