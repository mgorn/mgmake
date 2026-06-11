#pragma once

#ifndef MGMAKE_BUILD_REQUEST_HXX
#define MGMAKE_BUILD_REQUEST_HXX

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
    };
}

#endif
