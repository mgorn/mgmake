// This file is generated. Do not edit directly.
// Source: include/mgmake/mgmake.hxx

#ifndef MGMAKE_SINGLE_HEADER_HXX
#define MGMAKE_SINGLE_HEADER_HXX


// ===== begin include/mgmake/mgmake.hxx =====
#pragma once

#ifndef MGMAKE_MGMAKE_HXX
#define MGMAKE_MGMAKE_HXX


// ===== begin include/mgmake/backend/graphviz.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_GRAPHVIZ_HXX
#define MGMAKE_BACKEND_GRAPHVIZ_HXX


// ===== begin include/mgmake/build/request.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_REQUEST_HXX
#define MGMAKE_BUILD_REQUEST_HXX


// ===== begin include/mgmake/build/toolchain.hxx =====
#pragma once

#ifndef MGMAKE_BUILD_TOOLCHAIN_HXX
#define MGMAKE_BUILD_TOOLCHAIN_HXX

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::build {
    struct toolchain {
        // Determines how MGMake should pass args to the compiler driver(s)
        enum struct dialect {
            gcc, // GCC-like command arguments (typically with a '-' dash)
            msvc // Windows-like arguments (typically with a '/' slash)
        };

        std::string m_name; // The name of the toolchain
        dialect m_dialect = []{
#ifdef MGMAKE_PLATFORM_WINDOWS
            return dialect::msvc;
#else
            return dialect::gcc;
#endif
        }();

        std::string m_cc;
        std::string m_cxx;
        std::string m_ar;
        std::string m_linker;

        std::vector<std::string> m_compile_flags;
        std::vector<std::string> m_c_flags;
        std::vector<std::string> m_cxx_flags;
        std::vector<std::string> m_archive_flags;
        std::vector<std::string> m_link_flags;

        std::optional<std::string> m_arch_triple;
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

        // arch_triple
        [[nodiscard]] inline constexpr const std::optional<std::string>& arch_triple() const noexcept {
            return m_arch_triple;
        }
        inline constexpr auto& arch_triple(std::string_view triple) {
            m_arch_triple = std::string { triple };
            return *this;
        }
        inline constexpr auto& arch_triple(std::optional<std::string> triple) {
            m_arch_triple = std::move(triple);
            return *this;
        }
        inline constexpr auto& clear_arch_triple() noexcept {
            m_arch_triple.reset();
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

    static constexpr auto tc_clang_mg = build::toolchain{"clang-mg"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("clang-mg")
        .cxx("clang-mg++")
        .ar("llvm-ar")
        .linker("clang-mg++");

    static constexpr auto tc_clang = build::toolchain{"clang"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("clang")
        .cxx("clang++")
        .ar("llvm-ar")
        .linker("clang++");

    static constexpr auto tc_gcc = build::toolchain{"gcc"}
        .dialect(build::toolchain::dialect::gcc)
        .cc("gcc")
        .cxx("g++")
        .ar("ar")
        .linker("g++");
    
    static constexpr auto tc_msvc = build::toolchain{"msvc"}
        .dialect(build::toolchain::dialect::msvc)
        .cc("cl")
        .cxx("cl")
        .ar("lib")
        .linker("link");
}

#endif// ===== end include/mgmake/build/toolchain.hxx =====


#include <filesystem>
#include <vector>

namespace mgmake::build {
    struct request {
        toolchain m_tc;
        std::filesystem::path m_build_dir;
        std::vector<std::string> m_targets; // Which targets to build, empty = build all

        inline constexpr const toolchain& toolchain() const {
            return m_tc;
        }
        inline constexpr auto& toolchain(struct toolchain& tc) {
            m_tc = tc;
            return *this;
        }

        inline constexpr const std::filesystem::path& build_dir() const {
            return m_build_dir;
        }
        inline constexpr auto& build_dir(const std::filesystem::path dir) {
            m_build_dir = dir;
            return *this;
        }
    };
}

#endif// ===== end include/mgmake/build/request.hxx =====


// ===== begin include/mgmake/dag/artifact.hxx =====
#pragma once

#ifndef MGMAKE_DAG_ARTIFACT_HXX
#define MGMAKE_DAG_ARTIFACT_HXX

#include <filesystem>
#include <vector>

namespace mgmake::dag {
    struct artifact {
        using id = std::vector<artifact>::size_type;

        enum struct kind {
            source, // A source code (.cxx, .c, etc)
            generated, // Generated by another program
            phony // Fake placeholder
        } m_kind = kind::source;
        std::filesystem::path m_path;
    };
}

#endif// ===== end include/mgmake/dag/artifact.hxx =====


// ===== begin include/mgmake/dag/graph.hxx =====
#pragma once

#ifndef MGMAKE_DAG_GRAPH_HXX
#define MGMAKE_DAG_GRAPH_HXX


// ===== begin include/mgmake/detail/assert.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_ASSERT_HXX
#define MGMAKE_DETAIL_ASSERT_HXX

#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string_view>

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

namespace mgmake::detail {

inline void debug_break() {
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__clang__) || defined(__GNUC__)
    __builtin_trap();
#else
    std::abort();
#endif
}

[[noreturn]] inline void assertion_failed(
    std::string_view condition,
    std::string_view message,
    std::source_location location = std::source_location::current()
) {
    std::fprintf(
        stderr,
        "\nmgmake assertion failed\n"
        "  condition: %.*s\n"
        "  message:   %.*s\n"
        "  location:  %s:%u:%u\n"
        "  function:  %s\n\n",
        static_cast<int>(condition.size()),
        condition.data(),
        static_cast<int>(message.size()),
        message.data(),
        location.file_name(),
        location.line(),
        location.column(),
        location.function_name()
    );

    debug_break();

    // In case the platform/debugger allows execution to continue.
    std::abort();
}

inline void mgmk_assert_impl(
    bool condition,
    std::string_view condition_text,
    std::string_view message,
    std::source_location location = std::source_location::current()
) {
    if (!condition) {
        assertion_failed(condition_text, message, location);
    }
}

} // namespace mgmake::detail

#ifndef MGMK_ENABLE_ASSERTS
    #ifndef NDEBUG
        #define MGMK_ENABLE_ASSERTS 1
    #else
        #define MGMK_ENABLE_ASSERTS 0
    #endif
#endif

#if MGMK_ENABLE_ASSERTS
    #define mgmkassert(condition, message)                                      \
        do {                                                                    \
            ::mgmake::detail::mgmk_assert_impl(                                 \
                static_cast<bool>(condition),                                   \
                #condition,                                                     \
                message,                                                        \
                std::source_location::current()                                 \
            );                                                                  \
        } while (false)
#else
    #define mgmkassert(condition, message)                                      \
        do {                                                                    \
            (void)sizeof(condition);                                            \
        } while (false)
#endif

#endif// ===== end include/mgmake/detail/assert.hxx =====


// ===== begin include/mgmake/dag/action.hxx =====
#pragma once

#ifndef MGMAKE_DAG_ACTION_HXX
#define MGMAKE_DAG_ACTION_HXX

// skipped duplicate include: include/mgmake/dag/artifact.hxx


// ===== begin include/mgmake/sys/command_line.hxx =====
#pragma once

#ifndef MGMAKE_SYS_COMMAND_LINE_HXX
#define MGMAKE_SYS_COMMAND_LINE_HXX


// ===== begin include/mgmake/detail/convert.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_CONVERT_HXX
#define MGMAKE_DETAIL_CONVERT_HXX


// ===== begin include/mgmake/sys/platform.hxx =====
#pragma once

#ifndef MGMAKE_SYS_PLATFORM_HXX
#define MGMAKE_SYS_PLATFORM_HXX

// Cursed windows shit here
#if defined(_WIN32) 
    // You can still use MGMake on Windows without windows.h <3
    // with love - Michael
    #ifndef MGMK_NO_WINDOWS
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif

        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif

        #include <windows.h>
	    #pragma message("Windows is included here. This is probably the source of your pain.")
        #define MGMK_INCLUDED_WINDOWS
    #endif

    #define MGMK_PLATFORM_WINDOWS 1
#elif defined(__unix__) || defined(__APPLE__)
    #define MGMK_PLATFORM_POSIX 1
#else
    #define MGMK_PLATFORM_UNSUPPORTED 1
#endif

namespace mgmake::sys {
	enum struct platform {
		windows,
		posix,
		unsupported
	};

	static constexpr platform g_host_platform = [] constexpr {
#ifdef MGMK_PLATFORM_WINDOWS
		return platform::windows;
#elifdef MGMK_PLATFORM_POSIX
		return platform::posix;
#else
		return platform::unsupported;
#endif
	}();
}

#endif// ===== end include/mgmake/sys/platform.hxx =====


namespace mgmake::detail {
#ifdef MGMK_INCLUDED_WINDOWS
	inline constexpr std::string wide_to_utf8(std::wstring_view text) {
		if (text.empty()) {
			return {};
		}

		mgmkassert(text.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()), "Wide string is too large to convert to UTF-8");

		int wide_size = static_cast<int>(text.size());

		int utf8_size = WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			text.data(),
			wide_size,
			nullptr,
			0,
			nullptr,
			nullptr
		);

		mgmkassert(utf8_size > 0, "Failed to calculate UTF-8 argument size");

		std::string result;
		result.resize(static_cast<std::size_t>(utf8_size));

		int written = WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			text.data(),
			wide_size,
			result.data(),
			utf8_size,
			nullptr,
			nullptr
		);

		mgmkassert(written > 0, "Failed to convert command line argument to UTF-8");

		return result;
	}
#endif
}

#endif// ===== end include/mgmake/detail/convert.hxx =====


// ===== begin include/mgmake/sys/util.hxx =====
#pragma once

#ifndef MGMAKE_SYS_UTIL_HXX
#define MGMAKE_SYS_UTIL_HXX

// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <string>
#include <string_view>

namespace mgmake::sys {
#ifdef MGMK_PLATFORM_WINDOWS
	inline constexpr std::string shell_escape(std::string_view arg) {
		if (arg.empty()) {
			return "\"\"";
		}

		bool needs_quotes = false;

		for (const char ch : arg) {
			if (ch == ' ' || ch == '\t' || ch == '"' || ch == '\\') {
				needs_quotes = true;
				break;
			}
		}

		if (!needs_quotes) {
			return std::string(arg);
		}

		std::string result;
		result += '"';

		std::size_t backslashes = 0;

		for (const char ch : arg) {
			if (ch == '\\') {
				++backslashes;
				continue;
			}

			if (ch == '"') {
				result.append(backslashes * 2 + 1, '\\');
				result += '"';
				backslashes = 0;
				continue;
			}

			if (backslashes != 0) {
				result.append(backslashes, '\\');
				backslashes = 0;
			}

			result += ch;
		}

		if (backslashes != 0) {
			result.append(backslashes * 2, '\\');
		}

		result += '"';
		return result;
	}
#else
	inline constexpr std::string shell_escape(std::string_view arg) {
		if (arg.empty()) {
			return "''";
		}

		bool needs_quotes = false;

		for (const char ch : arg) {
			if (ch == ' ' || ch == '\t' || ch == '\'' || ch == '"' || ch == '$' || ch == '\\' || ch == '&' || ch == ';' || ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '|') {
				needs_quotes = true;
				break;
			}
		}

		if (!needs_quotes) {
			return std::string(arg);
		}

		std::string result;
		result += '\'';

		for (const char ch : arg) {
			if (ch == '\'') {
				result += "'\\''";
			} else {
				result += ch;
			}
		}

		result += '\'';
		return result;
	}
#endif
}

#endif// ===== end include/mgmake/sys/util.hxx =====


#include <cstdlib>
#include <span>
#include <string_view>
#include <vector>

namespace mgmake::sys {
	struct command_line {
		std::vector<std::string> m_args;

		inline constexpr std::string_view program_name() const {
			return m_args.empty() ? std::string_view{} : std::string_view(m_args[0]);
		}

		inline constexpr std::span<const std::string> user_args() const {
			if (m_args.size() <= 1) {
				return {};
			}

			return std::span<const std::string>(m_args).subspan(1);
		}

		inline constexpr std::string full_command() const {
			std::string result;

            for (std::size_t i = 0; i < m_args.size(); ++i) {
                if (i != 0) {
                    result += ' ';
                }

                result += sys::shell_escape(m_args[i]);
            }

            return result;
		}

		auto invoke() const {
			return std::system(full_command().c_str());
		}
	};

	inline constexpr command_line args_from_utf8(int argc, const char** argv) {
		command_line result;

		for (int i = 0; i < argc; ++i) {
			result.m_args.emplace_back(argv[i] ? argv[i] : "");
		}

		return result;
	}

#ifdef MGMK_INCLUDED_WINDOWS
	inline constexpr command_line args_from_wide(int argc, const wchar_t** argv) {
		command_line result;
		if (argc <= 0 || argv == nullptr) {
			return result;
		}
		result.m_args.reserve(static_cast<std::size_t>(argc));
		for (int i = 0; i < argc; ++i) {
			if (argv[i] == nullptr) {
				result.m_args.emplace_back();
				continue;
			}
			result.m_args.emplace_back(detail::wide_to_utf8(argv[i]));
		}
		return result;
	}
#endif
}

#endif// ===== end include/mgmake/sys/command_line.hxx =====


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

#endif// ===== end include/mgmake/dag/action.hxx =====

// skipped duplicate include: include/mgmake/dag/artifact.hxx

// ===== begin include/mgmake/dag/target.hxx =====
#pragma once

#ifndef MGMAKE_DAG_TARGET_HXX
#define MGMAKE_DAG_TARGET_HXX

// skipped duplicate include: include/mgmake/dag/artifact.hxx

#include <set>
#include <string>
#include <vector>

namespace mgmake::dag {
    struct target {
        using id = std::vector<target>::size_type;

        std::string m_name;

        // Empty is valid for interface/no-op/metadata targets
        std::set<artifact::id> m_outputs;

        // Other DAG targets this target conceptually depends on.
        // For now this mostly exists so graphviz/ninja can show target-level deps.
        std::set<target::id> m_dependencies;

		void add_dependency(const target::id dep) {
			m_dependencies.emplace(dep);
		}
    };
}

#endif// ===== end include/mgmake/dag/target.hxx =====


#include <utility>
#include <vector>

namespace mgmake::dag {
    struct graph {
        std::vector<artifact> m_artifacts;
        std::vector<action> m_actions;
        std::vector<target> m_targets;

        inline constexpr artifact::id create_artifact(auto&&... args) {
            m_artifacts.emplace_back(std::forward<decltype(args)>(args)...);
            return { m_artifacts.size() - 1 };
        }
        inline constexpr action::id create_action(auto&&... args) {
            m_actions.emplace_back(std::forward<decltype(args)>(args)...);
            return { m_actions.size() - 1 };
        }
        inline constexpr target::id create_target(auto&&... args) {
            m_targets.emplace_back(std::forward<decltype(args)>(args)...);
            return { m_targets.size() - 1 };
        }

		inline constexpr struct artifact& artifact(const artifact::id id) {
			mgmkassert(not m_artifacts.empty(), "Invalid artifact ID: there are no artifacts.");
			mgmkassert(id < m_artifacts.size(), "Invalid artifact ID");
			return m_artifacts.at(id);
		}
		inline constexpr const struct artifact& artifact(const artifact::id id) const {
			mgmkassert(not m_artifacts.empty(), "Invalid artifact ID: there are no artifacts.");
			mgmkassert(id < m_artifacts.size(), "Invalid artifact ID");
			return m_artifacts.at(id);
		}
		inline constexpr struct action& action(const action::id id) {
			mgmkassert(not m_actions.empty(), "Invalid action ID: there are no actions.");
			mgmkassert(id < m_actions.size(), "Invalid action ID");
			return m_actions.at(id);
		}
		inline constexpr const struct action& action(const action::id id) const {
			mgmkassert(not m_actions.empty(), "Invalid action ID: there are no actions.");
			mgmkassert(id < m_actions.size(), "Invalid action ID");
			return m_actions.at(id);
		}
		inline constexpr struct target& target(const target::id id) {
			mgmkassert(not m_targets.empty(), "Invalid target ID: there are no targets.");
			mgmkassert(id < m_targets.size(), "Invalid target ID");
			return m_targets.at(id);
		}
		inline constexpr const struct target& target(const target::id id) const {
			mgmkassert(not m_targets.empty(), "Invalid target ID: there are no targets.");
			mgmkassert(id < m_targets.size(), "Invalid target ID");
			return m_targets.at(id);
		}
    };
}

#endif// ===== end include/mgmake/dag/graph.hxx =====


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

        void generate(const dag::graph& graph, const build::request& req) const {
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

#endif// ===== end include/mgmake/backend/graphviz.hxx =====


// ===== begin include/mgmake/backend/ninja.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_NINJA_HXX
#define MGMAKE_BACKEND_NINJA_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx
// skipped duplicate include: include/mgmake/sys/util.hxx

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>

namespace mgmake::backend {
    namespace detail {
        inline std::string ninja_escape_build_text(std::string_view text) {
            std::string result;

            for (const char ch : text) {
                switch (ch) {
                    case '$':
                        result += "$$";
                        break;

                    case ' ':
                        result += "$ ";
                        break;

                    case ':':
                        result += "$:";
                        break;

                    case '|':
                        result += "$|";
                        break;

                    case '\n':
                        result += "$\n";
                        break;

                    default:
                        result += ch;
                        break;
                }
            }

            return result;
        }

        inline std::string ninja_escape_path(const std::filesystem::path& path) {
            return ninja_escape_build_text(path.generic_string());
        }

        inline std::string ninja_escape_variable_text(std::string_view text) {
            std::string result;

            for (const char ch : text) {
                switch (ch) {
                    case '$':
                        result += "$$";
                        break;

                    case '\n':
                        result += "$\n";
                        break;

                    default:
                        result += ch;
                        break;
                }
            }

            return result;
        }

        inline void write_artifact_list(std::ofstream& out, const dag::graph& graph, const std::vector<dag::artifact::id>& artifacts) {
            bool first = true;

            for (const auto id : artifacts) {
                const auto& artifact = graph.artifact(id);

                if (!first) {
                    out << ' ';
                }

                out << ninja_escape_path(artifact.m_path);
                first = false;
            }
        }
		inline void write_artifact_list(std::ofstream& out, const dag::graph& graph, const std::set<dag::artifact::id>& artifacts) {
			return write_artifact_list(out, graph, artifacts | std::ranges::to<std::vector<dag::artifact::id>>());
		}

        inline void create_output_directories(const dag::graph& graph) {
            for (const auto& action : graph.m_actions) {
                for (const auto output_id : action.m_outputs) {
                    const auto& output = graph.artifact(output_id);

                    if (output.m_kind == dag::artifact::kind::phony) {
                        continue;
                    }

                    const auto parent = output.m_path.parent_path();

                    if (!parent.empty()) {
                        std::filesystem::create_directories(parent);
                    }
                }
            }
        }

        inline void write_target_defaults(std::ofstream& out, const dag::graph& graph) {
            if (graph.m_targets.empty()) {
                return;
            }

            out << "\ndefault";

            for (const auto& target : graph.m_targets) {
                out << ' ' << ninja_escape_build_text(target.m_name);
            }

            out << "\n";
        }
    }

    struct ninja {
        std::filesystem::path m_output_file = "build.ninja";

        void generate(const dag::graph& graph, const build::request& req) const {
            auto output_path = req.build_dir() / m_output_file;
            if (output_path.has_parent_path()) {
                std::filesystem::create_directories(output_path.parent_path());
            }
            detail::create_output_directories(graph);

            std::ofstream out(output_path);
			mgmkassert(out.is_open(), "ninja backend: failed to open " + output_path.string());

            out << "# generated by mgmake\n\n";

            out << "builddir = "
                << detail::ninja_escape_variable_text(req.build_dir().generic_string())
                << "\n\n";

            bool needs_always = false;

            for (const auto& action : graph.m_actions) {
                if (action.m_always_run) {
                    needs_always = true;
                    break;
                }
            }

            if (needs_always) {
                out << "build __mgmake_always: phony\n\n";
            }

            for (auto i = 0; i < graph.m_actions.size(); ++i) {
                const auto& action = graph.action(i);

                mgmkassert(not action.m_outputs.empty(), "ninja backend: action '" + action.m_name + "' has no outputs");
				mgmkassert(not action.m_command.m_args.empty(), "ninja backend: action '" + action.m_name + "' has no command");

                out << "rule action_" << i << "\n";
                out << "  command = " << action.m_command.full_command() << "\n";

                if (!action.m_description.empty()) {
                    out << "  description = " << detail::ninja_escape_variable_text(action.m_description) << "\n";
                } else if (!action.m_name.empty()) {
                    out << "  description = " << detail::ninja_escape_variable_text(action.m_name) << "\n";
                }

                if (!action.m_working_directory.empty()) {
#if defined(_WIN32) // bruh
                    out << "  command = cd /d "
                        << detail::ninja_escape_variable_text(sys::shell_escape(action.m_working_directory.string()))
                        << " && "
                        << action.m_command.full_command()
                        << "\n";
#else
                    out << "  command = cd "
                        << detail::ninja_escape_variable_text(sys::shell_escape(action.m_working_directory.string()))
                        << " && "
                        << action.m_command.full_command()
                        << "\n";
#endif
                }

                out << "\n";

                out << "build ";
                detail::write_artifact_list(out, graph, action.m_outputs);
                out << ": action_" << i;

                if (!action.m_inputs.empty()) {
                    out << ' ';
                    detail::write_artifact_list(out, graph, action.m_inputs);
                }

                if (action.m_always_run) {
                    out << " | __mgmake_always";
                }

                out << "\n\n";
            }

            for (const auto& target : graph.m_targets) {
                out << "build " << detail::ninja_escape_build_text(target.m_name) << ": phony ";
                if (not target.m_outputs.empty()) {
                    out << ' ';
                    detail::write_artifact_list(out, graph, target.m_outputs);
                }
                for (const auto dep_id : target.m_dependencies) {
                    const auto& dep = graph.target(dep_id);
                    out << ' ' << detail::ninja_escape_build_text(dep.m_name);
                }
                out << "\n";
            }

            detail::write_target_defaults(out, graph);
        }

		std::expected<void, std::string> build(const dag::graph& graph, const build::request& req) const {
            auto output_path = req.build_dir() / m_output_file;
            if (output_path.has_parent_path()) {
                std::filesystem::create_directories(output_path.parent_path());
            }
			generate(graph, req);

			sys::command_line command;
			command.m_args.emplace_back("ninja");
			command.m_args.emplace_back("-f");
			command.m_args.emplace_back(output_path.string());

			const auto exit_code = command.invoke();

			if (exit_code != 0) {
				return std::unexpected(
					"ninja backend: ninja failed with exit code " + std::to_string(exit_code)
				);
			}

			return {};
		}
    };
}

#endif// ===== end include/mgmake/backend/ninja.hxx =====


// ===== begin include/mgmake/backend/traits.hxx =====
#pragma once

#ifndef MGMAKE_BACKEND_TRAITS_HXX
#define MGMAKE_BACKEND_TRAITS_HXX

// skipped duplicate include: include/mgmake/dag/graph.hxx

namespace mgmake::backend {
    // Generates graph output (graph.dot, build.ninja)
    trait generator {
        void generate(const dag::graph& graph, const build::request& req) const;
    };
    // Actually builds the program from the graph (invokes compiler, runs ninja)
    trait builder {
        void build(const dag::graph& graph, const build::request& req) const;
    };
}

#endif// ===== end include/mgmake/backend/traits.hxx =====

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/build/toolchain.hxx

// ===== begin include/mgmake/cli/action.hxx =====
#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

#include <string_view>

namespace mgmake::cli {
	enum struct action_kind {
		build,
		generate,
		clean,
		run,
		help,
		version
	};

	[[nodiscard]] inline constexpr std::string_view action_name(action_kind action) {
		switch (action) {
			case action_kind::build:
				return "build";
			case action_kind::generate:
				return "generate";
			case action_kind::clean:
				return "clean";
			case action_kind::run:
				return "run";
			case action_kind::help:
				return "help";
			case action_kind::version:
				return "version";
		}

		return "unknown";
	}

	[[nodiscard]] inline constexpr bool parse_action(std::string_view text, action_kind& out) {
		if (text == "build") {
			out = action_kind::build;
			return true;
		}

		if (text == "generate" || text == "gen") {
			out = action_kind::generate;
			return true;
		}

		if (text == "clean") {
			out = action_kind::clean;
			return true;
		}

		if (text == "run") {
			out = action_kind::run;
			return true;
		}

		if (text == "help") {
			out = action_kind::help;
			return true;
		}

		if (text == "version") {
			out = action_kind::version;
			return true;
		}

		return false;
	}
}

#endif// ===== end include/mgmake/cli/action.hxx =====


// ===== begin include/mgmake/cli/backend.hxx =====
#pragma once

#ifndef MGMAKE_CLI_BACKEND_HXX
#define MGMAKE_CLI_BACKEND_HXX

#include <string_view>

namespace mgmake::cli {
	enum struct backend_kind {
		automatic,
		graphviz,
		ninja,
		make,
		direct
	};

	[[nodiscard]] inline constexpr std::string_view backend_name(backend_kind backend) {
		switch (backend) {
			case backend_kind::automatic:
				return "automatic";
			case backend_kind::graphviz:
    			return "graphviz";
			case backend_kind::ninja:
				return "ninja";
			case backend_kind::make:
				return "make";
			case backend_kind::direct:
				return "direct";
		}

		return "unknown";
	}

	[[nodiscard]] inline constexpr bool parse_backend(std::string_view text, backend_kind& out) {
		if (text == "auto" || text == "automatic") {
			out = backend_kind::automatic;
			return true;
		}

		if (text == "graphviz" || text == "dot" || text == "graph") {
			out = backend_kind::graphviz;
			return true;
		}

		if (text == "ninja") {
			out = backend_kind::ninja;
			return true;
		}

		if (text == "make" || text == "makefile" || text == "makefiles") {
			out = backend_kind::make;
			return true;
		}

		if (text == "direct" || text == "compiler") {
			out = backend_kind::direct;
			return true;
		}

		return false;
	}
}

#endif// ===== end include/mgmake/cli/backend.hxx =====


// ===== begin include/mgmake/cli/options.hxx =====
#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

// skipped duplicate include: include/mgmake/cli/action.hxx
// skipped duplicate include: include/mgmake/cli/backend.hxx

#include <string>
#include <vector>

namespace mgmake::cli {
	struct options {
		action_kind m_action = action_kind::build;
		backend_kind m_backend = backend_kind::automatic;

		std::string m_build_dir = ".build";

		std::vector<std::string> m_targets;
		std::vector<std::string> m_passthrough_args;

		int m_jobs = 0;

		bool m_verbose = false;
		bool m_dry_run = false;
		bool m_show_help = false;
		bool m_show_version = false;
	};
}

#endif// ===== end include/mgmake/cli/options.hxx =====


// ===== begin include/mgmake/cli/parse.hxx =====
#pragma once

#ifndef MGMAKE_CLI_PARSE_HXX
#define MGMAKE_CLI_PARSE_HXX

// skipped duplicate include: include/mgmake/cli/options.hxx

// ===== begin include/mgmake/cli/util.hxx =====
#pragma once

#ifndef MGMAKE_CLI_UTIL_HXX
#define MGMAKE_CLI_UTIL_HXX

#include <charconv>
#include <cstdint>
#include <format>
#include <print>
#include <span>
#include <string>
#include <string_view>

namespace mgmake::cli {
	[[nodiscard]] inline constexpr bool parse_positive_int(std::string_view text, int& out) {
		if (text.empty()) {
			return false;
		}

		int value = 0;

		const char* first = text.data();
		const char* last = text.data() + text.size();

		auto [ptr, ec] = std::from_chars(first, last, value);

		if (ec != std::errc{} || ptr != last || value <= 0) {
			return false;
		}

		out = value;
		return true;
	}

	[[nodiscard]] inline constexpr bool consume_value(
		std::span<const std::string> args,
		std::size_t& index,
		std::string_view option_name,
		std::string& out,
		std::string& error
	) {
		if (index + 1 >= args.size()) {
			error = std::format("missing value after '{}'", option_name);
			return false;
		}

		++index;
		out = args[index];
		return true;
	}

	[[nodiscard]] inline constexpr bool is_option(std::string_view arg) {
		return arg.size() >= 2 && arg[0] == '-';
	}

	inline void print_help(std::string_view program_name) {
		if (program_name.empty()) {
			program_name = "mgmake";
		}

		std::println("usage:");
		std::println("  {} [command] [options] [targets...] [-- passthrough...]", program_name);
		std::println("");
		std::println("commands:");
		std::println("  build       Build the project. This is the default command.");
		std::println("  generate    Generate backend build files.");
		std::println("  clean       Remove generated build output.");
		std::println("  run         Build and run a target.");
		std::println("  help        Show this help text.");
		std::println("  version     Show version information.");
		std::println("");
		std::println("options:");
		std::println("  --backend <name>       Backend to use: auto, ninja, make, direct.");
		std::println("  --build-dir <path>     Build directory. Default: build.");
		std::println("  --target <name>        Target to build. Can be passed multiple times.");
		std::println("  -j, --jobs <count>     Number of parallel jobs.");
		std::println("  -v, --verbose          Print more detailed output.");
		std::println("  --dry-run              Print what would happen without doing it.");
		std::println("  -h, --help             Show this help text.");
		std::println("  --version              Show version information.");
	}
}

#endif// ===== end include/mgmake/cli/util.hxx =====


#include <format>
#include <vector>
#include <span>
#include <string>
#include <string_view>

namespace mgmake::cli {
	struct parse_result {
		bool m_ok = false;
		options m_value{};
		std::string m_error{};

		[[nodiscard]] operator bool() const {
			return m_ok;
		}
		[[nodiscard]] operator options() const {
			return m_value;
		}

		static parse_result success(options opts) {
			parse_result result;
			result.m_ok = true;
			result.m_value = std::move(opts);
			return result;
		}

		static parse_result failure(std::string message) {
			parse_result result;
			result.m_ok = false;
			result.m_error = std::move(message);
			return result;
		}
	};

	[[nodiscard]] inline constexpr parse_result parse(std::span<const std::string> args) {
		options opts;

		bool saw_first_positional = false;

		for (std::size_t i = 0; i < args.size(); ++i) {
			std::string_view arg = args[i];

			if (arg == "--") {
				for (++i; i < args.size(); ++i) {
					opts.m_passthrough_args.emplace_back(args[i]);
				}

				break;
			}

			if (arg == "-h" || arg == "--help") {
				opts.m_action = action_kind::help;
				opts.m_show_help = true;
				continue;
			}

			if (arg == "--version") {
				opts.m_action = action_kind::version;
				opts.m_show_version = true;
				continue;
			}

			if (arg == "-v" || arg == "--verbose") {
				opts.m_verbose = true;
				continue;
			}

			if (arg == "--dry-run") {
				opts.m_dry_run = true;
				continue;
			}

			if (arg == "--backend") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				backend_kind backend{};

				if (!parse_backend(value, backend)) {
					return parse_result::failure(
						std::format("unknown backend '{}'", value)
					);
				}

				opts.m_backend = backend;
				continue;
			}

			if (arg.starts_with("--backend=")) {
				std::string_view value = arg.substr(std::string_view("--backend=").size());

				backend_kind backend{};

				if (!parse_backend(value, backend)) {
					return parse_result::failure(
						std::format("unknown backend '{}'", value)
					);
				}

				opts.m_backend = backend;
				continue;
			}

			if (arg == "--build-dir") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				opts.m_build_dir = std::move(value);
				continue;
			}

			if (arg.starts_with("--build-dir=")) {
				std::string_view value = arg.substr(std::string_view("--build-dir=").size());

				if (value.empty()) {
					return parse_result::failure("missing value after '--build-dir='");
				}

				opts.m_build_dir = std::string(value);
				continue;
			}

			if (arg == "--target") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				opts.m_targets.emplace_back(std::move(value));
				continue;
			}

			if (arg.starts_with("--target=")) {
				std::string_view value = arg.substr(std::string_view("--target=").size());

				if (value.empty()) {
					return parse_result::failure("missing value after '--target='");
				}

				opts.m_targets.emplace_back(value);
				continue;
			}

			if (arg == "-j" || arg == "--jobs") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				int jobs = 0;

				if (!parse_positive_int(value, jobs)) {
					return parse_result::failure(
						std::format("invalid job count '{}'", value)
					);
				}

				opts.m_jobs = jobs;
				continue;
			}

			if (arg.starts_with("--jobs=")) {
				std::string_view value = arg.substr(std::string_view("--jobs=").size());

				int jobs = 0;

				if (!parse_positive_int(value, jobs)) {
					return parse_result::failure(
						std::format("invalid job count '{}'", value)
					);
				}

				opts.m_jobs = jobs;
				continue;
			}

			if (arg.starts_with("-j") && arg.size() > 2) {
				std::string_view value = arg.substr(2);

				int jobs = 0;

				if (!parse_positive_int(value, jobs)) {
					return parse_result::failure(
						std::format("invalid job count '{}'", value)
					);
				}

				opts.m_jobs = jobs;
				continue;
			}

			if (is_option(arg)) {
				return parse_result::failure(
					std::format("unknown option '{}'", arg)
				);
			}

			if (!saw_first_positional) {
				saw_first_positional = true;

				action_kind parsed_action{};

				if (parse_action(arg, parsed_action)) {
					opts.m_action = parsed_action;

					if (parsed_action == action_kind::help) {
						opts.m_show_help = true;
					}

					if (parsed_action == action_kind::version) {
						opts.m_show_version = true;
					}

					continue;
				}
			}

			opts.m_targets.emplace_back(arg);
		}

		return parse_result::success(std::move(opts));
	}

	inline constexpr auto parse(const sys::command_line& cmd) {
		return parse(cmd.m_args);
	}
}

#endif// ===== end include/mgmake/cli/parse.hxx =====

// skipped duplicate include: include/mgmake/cli/util.hxx
// skipped duplicate include: include/mgmake/dag/action.hxx
// skipped duplicate include: include/mgmake/dag/artifact.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx
// skipped duplicate include: include/mgmake/dag/target.hxx
// skipped duplicate include: include/mgmake/detail/convert.hxx

// ===== begin include/mgmake/detail/enum_string.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_ENUM_STRING_HXX
#define MGMAKE_DETAIL_ENUM_STRING_HXX


// ===== begin include/mgmake/detail/static_string.hxx =====
#pragma once

#ifndef MGMAKE_DETAIL_STATIC_STRING_HXX
#define MGMAKE_DETAIL_STATIC_STRING_HXX

namespace mgmake::detail {
	template <std::size_t N>
	struct static_string {
		std::array<char, N> m_data{};

		constexpr static_string() = default;

		constexpr static_string(const char (&str)[N]) {
			for (std::size_t i = 0; i < N; ++i) {
				m_data[i] = str[i];
			}
		}

		[[nodiscard]] static consteval std::size_t size() noexcept {
			return N - 1;
		}

		[[nodiscard]] static consteval bool empty() noexcept {
			return size() == 0;
		}

		[[nodiscard]] constexpr std::string_view view() const noexcept {
			return std::string_view{ m_data.data(), size() };
		}

		constexpr operator const char*() const {
			return m_data.data();
		}

		constexpr operator std::string_view() const {
			return view();
		}
	};

	template <std::size_t N1, std::size_t N2>
	constexpr auto operator+(
		const static_string<N1>& a,
		const static_string<N2>& b
	) {
		static_string<N1 + N2 - 1> result;

		for (std::size_t i = 0; i < N1 - 1; ++i) {
			result.m_data[i] = a.m_data[i];
		}

		for (std::size_t i = 0; i < N2; ++i) {
			result.m_data[i + N1 - 1] = b.m_data[i];
		}

		return result;
	}
}

#endif// ===== end include/mgmake/detail/static_string.hxx =====


#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace mgmake::detail {
    template <typename E>
    concept enum_type = std::is_enum_v<E>;

    template <enum_type E>
    [[nodiscard]] inline constexpr auto enum_underlying(E value) noexcept {
        return static_cast<std::underlying_type_t<E>>(value);
    }

    template <enum_type E>
    [[nodiscard]] inline constexpr bool enum_underlying_is_negative(E value) noexcept {
        using U = std::underlying_type_t<E>;

        if constexpr (std::is_signed_v<U>) {
            return enum_underlying(value) < 0;
        } else {
            return false;
        }
    }

    template <enum_type E>
    struct enum_view_entry {
        E m_value{};
        std::string_view m_name{};
    };

    template <auto Value, static_string Name> requires enum_type<std::remove_cv_t<decltype(Value)>>
    struct enum_entry {
        using value_type = std::remove_cv_t<decltype(Value)>;

        static constexpr value_type m_value = Value;
        static constexpr auto m_name = Name;

        [[nodiscard]] static constexpr value_type value() noexcept {
            return m_value;
        }

        [[nodiscard]] static constexpr std::string_view name() noexcept {
            return m_name.view();
        }

        [[nodiscard]] static consteval std::size_t name_size() noexcept {
            return m_name.size();
        }

        [[nodiscard]] static consteval bool name_empty() noexcept {
            return m_name.empty();
        }
    };

    template <enum_type E, typename... Entries>
    struct enum_table {
        using value_type = E;

        static_assert(
            (std::same_as<typename Entries::value_type, E> && ...),
            "mgmake enum_table entries must all use the table enum type"
        );

        [[nodiscard]] static consteval std::size_t size() noexcept {
            return sizeof...(Entries);
        }

        [[nodiscard]] static constexpr auto values() noexcept {
            return std::array<E, sizeof...(Entries)>{
                Entries::value()...
            };
        }

        [[nodiscard]] static constexpr auto names() noexcept {
            return std::array<std::string_view, sizeof...(Entries)>{
                Entries::name()...
            };
        }

        [[nodiscard]] static constexpr auto entries() noexcept {
            return std::array<enum_view_entry<E>, sizeof...(Entries)>{
                enum_view_entry<E>{ Entries::value(), Entries::name() }...
            };
        }

        [[nodiscard]] static constexpr bool contains_value(E value) noexcept {
            return ((Entries::value() == value) || ...);
        }

        [[nodiscard]] static constexpr bool contains_name(std::string_view name) noexcept {
            return ((Entries::name() == name) || ...);
        }

        [[nodiscard]] static constexpr std::optional<std::string_view> to_string_opt(
            E value
        ) noexcept {
            std::optional<std::string_view> result{};

            ((Entries::value() == value
                ? (result = Entries::name(), true)
                : false) || ...);

            return result;
        }

        [[nodiscard]] static constexpr std::string_view to_string(
            E value,
            std::string_view fallback = "unknown"
        ) noexcept {
            const auto result = to_string_opt(value);

            if (result.has_value()) {
                return *result;
            }

            return fallback;
        }

        [[nodiscard]] static constexpr std::optional<E> from_string(
            std::string_view name
        ) noexcept {
            std::optional<E> result{};

            ((Entries::name() == name
                ? (result = Entries::value(), true)
                : false) || ...);

            return result;
        }

        [[nodiscard]] static constexpr E from_string_or(
            std::string_view name,
            E fallback
        ) noexcept {
            const auto result = from_string(name);

            if (result.has_value()) {
                return *result;
            }

            return fallback;
        }

        template <typename Fn>
        static constexpr void for_each_name(Fn&& fn) {
            (fn(Entries::name()), ...);
        }

        template <typename Fn>
        static constexpr void for_each_entry(Fn&& fn) {
            (fn(Entries::value(), Entries::name()), ...);
        }

        [[nodiscard]] static consteval bool has_no_empty_names() noexcept {
            return ((!Entries::name_empty()) && ...);
        }

        [[nodiscard]] static consteval bool has_no_duplicate_names() noexcept {
            constexpr auto table_names = names();

            for (std::size_t i = 0; i < table_names.size(); ++i) {
                for (std::size_t j = i + 1; j < table_names.size(); ++j) {
                    if (table_names[i] == table_names[j]) {
                        return false;
                    }
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool has_no_duplicate_values() noexcept {
            constexpr auto table_values = values();

            for (std::size_t i = 0; i < table_values.size(); ++i) {
                for (std::size_t j = i + 1; j < table_values.size(); ++j) {
                    if (table_values[i] == table_values[j]) {
                        return false;
                    }
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool covers_zero_based_count(E count) noexcept {
            if (enum_underlying_is_negative(count)) {
                return false;
            }

            const auto count_value = static_cast<std::size_t>(enum_underlying(count));

            if (sizeof...(Entries) != count_value) {
                return false;
            }

            constexpr auto table_values = values();

            for (std::size_t expected = 0; expected < count_value; ++expected) {
                bool found = false;

                for (const auto value : table_values) {
                    if (enum_underlying_is_negative(value)) {
                        return false;
                    }

                    if (static_cast<std::size_t>(enum_underlying(value)) == expected) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool is_canonical() noexcept {
            return has_no_empty_names()
                && has_no_duplicate_names()
                && has_no_duplicate_values();
        }

        [[nodiscard]] static consteval bool is_zero_based_count_canonical(
            E count
        ) noexcept {
            return is_canonical()
                && covers_zero_based_count(count);
        }

        [[nodiscard]] static consteval bool is_display_aliases() noexcept {
            return has_no_empty_names()
                && has_no_duplicate_names();
        }

        [[nodiscard]] static consteval bool is_parse_aliases() noexcept {
            return has_no_duplicate_names();
        }

        template <static_string Separator = ", ">
        [[nodiscard]] static consteval auto choices_static_string() noexcept {
            constexpr std::size_t entry_count = sizeof...(Entries);
            constexpr std::size_t separator_size = Separator.size();

            constexpr std::size_t names_size =
                (Entries::name_size() + ... + std::size_t{ 0 });

            constexpr std::size_t separators_size =
                entry_count == 0
                    ? 0
                    : (entry_count - 1) * separator_size;

            constexpr std::size_t total_size = names_size + separators_size;

            static_string<total_size + 1> result{};

            std::size_t out = 0;
            std::size_t index = 0;

            auto append = [&](std::string_view text) constexpr {
                for (const char ch : text) {
                    result.m_data[out] = ch;
                    ++out;
                }
            };

            auto append_entry = [&](std::string_view name) constexpr {
                if (index != 0) {
                    append(Separator.view());
                }

                append(name);
                ++index;
            };

            (append_entry(Entries::name()), ...);

            result.m_data[total_size] = '\0';

            return result;
        }

        template <static_string Separator = ", ">
        [[nodiscard]] static std::string choices_string() {
            constexpr auto result = choices_static_string<Separator>();

            return std::string{ result.view() };
        }
    };
}

#endif// ===== end include/mgmake/detail/enum_string.hxx =====

// skipped duplicate include: include/mgmake/detail/static_string.hxx

// ===== begin include/mgmake/lower/target.hxx =====
#pragma once

#ifndef MGMK_LOWER_TARGET_HXX
#define MGMK_LOWER_TARGET_HXX

// skipped duplicate include: include/mgmake/dag/artifact.hxx
// skipped duplicate include: include/mgmake/dag/target.hxx

#include <filesystem>
#include <optional>
#include <set>
#include <vector>

namespace mgmake::lower {
	struct target {
		std::optional<dag::target::id> m_dag_target;
		std::vector<dag::artifact::id> m_linkable_artifacts;
		// Include dirs inherited by consumers of this lowered target.
		std::set<std::filesystem::path> m_include_dirs;
	};
}

#endif
// ===== end include/mgmake/lower/target.hxx =====


// ===== begin include/mgmake/lower/usage.hxx =====
#pragma once

#ifndef MGMK_LOWER_USAGE_HXX
#define MGMK_LOWER_USAGE_HXX

// skipped duplicate include: include/mgmake/dag/artifact.hxx
// skipped duplicate include: include/mgmake/dag/target.hxx

#include <filesystem>
#include <set>
#include <vector>

namespace mgmake::lower {
	struct usage {
		std::set<std::filesystem::path> m_include_dirs;
		std::vector<dag::artifact::id> m_link_inputs;
		std::set<dag::target::id> m_dag_dependencies;
	};
}

#endif
// ===== end include/mgmake/lower/usage.hxx =====


// ===== begin include/mgmake/lower/emitter.hxx =====
#pragma once

#ifndef MGMK_LOWER_EMITTER_HXX
#define MGMK_LOWER_EMITTER_HXX

// skipped duplicate include: include/mgmake/dag/action.hxx
// skipped duplicate include: include/mgmake/dag/artifact.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx
// skipped duplicate include: include/mgmake/dag/target.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace mgmake::lower {
	struct emitter {
		dag::graph& m_graph;

		emitter(dag::graph& graph)
			: m_graph{graph} {}

		dag::artifact::id source(const std::filesystem::path& path) {
			return m_graph.create_artifact(dag::artifact::kind::source, path);
		}

		dag::artifact::id generated(const std::filesystem::path& path) {
			return m_graph.create_artifact(dag::artifact::kind::generated, path);
		}

		const std::filesystem::path& path(dag::artifact::id id) const {
			return m_graph.artifact(id).m_path;
		}

		dag::action::id action(
			std::string name,
			std::string description,
			std::vector<dag::artifact::id> inputs,
			std::vector<dag::artifact::id> outputs,
			sys::command_line command
		) {
			return m_graph.create_action(
				std::move(name),
				std::move(description),
				std::move(inputs),
				std::move(outputs),
				false,
				std::move(command),
				std::filesystem::path{}
			);
		}

		dag::target::id target(dag::target target) {
			return m_graph.create_target(std::move(target));
		}
	};
}

#endif
// ===== end include/mgmake/lower/emitter.hxx =====


// ===== begin include/mgmake/lower/context.hxx =====
#pragma once

#ifndef MGMK_LOWER_CONTEXT_HXX
#define MGMK_LOWER_CONTEXT_HXX

// skipped duplicate include: include/mgmake/lower/emitter.hxx
// skipped duplicate include: include/mgmake/lower/target.hxx
// skipped duplicate include: include/mgmake/lower/usage.hxx
// skipped duplicate include: include/mgmake/build/request.hxx

// ===== begin include/mgmake/spec/executable.hxx =====
#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX


// ===== begin include/mgmake/spec/target.hxx =====
#pragma once

#ifndef MGMK_SPEC_TARGET_HXX
#define MGMK_SPEC_TARGET_HXX

#include <filesystem>
#include <set>
#include <string>
#include <string_view>

namespace mgmake::spec {
	template<typename target_t>
	struct target {
		std::string m_name;
		std::set<std::filesystem::path> m_sources;
		std::set<std::filesystem::path> m_include_dirs;
		std::set<std::string> m_linked_libraries;

		inline constexpr auto& name() const {
			return m_name;
		}

		inline constexpr auto& add_source(const std::filesystem::path& file) {
			m_sources.emplace(file);
			return self();
		}
		inline constexpr auto& sources() const {
			return m_sources;
		}

		inline constexpr auto& add_include_dir(const std::filesystem::path& file) {
			m_include_dirs.emplace(file);
			return self();
		}
		inline constexpr auto& include_dirs() const {
			return m_include_dirs;
		}

		inline constexpr auto& link(std::string_view lib) {
			m_linked_libraries.emplace(lib);
			return self();
		}
		inline constexpr auto& link(const std::string& lib) {
			return link(std::string_view{ lib });
		}
		inline constexpr auto& linked_libraries() const {
			return m_linked_libraries;
		}

		// Implicit cast to std::string_view for when the target needs to be identified by name
		operator std::string_view() const {
			return m_name;
		}
	private:
		inline constexpr target_t& self() {
			return *static_cast<target_t*>(this);
		}
	};
}

#endif
// ===== end include/mgmake/spec/target.hxx =====


#include <vector>

namespace mgmake::spec {
	struct executable : public target<executable> {
		using id = std::vector<executable>::size_type;
	};
}

#endif
// ===== end include/mgmake/spec/executable.hxx =====


// ===== begin include/mgmake/spec/library.hxx =====
#pragma once

#ifndef MGMK_SPEC_LIBRARY_HXX
#define MGMK_SPEC_LIBRARY_HXX

// skipped duplicate include: include/mgmake/spec/target.hxx

#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project;

	struct library : public target<library> {
		using id = std::vector<library>::size_type;

		enum struct kind {
			static_lib, // k prefix bc static is a keyword
			shared_lib,
			interface
		} m_kind;

		library(std::string_view name, kind k) : target<library>{ std::string{ name } }, m_kind{k} {}
	};
}

#endif
// ===== end include/mgmake/spec/library.hxx =====


#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project;
}

namespace mgmake::lower {
	struct context {
		const build::request& m_req;
		const spec::project& m_project;
		lower::emitter m_emit;

		context(
			dag::graph& graph,
			const build::request& req,
			const spec::project& project
		);

		lower::emitter& emit() {
			return m_emit;
		}

		const lower::emitter& emit() const {
			return m_emit;
		}

		const build::request& request() const {
			return m_req;
		}

		const build::toolchain& toolchain() const {
			return m_req.toolchain();
		}

		const lower::target& lower_library(spec::library::id id);
		void lower_executable(spec::executable::id id);

		lower::usage use_libraries(
			const std::set<std::string>& libraries,
			std::string_view owner_name
		);

		template<typename target_t>
		std::vector<dag::artifact::id> lower_objects(
			const target_t& target,
			const std::set<std::filesystem::path>& include_dirs
		);

	private:
		lower::target lower_interface_library(
			const spec::library& lib,
			lower::usage usage
		);

		lower::target lower_static_library(
			const spec::library& lib,
			lower::usage usage
		);

		lower::target lower_shared_library(
			const spec::library& lib,
			lower::usage usage
		);

		std::vector<std::optional<lower::target>> m_libraries;
		std::set<spec::library::id> m_active_libraries;
	};
}

#endif
// ===== end include/mgmake/lower/context.hxx =====

// skipped duplicate include: include/mgmake/spec/executable.hxx

// ===== begin include/mgmake/spec/executable_impl.hxx =====
#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_IMPL_HXX
#define MGMK_SPEC_EXECUTABLE_IMPL_HXX

// skipped duplicate include: include/mgmake/spec/executable.hxx

#endif
// ===== end include/mgmake/spec/executable_impl.hxx =====

// skipped duplicate include: include/mgmake/spec/library.hxx

// ===== begin include/mgmake/spec/library_impl.hxx =====
#pragma once

#ifndef MGMK_SPEC_LIBRARY_IMPL_HXX
#define MGMK_SPEC_LIBRARY_IMPL_HXX

// skipped duplicate include: include/mgmake/spec/library.hxx

#endif
// ===== end include/mgmake/spec/library_impl.hxx =====


// ===== begin include/mgmake/spec/project.hxx =====
#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

// skipped duplicate include: include/mgmake/build/request.hxx
// skipped duplicate include: include/mgmake/dag/graph.hxx
// skipped duplicate include: include/mgmake/detail/assert.hxx
// skipped duplicate include: include/mgmake/spec/executable.hxx
// skipped duplicate include: include/mgmake/spec/library.hxx

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project {
		std::string m_name;
		std::vector<spec::executable> m_executables;
		std::vector<spec::library> m_libraries;

		inline constexpr project& add_target(const spec::executable& exe) {
			mgmkassert(not exe.m_name.empty(), "mgmake spec: executable target has no name");
            mgmkassert(not find_library(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");
            mgmkassert(not find_executable(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");

			m_executables.emplace_back(exe);
			return *this;
		}
		inline constexpr project& add_target(const spec::library& lib) {
			mgmkassert(not lib.m_name.empty(), "mgmake spec: library target has no name");
            mgmkassert(not find_executable(lib.m_name).has_value(), "mgmake spec: target name conflict '" + lib.m_name + "'");

			// Skip if the library was already added
			if (find_library(lib.m_name).has_value()) {
                return *this;
            }

			m_libraries.emplace_back(lib);
			return *this;
		}

		const std::optional<spec::library::id> find_library(std::string_view name) const {
            for (spec::library::id idx = 0; idx < m_libraries.size(); idx++) {
				const auto& lib = m_libraries.at(idx);
                if (lib.m_name == name) {
                    return idx;
                }
            }
            return std::nullopt;
		}
		const spec::library* get_library(const spec::library::id idx) const {
			if (idx >= m_libraries.size())
				return nullptr;
			return &m_libraries.at(idx);
		}

		const std::optional<spec::executable::id> find_executable(std::string_view name) const {
            for (spec::executable::id idx = 0; idx < m_executables.size(); idx++) {
				const auto& exe = m_executables.at(idx);
                if (exe.m_name == name) {
                    return idx;
                }
            }
            return std::nullopt;
        }
		const spec::executable* get_executable(const spec::executable::id idx) const {
			if (idx >= m_executables.size())
				return nullptr;
			return &m_executables.at(idx);
		}

		dag::graph graph(const build::request& req) const;
	};
}

#endif
// ===== end include/mgmake/spec/project.hxx =====


// ===== begin include/mgmake/lower/objects.hxx =====
#pragma once

#ifndef MGMK_LOWER_OBJECTS_HXX
#define MGMK_LOWER_OBJECTS_HXX

// skipped duplicate include: include/mgmake/lower/context.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <cstddef>
#include <filesystem>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace mgmake::lower {
	template<typename target_t>
	inline std::vector<dag::artifact::id> context::lower_objects(
		const target_t& target,
		const std::set<std::filesystem::path>& include_dirs
	) {
		const auto& tc = toolchain();

		std::vector<dag::artifact::id> object_ids{};
		object_ids.reserve(target.m_sources.size());

		std::size_t source_index = 0;

		for (const auto& source : target.m_sources) {
			auto source_id = m_emit.source(source);

			std::filesystem::path object_path =
				request().build_dir() /
				"obj" /
				target.m_name /
				(std::to_string(source_index++) +
#if defined(MGMK_PLATFORM_WINDOWS)
					".obj"
#else
					".o"
#endif
				);

			auto object_id = m_emit.generated(object_path);

			sys::command_line command{};
			command.m_args.emplace_back(tc.cxx());

			for (const auto& flag : tc.compile_flags()) {
				command.m_args.emplace_back(flag);
			}

			for (const auto& flag : tc.cxx_flags()) {
				command.m_args.emplace_back(flag);
			}

			for (const auto& include_dir : include_dirs) {
				switch (tc.dialect()) {
					case build::toolchain::dialect::gcc:
						command.m_args.emplace_back(std::string{"-I"} + include_dir.string());
						break;

					case build::toolchain::dialect::msvc:
						command.m_args.emplace_back(std::string{"/I"} + include_dir.string());
						break;
				}
			}

			switch (tc.dialect()) {
				case build::toolchain::dialect::gcc:
					command.m_args.emplace_back("-c");
					command.m_args.emplace_back(source.string());
					command.m_args.emplace_back("-o");
					command.m_args.emplace_back(object_path.string());
					break;

				case build::toolchain::dialect::msvc:
					command.m_args.emplace_back("/c");
					command.m_args.emplace_back(source.string());
					command.m_args.emplace_back(std::string{"/Fo"} + object_path.string());
					break;
			}

			m_emit.action(
				std::string{"Compile "} + source.string(),
				std::string{"Compiles source file '"} + source.string() + "' for target '" + target.m_name + "'.",
				{ source_id },
				{ object_id },
				std::move(command)
			);

			object_ids.emplace_back(object_id);
		}

		return object_ids;
	}
}

#endif
// ===== end include/mgmake/lower/objects.hxx =====


// ===== begin include/mgmake/lower/context_impl.hxx =====
#pragma once

#ifndef MGMK_LOWER_CONTEXT_IMPL_HXX
#define MGMK_LOWER_CONTEXT_IMPL_HXX

// skipped duplicate include: include/mgmake/lower/context.hxx
// skipped duplicate include: include/mgmake/lower/objects.hxx
// skipped duplicate include: include/mgmake/detail/assert.hxx
// skipped duplicate include: include/mgmake/spec/project.hxx
// skipped duplicate include: include/mgmake/sys/command_line.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx

#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::lower {
	inline context::context(
		dag::graph& graph,
		const build::request& req,
		const spec::project& project
	)
		: m_req{req}
		, m_project{project}
		, m_emit{graph}
		, m_libraries(project.m_libraries.size()) {}

	inline lower::usage context::use_libraries(
		const std::set<std::string>& libraries,
		std::string_view owner_name
	) {
		lower::usage result{};

		for (const auto& library_name : libraries) {
			const auto linked_id = m_project.find_library(library_name);

			mgmkassert(
				linked_id.has_value(),
				"mgmake lower: target '" + std::string{owner_name} +
					"' links unknown library '" + library_name + "'"
			);

			const lower::target& dep = lower_library(linked_id.value());

			if (dep.m_dag_target.has_value()) {
				result.m_dag_dependencies.emplace(dep.m_dag_target.value());
			}

			result.m_link_inputs.insert(
				result.m_link_inputs.end(),
				dep.m_linkable_artifacts.begin(),
				dep.m_linkable_artifacts.end()
			);

			result.m_include_dirs.insert_range(dep.m_include_dirs);
		}

		return result;
	}

	inline const lower::target& context::lower_library(spec::library::id id) {
		mgmkassert(
			id < m_libraries.size(),
			"mgmake lower: invalid library id"
		);

		if (m_libraries.at(id).has_value()) {
			return m_libraries.at(id).value();
		}

		const auto& lib = m_project.m_libraries.at(id);

		mgmkassert(not lib.m_name.empty(), "mgmake lower: library target has no name");

		mgmkassert(
			not m_active_libraries.contains(id),
			"mgmake lower: cyclic library dependency involving '" + lib.m_name + "'"
		);

		m_active_libraries.emplace(id);

		lower::usage usage = use_libraries(
			lib.linked_libraries(),
			lib.m_name
		);

		switch (lib.m_kind) {
			case spec::library::kind::interface:
				m_libraries.at(id) = lower_interface_library(lib, std::move(usage));
				break;

			case spec::library::kind::static_lib:
				m_libraries.at(id) = lower_static_library(lib, std::move(usage));
				break;

			case spec::library::kind::shared_lib:
				m_libraries.at(id) = lower_shared_library(lib, std::move(usage));
				break;
		}

		m_active_libraries.erase(id);

		return m_libraries.at(id).value();
	}

	inline lower::target context::lower_interface_library(
		const spec::library& lib,
		lower::usage usage
	) {
		mgmkassert(
			lib.m_sources.empty(),
			"mgmake lower: interface library '" + lib.m_name + "' cannot have sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);
		auto link_inputs = std::move(usage.m_link_inputs);

		dag::target dag_target{
			lib.m_name,
			{},
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts = std::move(link_inputs);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline lower::target context::lower_static_library(
		const spec::library& lib,
		lower::usage usage
	) {
		const auto& tc = toolchain();

		mgmkassert(
			not lib.m_sources.empty(),
			"mgmake lower: static library '" + lib.m_name + "' has no sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(lib, include_dirs);

		std::filesystem::path archive_path;

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc:
				archive_path = request().build_dir() / "lib" / ("lib" + lib.m_name + ".a");
				break;

			case build::toolchain::dialect::msvc:
				archive_path = request().build_dir() / "lib" / (lib.m_name + ".lib");
				break;
		}

		auto archive_id = m_emit.generated(archive_path);

		sys::command_line command{};
		command.m_args.emplace_back(tc.ar());

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc: {
				if (tc.archive_flags().empty()) {
					command.m_args.emplace_back("rcs");
				}
				else {
					for (const auto& flag : tc.archive_flags()) {
						command.m_args.emplace_back(flag);
					}
				}

				command.m_args.emplace_back(archive_path.string());

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(m_emit.path(object_id).string());
				}

				break;
			}

			case build::toolchain::dialect::msvc: {
				for (const auto& flag : tc.archive_flags()) {
					command.m_args.emplace_back(flag);
				}

				command.m_args.emplace_back(std::string{"/OUT:"} + archive_path.string());

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(m_emit.path(object_id).string());
				}

				break;
			}
		}

		m_emit.action(
			std::string{"Build static library "} + lib.m_name,
			std::string{"Builds static library target '"} + lib.m_name + "'.",
			object_ids,
			{ archive_id },
			std::move(command)
		);

		dag::target dag_target{
			lib.m_name,
			{ archive_id },
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts.emplace_back(archive_id);
		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline lower::target context::lower_shared_library(
		const spec::library& lib,
		lower::usage usage
	) {
		const auto& tc = toolchain();

		mgmkassert(
			not lib.m_sources.empty(),
			"mgmake lower: shared library '" + lib.m_name + "' has no sources"
		);

		mgmkassert(
			tc.dialect() == build::toolchain::dialect::gcc,
			"mgmake lower: shared library lowering is currently only implemented for GCC-like toolchains"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(lib, include_dirs);

		std::filesystem::path shared_path;

#if defined(__APPLE__)
		shared_path = request().build_dir() / "lib" / ("lib" + lib.m_name + ".dylib");
#else
		shared_path = request().build_dir() / "lib" / ("lib" + lib.m_name + ".so");
#endif

		auto shared_id = m_emit.generated(shared_path);

		sys::command_line command{};
		command.m_args.emplace_back(tc.linker());

#if defined(__APPLE__)
		command.m_args.emplace_back("-dynamiclib");
#else
		command.m_args.emplace_back("-shared");
#endif

		for (auto object_id : object_ids) {
			command.m_args.emplace_back(m_emit.path(object_id).string());
		}

		for (auto link_input : usage.m_link_inputs) {
			command.m_args.emplace_back(m_emit.path(link_input).string());
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		command.m_args.emplace_back("-o");
		command.m_args.emplace_back(shared_path.string());

		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_link_inputs.begin(), usage.m_link_inputs.end());

		m_emit.action(
			std::string{"Build shared library "} + lib.m_name,
			std::string{"Builds shared library target '"} + lib.m_name + "'.",
			std::move(inputs),
			{ shared_id },
			std::move(command)
		);

		dag::target dag_target{
			lib.m_name,
			{ shared_id },
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts.emplace_back(shared_id);
		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline void context::lower_executable(spec::executable::id id) {
		mgmkassert(
			id < m_project.m_executables.size(),
			"mgmake lower: invalid executable id"
		);

		const auto& exe = m_project.m_executables.at(id);
		const auto& tc = toolchain();

		mgmkassert(not exe.m_name.empty(), "mgmake lower: executable target has no name");
		mgmkassert(
			not exe.m_sources.empty(),
			"mgmake lower: executable target '" + exe.m_name + "' has no sources"
		);

		lower::usage usage = use_libraries(
			exe.linked_libraries(),
			exe.m_name
		);

		auto include_dirs = exe.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(exe, include_dirs);
		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_link_inputs.begin(), usage.m_link_inputs.end());

		std::filesystem::path output = request().build_dir() / exe.m_name;

#if defined(MGMK_PLATFORM_WINDOWS)
		output += ".exe";
#endif

		auto output_id = m_emit.generated(output);

		sys::command_line command{};
		command.m_args.emplace_back(tc.cxx());

		for (auto object_id : object_ids) {
			command.m_args.emplace_back(m_emit.path(object_id).string());
		}

		for (auto link_input : usage.m_link_inputs) {
			command.m_args.emplace_back(m_emit.path(link_input).string());
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc:
				command.m_args.emplace_back("-o");
				command.m_args.emplace_back(output.string());
				break;

			case build::toolchain::dialect::msvc:
				command.m_args.emplace_back(std::string{"/Fe"} + output.string());
				break;
		}

		m_emit.action(
			std::string{"Build executable "} + exe.m_name,
			std::string{"Builds executable target '"} + exe.m_name + "'.",
			std::move(inputs),
			{ output_id },
			std::move(command)
		);

		dag::target dag_target{
			exe.m_name,
			{ output_id },
			std::move(usage.m_dag_dependencies)
		};

		m_emit.target(std::move(dag_target));
	}
}

#endif
// ===== end include/mgmake/lower/context_impl.hxx =====


// ===== begin include/mgmake/spec/project_impl.hxx =====
#pragma once

#ifndef MGMK_SPEC_PROJECT_IMPL_HXX
#define MGMK_SPEC_PROJECT_IMPL_HXX

// skipped duplicate include: include/mgmake/spec/project.hxx
// skipped duplicate include: include/mgmake/lower/context_impl.hxx

namespace mgmake::spec {
	inline dag::graph project::graph(const build::request& req) const {
		dag::graph result{};
		lower::context ctx{result, req, *this};

		for (spec::library::id id = 0; id < m_libraries.size(); ++id) {
			ctx.lower_library(id);
		}

		for (spec::executable::id id = 0; id < m_executables.size(); ++id) {
			ctx.lower_executable(id);
		}

		return result;
	}
}

#endif
// ===== end include/mgmake/spec/project_impl.hxx =====

// skipped duplicate include: include/mgmake/sys/command_line.hxx
// skipped duplicate include: include/mgmake/sys/platform.hxx
// skipped duplicate include: include/mgmake/sys/util.hxx

namespace mgmake {
	template<typename ProjectType>
	int entry(const sys::command_line& command_line) {
		auto parsed = cli::parse(command_line.user_args());

		if (!parsed) {
			std::println(stderr, "mgmake: error: {}", parsed.m_error);
			std::println(stderr, "try '{} help'", command_line.program_name());
			return 2;
		}

		const cli::options& opts = parsed.m_value;

		if (opts.m_show_help) {
			cli::print_help(command_line.program_name());
			return 0;
		}

		std::println("action: {}", cli::action_name(opts.m_action));
		std::println("backend: {}", cli::backend_name(opts.m_backend));
		std::println("build dir: {}", opts.m_build_dir);

		for (const auto& target : opts.m_targets) {
			std::println("target: {}", target);
		}

		return 0;
	}
}
namespace mgmk = mgmake;

#if defined(MGMK_PLATFORM_WINDOWS) and defined(WIN32_LEAN_AND_MEAN)
#define MGMAKE_BUILD_ENTRY(ProjectType) \
int wmain(int argc, wchar_t** argv) { \
    auto args = ::mgmk::sys::args_from_wide(argc, argv); \
    return ::mgmk::entry<ProjectType>(args); \
}
#else
#define MGMAKE_BUILD_ENTRY(ProjectType) \
int main(int argc, char** argv) { \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv); \
    return ::mgmk::entry<ProjectType>(args); \
}
#endif

// Short-hand
#define MGMK_BUILD_ENTRY MGMAKE_BUILD_ENTRY
#define MGMK_ENTRY MGMK_BUILD_ENTRY

#endif
// ===== end include/mgmake/mgmake.hxx =====


#endif // MGMAKE_SINGLE_HEADER_HXX
