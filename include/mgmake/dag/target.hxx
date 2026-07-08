#pragma once

#ifndef MGMAKE_DAG_TARGET_HXX
#define MGMAKE_DAG_TARGET_HXX

#include "artifact.hxx"

#include <set>
#include <string>
#include <vector>

// A DAG target names a user-visible build goal and points at the artifacts/actions that satisfy it.

namespace mgmake::dag {
    struct target {
        using id = std::vector<target>::size_type;

        std::string m_name;

        // Empty is valid for interface/no-op/metadata targets
        std::set<artifact::id> m_outputs;

        // Other DAG targets this target conceptually depends on.
        // For now this mostly exists so graphviz/ninja can show target-level deps.
        std::set<target::id> m_dependencies;

        void add_output(const artifact::id art) {
			m_outputs.emplace(art);
		}

		void add_dependency(const target::id dep) {
			m_dependencies.emplace(dep);
		}

        const std::string_view name() const {
            return m_name;
        }

        const std::set<artifact::id>& outputs() const {
            return m_outputs;
        }

        const std::set<target::id>& dependencies() const {
            return m_dependencies;
        }
    };
}

#endif // MGMAKE_DAG_TARGET_HXX