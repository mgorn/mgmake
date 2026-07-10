#pragma once

#ifndef MGMAKE_CLI_ENTRY_HXX
#define MGMAKE_CLI_ENTRY_HXX

#include "default_options.hxx"
#include "options.hxx"
#include "parser.hxx"

#include "../sys/exit_code.hxx"
#include "../sys/shell.hxx"

#include <print>
#include <utility>

namespace mgmake::cli {
    template<auto project_v = nullptr, auto toolchains_v = nullptr, typename options_t = default_options>
    inline sys::exit_code entry(sys::shell cmd) {
        // construct the parser at compile time :)
        using p = parser<options_t>;

        // parse cmd at runtime
        if (auto result = p::parse(cmd)) {
            return sys::exit_code::success;
        } else {
            std::println(stderr, "{}", result.error());
            return sys::exit_code::usage_error;
        }
    }

    template<auto project_v = nullptr, auto toolchains_v = nullptr, typename options_t = default_options>
    inline sys::exit_code entry(int argc, char* argv[]) {
        return entry<project_v, toolchains_v, options_t>(sys::shell::from_args(argc, argv));
    }
}

#define MGMK_ENTRY(...) \
int main(int argc, char* argv[]) { \
    return std::to_underlying(::mgmake::cli::entry<__VA_ARGS__>(argc, argv)); \
}

#endif // MGMAKE_CLI_ENTRY_HXX