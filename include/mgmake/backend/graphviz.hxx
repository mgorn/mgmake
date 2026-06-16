#pragma once

#ifndef MGMAKE_BACKEND_GRAPHVIZ_HXX
#define MGMAKE_BACKEND_GRAPHVIZ_HXX

#include "../build/request.hxx"
#include "../cli/options.hxx"
#include "../dag/artifact.hxx"
#include "../dag/graph.hxx"

#include <filesystem>
#include <fstream>

namespace mgmake::backend {
    namespace detail {
        inline std::string dot_escape(std::string_view text) {
            std::string result;

            for (const char ch : text) {
                switch (ch) {
                    case '\\':
                        result += "\\\\";
                        break;

                    case '"':
                        result += "\\\"";
                        break;

                    case '\n':
                        result += "\\n";
                        break;

                    case '\r':
                        break;

                    default:
                        result += ch;
                        break;
                }
            }

            return result;
        }

        inline std::string dot_label(const std::filesystem::path& path) {
            return dot_escape(path.generic_string());
        }

        inline std::string artifact_shape(dag::artifact::kind kind) {
            switch (kind) {
                case dag::artifact::kind::source:
                    return "box";

                case dag::artifact::kind::generated:
                    return "component";

                case dag::artifact::kind::phony:
                    return "note";
            }

            return "box";
        }

        inline std::string artifact_kind_name(dag::artifact::kind kind) {
            switch (kind) {
                case dag::artifact::kind::source:
                    return "source";

                case dag::artifact::kind::generated:
                    return "generated";

                case dag::artifact::kind::phony:
                    return "phony";
            }

            return "artifact";
        }
    }

    template<bool show_commands = true, bool show_action_ids = true, bool show_artifact_ids = true, bool show_targets = true>
    struct graphviz {
        std::filesystem::path m_output_file = "graph.dot";

        void generate(const cli::options&, const dag::graph& graph, const build::request& req) const {
            auto output_path = req.build_dir() / m_output_file;
            if (output_path.has_parent_path()) {
                std::filesystem::create_directories(output_path.parent_path());
            }
            std::ofstream out(output_path);

            out << "digraph mgmake {\n";
            out << "    rankdir=LR;\n";
            out << "    graph [fontname=\"monospace\"];\n";
            out << "    node [fontname=\"monospace\"];\n";
            out << "    edge [fontname=\"monospace\"];\n\n";

            for (auto i = 0; i < graph.m_artifacts.size(); ++i) {
                const auto& artifact = graph.artifact(i);

                std::string label;

                if constexpr (show_artifact_ids) {
                    label += "artifact ";
                    label += std::to_string(i);
                    label += "\\n";
                }

                label += detail::artifact_kind_name(artifact.m_kind);
                label += "\\n";
                label += detail::dot_label(artifact.m_path);

                out << "    artifact_" << i
                    << " [shape=" << detail::artifact_shape(artifact.m_kind)
                    << ", label=\"" << label << "\"];\n";
            }

            out << "\n";

            for (auto i = 0; i < graph.m_actions.size(); ++i) {
                const auto& action = graph.action(i);

                std::string label;

                if (show_action_ids) {
                    label += "action ";
                    label += std::to_string(i);
                    label += "\\n";
                }

                if (!action.m_name.empty()) {
                    label += detail::dot_escape(action.m_name);
                } else {
                    label += "unnamed action";
                }

                if (!action.m_description.empty()) {
                    label += "\\n";
                    label += detail::dot_escape(action.m_description);
                }

                if (show_commands && !action.m_command.m_args.empty()) {
                    label += "\\n";

                    for (std::size_t arg_i = 0; arg_i < action.m_command.m_args.size(); ++arg_i) {
                        if (arg_i != 0) {
                            label += " ";
                        }

                        label += detail::dot_escape(action.m_command.m_args[arg_i]);
                    }
                }

                out << "    action_" << i
                    << " [shape=ellipse, label=\"" << label << "\"];\n";
            }

            out << "\n";

            if constexpr (show_targets) {
                for (auto i = 0; i < graph.m_targets.size(); ++i) {
                    const auto& target = graph.target(i);

                    std::string label = "target";
                    label += "\\n";
                    label += detail::dot_escape(target.m_name);

                    out << "    target_" << i
                        << " [shape=house, label=\"" << label << "\"];\n";
                }

                out << "\n";
            }

            for (auto i = 0; i < graph.m_actions.size(); ++i) {
                const auto& action = graph.action(i);

                for (const auto input : action.m_inputs) {
                    out << "    artifact_" << input << " -> action_" << i << ";\n";
                }

                for (const auto output : action.m_outputs) {
                    out << "    action_" << i << " -> artifact_" << output << ";\n";
                }
            }

            if constexpr (show_targets) {
                out << "\n";

                for (auto i = 0; i < graph.m_targets.size(); ++i) {
                    const auto& target = graph.target(i);

                    for (const auto output : target.m_outputs) {
                        out << "    target_" << i << " -> artifact_" << output << ";\n";
                    }
                }
            }

            out << "}\n";
        }
    };
}

#endif
