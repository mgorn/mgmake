#pragma once

#ifndef MGMAKE_DAG_ACTION_HXX
#define MGMAKE_DAG_ACTION_HXX

#include "artifact.hxx"

#include "../sys/command_line.hxx"

#include <filesystem>
#include <string>
#include <vector>

namespace mgmake::dag {
    struct action {
        using id = std::vector<action>::size_type;

        std::string m_name;
        std::string m_description;

        std::vector<artifact::id> m_inputs;
        std::vector<artifact::id> m_outputs;

        bool m_always_run = false; // always_fun = false :(

        sys::command_line m_command;
        std::filesystem::path m_working_directory;
    };
}

#endif