#pragma once

#ifndef MGMAKE_CLI_ENTRY_HXX
#define MGMAKE_CLI_ENTRY_HXX

#include "config.hxx"
#include "dispatcher.hxx"
#include "options.hxx"
#include "parser.hxx"

#include "../sys/exit_code.hxx"
#include "../sys/shell.hxx"

#include <print>
#include <utility>

namespace mgmake::cli {
    template<typename config_t = config>
    inline sys::exit_code entry(sys::shell cmd) {
        // construct the parser & dispatcher at compile time :)
		using d = dispatcher<config_t>;
        using p = parser<typename config_t::options_type>;

        // parse cmd at runtime
        if (auto result = p::template parse<d>(cmd)) {
			auto opts = result.value();
			d::invoke(cmd, opts);
            return sys::exit_code::success;
        } else {
            std::println(stderr, "{}", result.error());
            return sys::exit_code::usage_error;
        }
    }

    template<typename config_t = config>
    inline sys::exit_code entry(int argc, char* argv[]) {
        return entry<config_t>(sys::shell::from_args(argc, argv));
    }
}

// Pass a `cli::config` type with your project
#define MGMK_ENTRY(...) \
int main(int argc, char* argv[]) { \
    return std::to_underlying(::mgmake::cli::entry<__VA_ARGS__>(argc, argv)); \
}

#endif // MGMAKE_CLI_ENTRY_HXX