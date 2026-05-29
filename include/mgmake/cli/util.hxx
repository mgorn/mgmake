#pragma once

#ifndef MGMAKE_CLI_UTIL_HXX
#define MGMAKE_CLI_UTIL_HXX

#include <charconv>
#include <cstdint>
#include <format>
#include <print>
#include <span>
#include <string>
#include <string_view>

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

	[[nodiscard]] inline constexpr bool consume_value(
		std::span<const std::string> args,
		std::size_t& index,
		std::string_view option_name,
		std::string& out,
		std::string& error
	) {
		if (index + 1 >= args.size()) {
			error = std::format("missing value after '{}'", option_name);
			return false;
		}

		++index;
		out = args[index];
		return true;
	}

	[[nodiscard]] inline constexpr bool is_option(std::string_view arg) {
		return arg.size() >= 2 && arg[0] == '-';
	}

	inline void print_help(std::string_view program_name) {
		if (program_name.empty()) {
			program_name = "mgmake";
		}

		std::println("usage:");
		std::println("  {} [command] [options] [targets...] [-- passthrough...]", program_name);
		std::println("");
		std::println("commands:");
		std::println("  build       Build the project. This is the default command.");
		std::println("  generate    Generate backend build files.");
		std::println("  clean       Remove generated build output.");
		std::println("  run         Build and run a target.");
		std::println("  help        Show this help text.");
		std::println("  version     Show version information.");
		std::println("");
		std::println("options:");
		std::println("  --backend <name>       Backend to use: auto, ninja, make, direct.");
		std::println("  --build-dir <path>     Build directory. Default: build.");
		std::println("  --target <name>        Target to build. Can be passed multiple times.");
		std::println("  -j, --jobs <count>     Number of parallel jobs.");
		std::println("  -v, --verbose          Print more detailed output.");
		std::println("  --dry-run              Print what would happen without doing it.");
		std::println("  -h, --help             Show this help text.");
		std::println("  --version              Show version information.");
	}
}

#endif