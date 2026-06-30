#pragma once

#ifndef MGMAKE_BUILD_REQUEST_HXX
#define MGMAKE_BUILD_REQUEST_HXX

#include "../cli/backend.hxx"
#include "../discovery/resolved_toolchain.hxx"
#include "toolchain.hxx"
#include "../sys/platform.hxx"

#include <filesystem>
#include <initializer_list>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// A build request is the normalized execution context created from CLI options and tool discovery.
// Later phases read this instead of re-parsing user options.

namespace mgmake::build {
    struct request {
        toolchain m_tc;
        cli::backend_kind m_backend = cli::backend_kind::automatic;
        std::filesystem::path m_build_dir;
        std::vector<std::string> m_targets; // Which targets to build, empty = build all
        sys::target m_target = sys::g_host_target;
        std::optional<discovery::resolved_toolchain> m_resolved_toolchain{};
        bool m_discover_source_dependencies = false;

        [[nodiscard]] inline constexpr const toolchain& toolchain() const {
            return m_tc;
        }
        inline constexpr auto& toolchain(struct toolchain& tc) {
            m_tc = tc;
            return *this;
        }

        [[nodiscard]] inline constexpr cli::backend_kind backend() const noexcept {
            return m_backend;
        }

        inline constexpr auto& backend(cli::backend_kind value) noexcept {
            m_backend = value;
            return *this;
        }

        [[nodiscard]] inline constexpr const std::filesystem::path& build_dir() const {
            return m_build_dir;
        }
        inline constexpr auto& build_dir(const std::filesystem::path dir) {
            m_build_dir = dir;
            return *this;
        }

        [[nodiscard]] inline const sys::target& target() const noexcept {
            return m_target;
        }

        inline auto& target(sys::target value) {
            m_target = std::move(value);
            return *this;
        }

        [[nodiscard]] inline constexpr sys::platform target_platform() const noexcept {
            return m_target.m_platform;
        }

        inline constexpr auto& target_platform(sys::platform value) noexcept {
            m_target.m_platform = value;
            return *this;
        }

        [[nodiscard]] inline const discovery::resolved_tool* discovered_tool(
            discovery::tool_role role
        ) const noexcept {
            if (!m_resolved_toolchain.has_value()) {
                return nullptr;
            }

            return m_resolved_toolchain->find(role);
        }

        [[nodiscard]] inline std::filesystem::path tool_path(
            discovery::tool_role role,
            std::filesystem::path fallback = {}
        ) const {
            if (const auto* tool = discovered_tool(role)) {
                return tool->m_path;
            }

            return fallback;
        }

        [[nodiscard]] inline const discovery::resolved_tool* discovered_tool_any(
            std::initializer_list<discovery::tool_role> roles
        ) const noexcept {
            if (!m_resolved_toolchain.has_value()) {
                return nullptr;
            }

            return m_resolved_toolchain->find_any(roles);
        }

        [[nodiscard]] inline std::filesystem::path tool_path_any(
            std::initializer_list<discovery::tool_role> roles,
            std::filesystem::path fallback = {}
        ) const {
            if (const auto* tool = discovered_tool_any(roles)) {
                return tool->m_path;
            }

            return fallback;
        }

        [[nodiscard]] inline const discovery::tool_environment& tool_environment() const noexcept {
            static const discovery::tool_environment empty{};

            if (!m_resolved_toolchain.has_value()) {
                return empty;
            }

            return m_resolved_toolchain->m_environment;
        }

        [[nodiscard]] inline const std::vector<std::string>& compile_prefix_args() const noexcept {
            static const std::vector<std::string> empty{};

            if (!m_resolved_toolchain.has_value()) {
                return empty;
            }

            return m_resolved_toolchain->m_compile_prefix_args;
        }

        [[nodiscard]] inline const std::vector<std::string>& link_prefix_args() const noexcept {
            static const std::vector<std::string> empty{};

            if (!m_resolved_toolchain.has_value()) {
                return empty;
            }

            return m_resolved_toolchain->m_link_prefix_args;
        }

        inline const void discover_source_dependencies(bool value) noexcept {
            m_discover_source_dependencies = value;
        }
        [[nodiscard]] inline const bool discover_source_dependencies() const noexcept {
            return m_discover_source_dependencies;
        }
    };
}

#endif // MGMAKE_BUILD_REQUEST_HXX
