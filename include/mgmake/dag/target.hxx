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
        std::vector<artifact::id> m_outputs;
    };
}

#endif