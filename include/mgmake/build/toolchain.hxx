#pragma once

#ifndef MGMAKE_BUILD_TOOLCHAIN_HXX
#define MGMAKE_BUILD_TOOLCHAIN_HXX

#include "../discovery/mode.hxx"
#include "../discovery/tool_binding.hxx"
#include "../discovery/tool_role.hxx"
#include "../sys/platform.hxx"

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Toolchain is the user-facing description of compiler dialect, tool names, flags, roots, and target behavior.
// Tool discovery may rewrite tool names to absolute resolved paths in build::request.

namespace mgmake::build {
    struct toolchain {
        // Determines how MGMake should pass args to the compiler driver(s)
        enum struct dialect {
            gcc, // GCC-like command arguments (typically with a '-' dash)
            msvc // Windows-like arguments (typically with a '/' slash)
        };

        enum struct target_mode {
            implicit,
            clang_target,
            custom
        };

        std::string m_name; // The name of the toolchain
        dialect m_dialect = []{
#ifdef MGMK_PLATFORM_WINDOWS
            return dialect::msvc;
#else
            return dialect::gcc;
#endif // MGMK_PLATFORM_WINDOWS
        }();
        target_mode m_target_mode = target_mode::implicit;

        std::string m_cc;
        std::string m_cxx;
        std::string m_ar;
        std::string m_linker;
        std::vector<discovery::tool_binding> m_tools{};

        std::optional<std::string> m_version{};
        std::optional<std::string> m_sdk_root{};
        std::optional<std::string> m_toolchain_root{};
        std::optional<std::string> m_package_root{};
        std::vector<std::string> m_search_roots{};
        discovery::mode m_discovery_mode = discovery::mode::exact;

        std::vector<std::string> m_compile_flags;
        std::vector<std::string> m_c_flags;
        std::vector<std::string> m_cxx_flags;
        std::vector<std::string> m_archive_flags;
        std::vector<std::string> m_link_flags;

        std::optional<std::string> m_target_triple;
        std::optional<std::string> m_sysroot;

       [[nodiscard]] inline constexpr std::string_view name() const {
            return m_name;
        }
        inline constexpr auto& name(std::string_view str) {
            m_name = str;
            return *this;
        }

        [[nodiscard]] inline constexpr enum dialect dialect() const noexcept {
            return m_dialect;
        }
        inline constexpr auto& dialect(enum dialect value) noexcept {
            m_dialect = value;
            return *this;
        }

        [[nodiscard]] inline constexpr target_mode target_selection() const noexcept {
            return m_target_mode;
        }
        inline constexpr auto& target_selection(target_mode mode) noexcept {
            m_target_mode = mode;
            return *this;
        }

        // cc
        [[nodiscard]] inline constexpr const std::string& cc() const noexcept {
            return m_cc;
        }
        inline constexpr auto& cc(std::string path) {
            m_cc = std::move(path);
            return *this;
        }

        // cxx
        [[nodiscard]] inline constexpr const std::string& cxx() const noexcept {
            return m_cxx;
        }
        inline constexpr auto& cxx(std::string path) {
            m_cxx = std::move(path);
            return *this;
        }

        // ar
        [[nodiscard]] inline constexpr const std::string& ar() const noexcept {
            return m_ar;
        }
        inline constexpr auto& ar(std::string path) {
            m_ar = std::move(path);
            return *this;
        }

        // linker
        [[nodiscard]] inline constexpr const std::string& linker() const noexcept {
            return m_linker;
        }
        inline constexpr auto& linker(std::string path) {
            m_linker = std::move(path);
            return *this;
        }

        inline constexpr auto& tool(discovery::tool_role role, std::string name) {
            for (auto& binding : m_tools) {
                if (binding.m_role == role) {
                    binding.m_name = std::move(name);
                    return *this;
                }
            }

            m_tools.push_back(discovery::tool_binding{
                .m_role = role,
                .m_name = std::move(name)
            });

            return *this;
        }

        [[nodiscard]] inline constexpr std::string_view tool(
            discovery::tool_role role
        ) const noexcept {
            for (const auto& binding : m_tools) {
                if (binding.m_role == role) {
                    return binding.m_name;
                }
            }

            switch (role) {
                case discovery::tool_role::c_compiler:
                    return m_cc;

                case discovery::tool_role::cxx_compiler:
                    return m_cxx;

                case discovery::tool_role::archiver:
                    return m_ar;

                case discovery::tool_role::linker:
                    return m_linker;

                default:
                    return {};
            }
        }

        inline constexpr auto& assembler(std::string name) {
            return tool(discovery::tool_role::assembler, std::move(name));
        }

        inline constexpr auto& ranlib(std::string name) {
            return tool(discovery::tool_role::ranlib, std::move(name));
        }

        inline constexpr auto& librarian(std::string name) {
            return tool(discovery::tool_role::librarian, std::move(name));
        }

        inline constexpr auto& shared_linker(std::string name) {
            return tool(discovery::tool_role::shared_linker, std::move(name));
        }

        inline constexpr auto& resource_compiler(std::string name) {
            return tool(discovery::tool_role::resource_compiler, std::move(name));
        }

        inline constexpr auto& manifest_tool(std::string name) {
            return tool(discovery::tool_role::manifest_tool, std::move(name));
        }

        inline constexpr auto& dll_tool(std::string name) {
            return tool(discovery::tool_role::dll_tool, std::move(name));
        }

        inline constexpr auto& strip(std::string name) {
            return tool(discovery::tool_role::strip, std::move(name));
        }

        inline constexpr auto& objcopy(std::string name) {
            return tool(discovery::tool_role::objcopy, std::move(name));
        }

        inline constexpr auto& objdump(std::string name) {
            return tool(discovery::tool_role::objdump, std::move(name));
        }

        inline constexpr auto& nm(std::string name) {
            return tool(discovery::tool_role::nm, std::move(name));
        }

        inline constexpr auto& readelf(std::string name) {
            return tool(discovery::tool_role::readelf, std::move(name));
        }

        inline constexpr auto& cmake(std::string name) {
            return tool(discovery::tool_role::cmake, std::move(name));
        }

        inline constexpr auto& pkg_config(std::string name) {
            return tool(discovery::tool_role::pkg_config, std::move(name));
        }

        inline constexpr auto& discovery_mode(discovery::mode value) noexcept {
            m_discovery_mode = value;
            return *this;
        }

        [[nodiscard]] inline constexpr discovery::mode discovery_mode() const noexcept {
            return m_discovery_mode;
        }

        inline constexpr auto& sdk_root(std::string path) {
            m_sdk_root = std::move(path);
            return *this;
        }

        inline constexpr auto& toolchain_root(std::string path) {
            m_toolchain_root = std::move(path);
            return *this;
        }

        inline constexpr auto& package_root(std::string path) {
            m_package_root = std::move(path);
            return *this;
        }

        inline constexpr auto& add_search_root(std::string path) {
            m_search_roots.emplace_back(std::move(path));
            return *this;
        }

        // compile_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& compile_flags() const noexcept {
            return m_compile_flags;
        }
        inline constexpr auto& compile_flags(std::vector<std::string> flags) {
            m_compile_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& compile_flags(std::initializer_list<std::string_view> flags) {
            m_compile_flags.clear();

            for (std::string_view flag : flags) {
                m_compile_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_compile_flag(std::string_view flag) {
            m_compile_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_compile_flags() noexcept {
            m_compile_flags.clear();
            return *this;
        }

        // c_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& c_flags() const noexcept {
            return m_c_flags;
        }
        inline constexpr auto& c_flags(std::vector<std::string> flags) {
            m_c_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& c_flags(std::initializer_list<std::string_view> flags) {
            m_c_flags.clear();

            for (std::string_view flag : flags) {
                m_c_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_c_flag(std::string_view flag) {
            m_c_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_c_flags() noexcept {
            m_c_flags.clear();
            return *this;
        }

        // cxx_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& cxx_flags() const noexcept {
            return m_cxx_flags;
        }
        inline constexpr auto& cxx_flags(std::vector<std::string> flags) {
            m_cxx_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& cxx_flags(std::initializer_list<std::string_view> flags) {
            m_cxx_flags.clear();

            for (std::string_view flag : flags) {
                m_cxx_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_cxx_flag(std::string_view flag) {
            m_cxx_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_cxx_flags() noexcept {
            m_cxx_flags.clear();
            return *this;
        }

        // archive_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& archive_flags() const noexcept {
            return m_archive_flags;
        }
        inline constexpr auto& archive_flags(std::vector<std::string> flags) {
            m_archive_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& archive_flags(std::initializer_list<std::string_view> flags) {
            m_archive_flags.clear();

            for (std::string_view flag : flags) {
                m_archive_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_archive_flag(std::string_view flag) {
            m_archive_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_archive_flags() noexcept {
            m_archive_flags.clear();
            return *this;
        }

        // link_flags
        [[nodiscard]] inline constexpr const std::vector<std::string>& link_flags() const noexcept {
            return m_link_flags;
        }
        inline constexpr auto& link_flags(std::vector<std::string> flags) {
            m_link_flags = std::move(flags);
            return *this;
        }
        inline constexpr auto& link_flags(std::initializer_list<std::string_view> flags) {
            m_link_flags.clear();

            for (std::string_view flag : flags) {
                m_link_flags.emplace_back(flag);
            }

            return *this;
        }
        inline constexpr auto& add_link_flag(std::string_view flag) {
            m_link_flags.emplace_back(flag);
            return *this;
        }
        inline constexpr auto& clear_link_flags() noexcept {
            m_link_flags.clear();
            return *this;
        }

        // target_triple
        [[nodiscard]] inline constexpr const std::optional<std::string>& target_triple() const noexcept {
            return m_target_triple;
        }
        inline constexpr auto& target_triple(std::string triple) {
            m_target_triple = std::move(triple);
            return *this;
        }
        inline constexpr auto& clear_target_triple() noexcept {
            m_target_triple.reset();
            return *this;
        }

        // sysroot
        [[nodiscard]] inline constexpr const std::optional<std::string>& sysroot() const noexcept {
            return m_sysroot;
        }
        inline constexpr auto& sysroot(std::string path) {
            m_sysroot = std::move(path);
            return *this;
        }
        inline constexpr auto& sysroot(std::optional<std::string> path) {
            m_sysroot = std::move(path);
            return *this;
        }
        inline constexpr auto& clear_sysroot() noexcept {
            m_sysroot.reset();
            return *this;
        }
    };

    inline const auto tc_clang_mg = build::toolchain{"clang-mg"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("clang-mg")
        .cxx("clang-mg++")
        .ar("llvm-ar")
        .ranlib("llvm-ranlib")
        .linker("clang-mg++")
        .shared_linker("clang-mg++")
        .resource_compiler("llvm-rc")
        .strip("llvm-strip")
        .objcopy("llvm-objcopy")
        .objdump("llvm-objdump")
        .nm("llvm-nm")
        .readelf("llvm-readelf")
        .target_selection(build::toolchain::target_mode::clang_target)
        .discovery_mode(discovery::mode::family_fallback);

    inline const auto tc_clang = build::toolchain{"clang"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("clang")
        .cxx("clang++")
        .ar("llvm-ar")
        .ranlib("llvm-ranlib")
        .linker("clang++")
        .shared_linker("clang++")
        .resource_compiler("llvm-rc")
        .strip("llvm-strip")
        .objcopy("llvm-objcopy")
        .objdump("llvm-objdump")
        .nm("llvm-nm")
        .readelf("llvm-readelf")
        .target_selection(build::toolchain::target_mode::clang_target)
        .discovery_mode(discovery::mode::family_fallback);

    inline const auto tc_gcc = build::toolchain{"gcc"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("gcc")
        .cxx("g++")
        .ar("ar")
        .ranlib("ranlib")
        .linker("g++")
        .shared_linker("g++")
        .resource_compiler("windres")
        .strip("strip")
        .objcopy("objcopy")
        .objdump("objdump")
        .nm("nm")
        .readelf("readelf")
        .discovery_mode(discovery::mode::family_fallback);
    
    inline const auto tc_msvc = build::toolchain{"msvc"}
        .dialect(build::toolchain::dialect::msvc)
        .cc("cl")
        .cxx("cl")
        .ar("lib")
        .librarian("lib")
        .linker("link")
        .shared_linker("link")
        .resource_compiler("rc")
        .manifest_tool("mt")
        .tool(discovery::tool_role::debug_symbol_tool, "mspdbsrv")
        .discovery_mode(discovery::mode::family_fallback);
}

#endif // MGMAKE_BUILD_TOOLCHAIN_HXX
