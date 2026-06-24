#pragma once

#ifndef MGMAKE_CLI_HELP_HXX
#define MGMAKE_CLI_HELP_HXX

#include "parse.hxx"
#include "../build/toolchain_registry.hxx"

#include <print>
#include <string_view>

// Help output is generated from the same parser definitions used by cli::parse.

namespace mgmake::cli {
	template <build::toolchain_registry_like Toolchains>
	inline void print_help(
		std::string_view program_name,
		const Toolchains& toolchains
	) {
		print_help(program_name);

		std::println("");
		std::println("Available toolchains:");
		std::println("  {}", toolchains.choices_string());
	}
}

#endif // MGMAKE_CLI_HELP_HXX
