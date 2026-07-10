#pragma once

#ifndef MGMAKE_CLI_DEFAULT_OPTIONS_HXX
#define MGMAKE_CLI_DEFAULT_OPTIONS_HXX

#include "option.hxx"
#include "options.hxx"

#include "../meta/type_list.hxx"

#include <print>

namespace mgmake::cli {
    using help_option = option
        ::name<"help">::short_name<'h'>
        ::description<"Show help.">
		// Parsing parses action first, flags next
		// this will override the requested action & make mgmake use the help action instead
		::callback<[](options& opts){
			opts.m_action = action::kind::help;
		}>
        //::assign<meta::member_access<&options::m_action>, action::kind::help>
		::build;

    // Type list of default options
    // this way you can add your own option to default_options
    // before passing the list to the entry for your own CLI
    // options
    using default_options = meta::type_list<
        help_option
    >;
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX