#pragma once

#ifndef MGMAKE_BACKEND_NINJA_HXX
#define MGMAKE_BACKEND_NINJA_HXX

#include "../dag/graph.hxx"

namespace mgmake::backend {
    struct ninja {
        void generate(const dag::graph& graph);
        void build(const dag::graph& graph);
    };
}

#endif