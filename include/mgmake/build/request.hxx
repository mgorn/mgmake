#pragma once

#ifndef MGMAKE_BUILD_REQUEST_HXX
#define MGMAKE_BUILD_REQUEST_HXX

#include "../discovery/resolved_tool.hxx"
#include "../discovery/tool_environment.hxx"
#include "toolchain.hxx"
#include "../sys/platform.hxx"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace mgmake::build {
    struct request {
        toolchain m_tc;
        std::filesystem::path m_build_dir;
        std::vector<std::string> m_targets; // Which targets to build, empty = build all
        sys::target m_target = sys::g_host_target;
        std::vector<discovery::resolved_tool> m_discovered_tools{};
        discovery::tool_environment m_tool_environment{};

        [[nodiscard]] inline constexpr const toolchain& toolchain() const {
            return m_tc;
        }
        inline constexpr auto& toolchain(struct toolchain& tc) {
            m_tc = tc;
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
            for (const auto& tool : m_discovered_tools) {
                if (tool.m_role == role) {
                    return &tool;
                }
            }

            return nullptr;
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

        [[nodiscard]] inline const discovery::tool_environment& tool_environment() const noexcept {
            return m_tool_environment;
        }
    };
}

#endif
