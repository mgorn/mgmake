#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "option.hxx"

#include "../meta/type_list.hxx"

#include <print>

namespace mgmake::cli {
    using help_option = option
        ::name<"help">::short_name<'h'>
        ::description<"Show help.">
        ::callback<[](auto& opts) {
            // TODO: Print help menu
            std::println("TODO: Restore Help");
        }>;

    // Type list of default options
    // this way you can add your own option to default_options
    // before passing the list to the entry for your own CLI
    // options
    using default_options = meta::type_list<
        help_option
    >;
}

#endif // MGMAKE_CLI_OPTIONS_HXX