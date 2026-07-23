#pragma once

#ifndef MGMAKE_SYS_SHELL_HXX
#define MGMAKE_SYS_SHELL_HXX

#include <algorithm>   // std::ranges::find_first_of
#include <array>       // std::array
#include <cstddef>     // std::size_t
#include <cstdlib>     // std::system
#include <print>       // std::println
#include <ranges>      // views, ranges::to
#include <span>        // std::span
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::exchange
#include <vector>      // std::vector

#ifdef MGMK_PLATFORM_WINDOWS
#include <process.h>    // _P_WAIT, _spawnvp
#else
#include <sys/wait.h>   // WEXITSTATUS, WIFEXITED, WIFSIGNALED, WTERMSIG
#endif

namespace mgmake::sys {
    struct shell {
        inline constexpr std::string_view program_name() const {
            return m_args.empty() ? std::string_view{} : m_args.at(0);
        }

        inline constexpr std::span<const std::string> user_args() const {
            if (m_args.size() <= 1) {
                return {};
            }
            return view().subspan(1);
        }

        inline constexpr std::span<const std::string> view() const {
            return { m_args };
        }

        // Quotes and joins the arguments into command-line text used for display and backend generation.
        inline constexpr std::string full_command() const {
            return m_args
                | std::views::transform(arg_escape)
                | std::views::join_with(' ')
                | std::ranges::to<std::string>();
        }

        int invoke(bool verbose, bool dry_run) const {
            if (verbose or dry_run) {
                std::println("{}", full_command());
            }

            if (dry_run) {
                return 0;
            }

#ifdef MGMK_PLATFORM_WINDOWS
            if (m_args.empty()) {
                return -1;
            }

            // _spawnvp expects the same null-terminated argv layout as main(). The strings
            // remain owned by m_args, so these pointers stay valid for the synchronous call.
            std::vector<const char*> argv;
            argv.reserve(m_args.size() + 1);

            for (const auto& arg : m_args) {
                argv.push_back(arg.c_str());
            }
            argv.push_back(nullptr);

            // _P_WAIT blocks until the child exits and returns its exit code. Passing argv
            // directly avoids cmd.exe entirely, so values such as %PATH% and !DELAYED! are
            // delivered literally instead of being expanded as command-processor syntax.
            return static_cast<int>(_spawnvp(_P_WAIT, argv.front(), argv.data()));
#else
            // std::system executes through the POSIX shell and returns an encoded wait status,
            // not merely the integer passed to exit(). Decode the status using <sys/wait.h>.
            const int status = std::system(full_command().c_str());

            // A return value of -1 means the shell could not be started or waited upon.
            if (status == -1) {
                return -1;
            }

            // Normal termination: extract the value passed to exit() or returned from main().
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }

            // Signal termination: follow the conventional shell representation 128 + signal.
            // For example, SIGTERM is signal 15 and is therefore reported as exit code 143.
            if (WIFSIGNALED(status)) {
                return 128 + WTERMSIG(status);
            }

            // system() should normally report one of the states above. Preserve the raw status
            // for any implementation-specific state rather than inventing another mapping.
            return status;
#endif
        }

        // Returns true when an argument needs quoting in reconstructed command-line text.
        static inline constexpr bool arg_needs_escape(std::string_view arg) {
            return arg.empty() or (std::ranges::find_first_of(arg, special_characters) != arg.end());
        }

		// `_spawnvp` launches the executable directly and passes the stored arguments
		// through the Windows CRT argument parser rather than through `cmd.exe`.
		//
		// Because `cmd.exe` is not involved, shell metacharacters such as `&`, `|`,
		// `<`, `>`, `^`, `%`, `!`, `(`, and `)` have no special meaning here and must
		// be preserved as ordinary argument characters.
		//
		// Only whitespace and `"` affect how a reconstructed Windows command line is
		// split back into argv entries, so only those characters require CRT quoting.
		//
		// This escaping is suitable for direct process invocation. A command string
		// intended for `cmd.exe`, a batch file, or a shell-backed build backend would
		// require separate cmd-specific escaping.
        static inline constexpr std::array special_characters{
#ifdef MGMK_PLATFORM_WINDOWS
        ' ', '\t', '"'
#else
        ' ', '\t', '\'', '"', '$', '\\', '&', ';', '(', ')', '<', '>', '|'
#endif // MGMK_PLATFORM_WINDOWS
        };

        static inline constexpr std::string_view empty_arg_escape{
#ifdef MGMK_PLATFORM_WINDOWS
            "\"\""
#else
            "''"
#endif // MGMK_PLATFORM_WINDOWS
        };

        static inline constexpr char quote_character{
#ifdef MGMK_PLATFORM_WINDOWS
            '"'
#else
            '\''
#endif // MGMK_PLATFORM_WINDOWS
        };

        // Quotes one argument in reconstructed command-line text.
        static inline constexpr std::string arg_escape(std::string_view arg) {
            if (not arg_needs_escape(arg)) {
                return std::string{ arg };
            }

            if (arg.empty()) {
                return std::string{ empty_arg_escape };
            }

            std::string result;
            result.reserve(arg.size() + 2);
            result += quote_character;

#ifdef MGMK_PLATFORM_WINDOWS
            static constexpr char escape_character = '\\';
            std::size_t backslashes = 0;

            for (const char ch : arg) {
                if (ch == escape_character) {
                    ++backslashes;
                    continue;
                }

                if (ch == quote_character) {
                    result.append((std::exchange(backslashes, 0) * 2) + 1, escape_character);
                    result.push_back(quote_character);
                    continue;
                }

                result.append(std::exchange(backslashes, 0), escape_character);
                result.push_back(ch);
            }

            result.append(backslashes * 2, escape_character);
#else
            static constexpr std::string_view posix_quote_escape = "'\\''";

            auto escaped_body = arg
                | std::views::split(quote_character)
                | std::views::join_with(posix_quote_escape);

            result.append_range(escaped_body);
#endif // MGMK_PLATFORM_WINDOWS

            result += quote_character;
            return result;
        }

        // Copies the already-tokenized argc/argv arguments; escaping is handled when command text is generated.
        static inline constexpr shell from_args(int argc, const char* const* argv) {
            if (argc <= 0 or argv == nullptr) {
                return {};
            }

            return {
                std::views::counted(argv, argc)
                | std::ranges::to<std::vector<std::string>>()
            };
        }

        std::vector<std::string> m_args{};
    };
}

#endif // MGMAKE_SYS_SHELL_HXX