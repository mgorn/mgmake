#pragma once

#ifndef MGMAKE_DAG_ACTION_HXX
#define MGMAKE_DAG_ACTION_HXX

#include "artifact.hxx"

#include "../dep/file.hxx"
#include "../sys/command_line.hxx"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// A DAG action is an edge: it consumes input artifacts, produces output artifacts, and carries the command to run.

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

        // Dependencies discovered from an existing compiler dependency file.
        // These are mgmake graph metadata, not backend explicit inputs.
        std::vector<artifact::id> m_discovered_dependencies;

        // Compile actions can emit dependency side files. Other action kinds leave this empty.
        std::optional<dep::file> m_depfile;
    };
}

#endif // MGMAKE_DAG_ACTION_HXX