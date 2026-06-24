#pragma once

#ifndef MGMAKE_CLI_UTIL_HXX
#define MGMAKE_CLI_UTIL_HXX

#include <charconv>
#include <string_view>

// Small CLI helpers stay separate so parsers can share spelling and prefix checks.

namespace mgmake::cli {
	[[nodiscard]] inline constexpr bool parse_positive_int(std::string_view text, int& out) {
		if (text.empty()) {
			return false;
		}

		int value = 0;

		const char* first = text.data();
		const char* last = text.data() + text.size();

		auto [ptr, ec] = std::from_chars(first, last, value);

		if (ec != std::errc{} || ptr != last || value <= 0) {
			return false;
		}

		out = value;
		return true;
	}

	[[nodiscard]] inline constexpr bool is_option(std::string_view arg) {
		return arg.size() >= 2 && arg[0] == '-';
	}

}

#endif // MGMAKE_CLI_UTIL_HXX
