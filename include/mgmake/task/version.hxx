#pragma once

#ifndef MGMAKE_TASK_VERSION_HXX
#define MGMAKE_TASK_VERSION_HXX

#include "../cli/default_options.hxx"
#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct version {
		static constexpr auto option = cli::option
			.name<"version">()
			.description<"Display the version of MGMake.">()
			.task<true>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
            // Only display version number for short
            if (opts.template get<"short">()) {
                std::println("{}", MGMK_VERSION);
                return sys::exit_code::success;
            }
            // Otherwise print "MGMake <version>"
			std::println("MGMake {}", MGMK_VERSION);
            // Include additional info for verbose
            if (opts.template get<"verbose">()) {
                // Commit
                print_commit();
                // Compiler
                print_compiler();
                // C++ mode
                std::println("C++ mode: {}", cpp_mode());
                // Standard lib
                print_standard_library();
                // Host platform
                // Build mode
                std::println("Build mode: {}", build_mode());
            }
			return sys::exit_code::success;
		}

        static inline constexpr void print_commit() {
#if defined(MGMK_VERSION_COMMIT)
			if constexpr (MGMK_VERSION_DIRTY) {
				std::println("Commit: {}-dirty", MGMK_VERSION_COMMIT);
			} else {
				std::println("Commit: {}", MGMK_VERSION_COMMIT);
			}
#else
			std::println("Commit: unknown");
#endif
		}

        static inline constexpr void print_compiler() {
#if defined(__apple_build_version__)
			std::println("Compiler: Apple Clang {}", __clang_version__);
#elif defined(__clang__)
			std::println("Compiler: Clang {}", __clang_version__);
#elif defined(_MSC_VER)
			std::println("Compiler: MSVC {}", _MSC_FULL_VER);
#elif defined(__GNUC__)
			std::println("Compiler: GCC {}", __VERSION__);
#else
			std::println("Compiler: unknown");
#endif
		}

        static inline constexpr void print_standard_library() {
#if defined(_LIBCPP_VERSION)
			std::println("Standard lib: libc++");
#elif defined(__GLIBCXX__)
			std::println("Standard lib: libstdc++");
#elif defined(_MSVC_STL_UPDATE) || defined(_CPPLIB_VER)
			std::println("Standard lib: MSVC STL");
#else
			std::println("Standard lib: unknown");
#endif
		}

        [[nodiscard]] static inline constexpr std::string_view cpp_mode() noexcept {
#if defined(_MSVC_LANG)
			constexpr long version = _MSVC_LANG;
#else
			constexpr long version = __cplusplus;
#endif

			if constexpr (version > 202302L) {
				return "C++26";
			} else if constexpr (version >= 202302L) {
				return "C++23";
			} else if constexpr (version >= 202002L) {
				return "C++20";
			} else if constexpr (version >= 201703L) {
				return "C++17";
			} else {
				return "pre-C++17";
			}
		}

        [[nodiscard]] static inline constexpr std::string_view build_mode() noexcept {
#if defined(MGMK_BUILD_DEBUG)
			return "Debug";
#elif defined(MGMK_BUILD_RELEASE)
			return "Release";
#elif defined(NDEBUG)
			return "Release";
#else
			return "Debug (Unspecified)";
#endif
		}
	};
}

#endif // MGMAKE_TASK_VERSION_HXX