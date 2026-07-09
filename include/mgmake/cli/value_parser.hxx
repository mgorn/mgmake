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
	template <typename type_t>
	struct value_parser;

	template <>
	struct value_parser<std::string> {
		[[nodiscard]] static std::expected<std::string,std::string> parse(std::string_view text) {
			return std::string{ text };
		}
	};

	template <>
	struct value_parser<int> {
		[[nodiscard]] static std::expected<int,std::string> parse(std::string_view text) {
			if (text.empty()) {
				return std::unexpected(std::format("invalid integer value '{}'", text));
			}

			try {
				return std::stoi(text);
			} catch (...) {}
			return std::unexpected(std::format("invalid integer value '{}'", text));
		}
	};
}

#endif // MGMAKE_CLI_VALUE_PARSER_HXX
