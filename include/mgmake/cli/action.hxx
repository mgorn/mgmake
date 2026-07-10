#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

#include "../meta/type_list.hxx"

namespace mgmake::cli {
    struct action {
        enum struct kind : uint8_t {
            // meta actions, no cli required
            help,
            version,

            // normal actions, require cli
            tools,
            clean,
            generate,
            build,
            run,
            graph,

            // # of action kinds std::to_underlying(kind::count)
            count
        };
    };
}

#endif // MGMAKE_CLI_ACTION_HXX