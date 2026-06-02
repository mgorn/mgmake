#pragma once

#ifndef MGMAKE_DAG_TARGET_HXX
#define MGMAKE_DAG_TARGET_HXX

#include "artifact.hxx"

#include <string>
#include <vector>

namespace mgmake::dag {
    struct target {
        using id = std::vector<target>::size_type;

        std::string m_name;

        // Empty is valid for interface/no-op/metadata targets
        std::vector<artifact::id> m_outputs;

        // Other DAG targets this target conceptually depends on.
        // For now this mostly exists so graphviz/ninja can show target-level deps.
        std::vector<target::id> m_dependencies;
    };
}

#endif