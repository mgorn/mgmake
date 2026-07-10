#pragma once

#ifndef MGMAKE_CLI_UTIL_HXX
#define MGMAKE_CLI_UTIL_HXX

#include <charconv>
#include <string_view>

// Small CLI helpers stay separate so parsers can share spelling and prefix checks.

namespace mgmake::cli {
	[[nodiscard]] inline constexpr bool is_option(std::string_view arg) {
		return arg.size() >= 2 && arg[0] == '-';
	}

}

#endif // MGMAKE_CLI_UTIL_HXX
