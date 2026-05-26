#pragma once

#ifndef MGMAKE_HXX
#define MGMAKE_HXX

#include <array>
#include <charconv>
#include <cstddef>
#include <format>
#include <limits>
#include <print>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>

    #define MGMK_PLATFORM_WINDOWS 1
#elif defined(__unix__) || defined(__APPLE__)
    #define MGMK_PLATFORM_POSIX 1
#else
    #define MGMK_PLATFORM_UNSUPPORTED 1
#endif

namespace mgmake {
	namespace detail {
		// Poof! You have a T
		template<typename T>
		constexpr decltype(auto) poof() {
			constexpr T value;
			return value;
		}

		// Compile-time string type
		template<std::size_t N>
		struct static_string {
			std::array<char, N> data{};

			constexpr static_string() = default;
			// Constexpr constructor to allow implicit conversion from string literals
			constexpr static_string(const char (&str)[N]) {
				for (std::size_t i = 0; i < N; ++i) {
					data[i] = str[i];
				}
			}

			[[nodiscard]] constexpr std::string_view view() const {
				return data.data();
			}

			// Required for template parameter equivalence checks in C++20
			constexpr operator const char*() const { return data.data(); }

			constexpr operator std::string_view() const { return data.view(); }
		};

		// Concat 2 static strings
		template<size_t N1, size_t N2>
		constexpr auto operator+(const static_string<N1>& a, const static_string<N2>& b) {
			static_string<N1 + N2 - 1> result;

			for (size_t i = 0; i < N1 - 1; ++i)
				result.data[i] = a.data[i];

			for (size_t i = 0; i < N2; ++i)
				result.data[i + N1 - 1] = b.data[i];

			return result;
		}

#ifdef MGMK_PLATFORM_WINDOWS
		inline constexpr std::string wide_to_utf8(std::wstring_view text) {
			if (text.empty()) {
				return {};
			}

			if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
				throw std::runtime_error("Wide string is too large to convert to UTF-8");
			}

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

			if (utf8_size <= 0) {
				throw std::runtime_error("Failed to calculate UTF-8 argument size");
			}

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

			if (written <= 0) {
				throw std::runtime_error("Failed to convert command line argument to UTF-8");
			}

			return result;
		}
#endif
	}

	// System functions
	// Typically platform-independent wrappers around platform-specific functions
	namespace sys {
		enum struct platform {
			windows,
			posix,
			unsupported
		};

		struct command_line {
			std::vector<std::string> args;

			std::string_view program_name() const {
				return args.empty() ? std::string_view{} : std::string_view(args[0]);
			}

			std::span<const std::string> user_args() const {
				if (args.size() <= 1) {
					return {};
				}

				return std::span<const std::string>(args).subspan(1);
			}
		};

		inline constexpr command_line args_from_utf8(int argc, char** argv) {
			command_line result;

			for (int i = 0; i < argc; ++i) {
				result.args.emplace_back(argv[i] ? argv[i] : "");
			}

			return result;
		}

#ifdef MGMK_PLATFORM_WINDOWS
		inline constexpr command_line args_from_wide(int argc, wchar_t** argv) {
			command_line result;
			if (argc <= 0 || argv == nullptr) {
				return result;
			}
			result.args.reserve(static_cast<std::size_t>(argc));
			for (int i = 0; i < argc; ++i) {
				if (argv[i] == nullptr) {
					result.args.emplace_back();
					continue;
				}
				result.args.emplace_back(detail::wide_to_utf8(argv[i]));
			}
			return result;
		}
#endif
	}

	namespace cli {
		enum struct action_kind {
			build,
			generate,
			clean,
			run,
			help,
			version
		};

		enum struct backend_kind {
			automatic,
			ninja,
			make,
			direct
		};

		struct options {
			action_kind action = action_kind::build;
			backend_kind backend = backend_kind::automatic;

			std::string build_dir = "build";

			std::vector<std::string> targets;
			std::vector<std::string> passthrough_args;

			int jobs = 0;

			bool verbose = false;
			bool dry_run = false;
			bool show_help = false;
			bool show_version = false;
		};

		struct parse_result {
			bool ok = false;
			options value{};
			std::string error{};

			[[nodiscard]] explicit operator bool() const {
				return ok;
			}

			static parse_result success(options opts) {
				parse_result result;
				result.ok = true;
				result.value = std::move(opts);
				return result;
			}

			static parse_result failure(std::string message) {
				parse_result result;
				result.ok = false;
				result.error = std::move(message);
				return result;
			}
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

		[[nodiscard]] inline constexpr std::string_view backend_name(backend_kind backend) {
			switch (backend) {
				case backend_kind::automatic:
					return "automatic";
				case backend_kind::ninja:
					return "ninja";
				case backend_kind::make:
					return "make";
				case backend_kind::direct:
					return "direct";
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

		[[nodiscard]] inline constexpr bool parse_backend(std::string_view text, backend_kind& out) {
			if (text == "auto" || text == "automatic") {
				out = backend_kind::automatic;
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

		[[nodiscard]] inline constexpr parse_result parse(std::span<const std::string> args) {
			options opts;

			bool saw_first_positional = false;

			for (std::size_t i = 0; i < args.size(); ++i) {
				std::string_view arg = args[i];

				if (arg == "--") {
					for (++i; i < args.size(); ++i) {
						opts.passthrough_args.emplace_back(args[i]);
					}

					break;
				}

				if (arg == "-h" || arg == "--help") {
					opts.action = action_kind::help;
					opts.show_help = true;
					continue;
				}

				if (arg == "--version") {
					opts.action = action_kind::version;
					opts.show_version = true;
					continue;
				}

				if (arg == "-v" || arg == "--verbose") {
					opts.verbose = true;
					continue;
				}

				if (arg == "--dry-run") {
					opts.dry_run = true;
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

					opts.backend = backend;
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

					opts.backend = backend;
					continue;
				}

				if (arg == "--build-dir") {
					std::string value;
					std::string error;

					if (!consume_value(args, i, arg, value, error)) {
						return parse_result::failure(std::move(error));
					}

					opts.build_dir = std::move(value);
					continue;
				}

				if (arg.starts_with("--build-dir=")) {
					std::string_view value = arg.substr(std::string_view("--build-dir=").size());

					if (value.empty()) {
						return parse_result::failure("missing value after '--build-dir='");
					}

					opts.build_dir = std::string(value);
					continue;
				}

				if (arg == "--target") {
					std::string value;
					std::string error;

					if (!consume_value(args, i, arg, value, error)) {
						return parse_result::failure(std::move(error));
					}

					opts.targets.emplace_back(std::move(value));
					continue;
				}

				if (arg.starts_with("--target=")) {
					std::string_view value = arg.substr(std::string_view("--target=").size());

					if (value.empty()) {
						return parse_result::failure("missing value after '--target='");
					}

					opts.targets.emplace_back(value);
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

					opts.jobs = jobs;
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

					opts.jobs = jobs;
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

					opts.jobs = jobs;
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
						opts.action = parsed_action;

						if (parsed_action == action_kind::help) {
							opts.show_help = true;
						}

						if (parsed_action == action_kind::version) {
							opts.show_version = true;
						}

						continue;
					}
				}

				opts.targets.emplace_back(arg);
			}

			return parse_result::success(std::move(opts));
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

	template<typename ProjectType>
	int entry(const sys::command_line& command_line) {
		auto parsed = cli::parse(command_line.user_args());

		if (!parsed) {
			std::println(stderr, "mgmake: error: {}", parsed.error);
			std::println(stderr, "try '{} help'", command_line.program_name());
			return 2;
		}

		const cli::options& opts = parsed.value;

		if (opts.show_help) {
			cli::print_help(command_line.program_name());
			return 0;
		}

		std::println("action: {}", cli::action_name(opts.action));
		std::println("backend: {}", cli::backend_name(opts.backend));
		std::println("build dir: {}", opts.build_dir);

		for (const auto& target : opts.targets) {
			std::println("target: {}", target);
		}

		return 0;
	}
}
namespace mgmk = mgmake;

#ifdef MGMK_PLATFORM_WINDOWS
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