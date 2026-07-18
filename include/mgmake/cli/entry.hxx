#pragma once

#ifndef MGMAKE_CLI_ENTRY_HXX
#define MGMAKE_CLI_ENTRY_HXX

#include "parser.hxx"

#include "../sys/exit_code.hxx"
#include "../sys/shell.hxx"
#include "../task/dispatcher.hxx"
#include "../config.hxx"

#include <print>
#include <utility>

namespace mgmake::cli {
    template<typename config_builder_t = config>
    inline sys::exit_code entry(sys::shell cmd) {
		// Finalize the given config
		using config_type = config_builder_t::build;
		using opt_storage_type = typename config_type::option_storage_type;

        // construct the parser & dispatcher at compile time :)
		using d = task::dispatcher<config_type>;
        using p = parser<opt_storage_type>;

        // parse cmd at runtime
        if (auto parse_result = p::template parse<d>(cmd)) {
			auto opts = parse_result.value();

			if (auto dispatch_result = d::invoke(cmd, opts)) {
                return dispatch_result.value();
            } else {
                std::println(stderr, "{}", dispatch_result.error());
                return sys::exit_code::task_failure;
            }
        } else {
            std::println(stderr, "{}", parse_result.error());
            return sys::exit_code::usage_error;
        }
    }

    template<typename config_builder_t = config>
    inline sys::exit_code entry(int argc, char* argv[]) {
        return entry<config_builder_t>(sys::shell::from_args(argc, argv));
    }
}

// Pass a `cli::config` type with your project
#define MGMK_ENTRY(...) \
int main(int argc, char* argv[]) { \
    return std::to_underlying(::mgmake::cli::entry<__VA_ARGS__>(argc, argv)); \
}

#endif // MGMAKE_CLI_ENTRY_HXX