#pragma once

#ifndef MGMAKE_TASK_TASK_TRAITS_HXX
#define MGMAKE_TASK_TASK_TRAITS_HXX

#include "../sys/exit_code.hxx"

#include <concepts>
#include <expected>
#include <string>

// Forward decl if needed
namespace mgmake::cli {
	struct options;
}
namespace mgmake::sys {
	struct shell;
}

namespace mgmake::task {
	template<typename task_t, auto config_v>
	concept task_handler = requires(const sys::shell& cmd, const cli::options& opts) {
		{
			task_t::template handle<config_v>(cmd, opts)
		} -> std::same_as<std::expected<sys::exit_code, std::string>>;
	};

	template<typename task_t>
	struct task_traits {
		using task_type = task_t;
		static constexpr auto option = task_type::option;

		template<auto config_v>
		static constexpr bool valid_handler = task_handler<task_type, config_v>;

		static constexpr auto name() {
			return option.name();
		}
		static constexpr auto description() {
			return option.description();
		}

		static constexpr bool match(std::string_view arg) {
			return option.match(arg);
		}
	};
}

#endif // MGMAKE_TASK_TASK_HXX