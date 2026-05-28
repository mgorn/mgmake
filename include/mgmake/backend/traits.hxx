#pragma once

#ifndef MGMAKE_BACKEND_TRAITS_HXX
#define MGMAKE_BACKEND_TRAITS_HXX

#include "../dag/graph.hxx"

namespace mgmake::backend {
    // Generates graph output (graph.dot, build.ninja)
    trait generator {
        void generate(const dag::graph& graph);
    };
    // Actually builds the program from the graph (invokes compiler, runs ninja)
    trait builder {
        void build(const dag::graph& graph);
    };
}

#endif