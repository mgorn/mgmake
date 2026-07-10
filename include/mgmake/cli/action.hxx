#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

#include <cstdint>

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
            build, // default (see options.hxx)
            run,
            graph,

            // # of action kinds std::to_underlying(kind::count)
            count
        };
    };
}

#endif // MGMAKE_CLI_ACTION_HXX