#pragma once

#ifndef MGMAKE_TASK_TASK_TRAITS_HXX
#define MGMAKE_TASK_TASK_TRAITS_HXX

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
	template<typename task_t, typename config_t>
	concept task_handler = requires(const sys::shell& cmd, const cli::options& opts) {
		{
			task_t::template handle<config_t>(cmd, opts)
		} -> std::same_as<std::expected<sys::exit_code, std::string>>;
	};

	template<typename task_t>
	struct task_traits {
		using task_type = task_t;
		using option_type = task_type::option_type;
		template<typename config_t>
		static constexpr bool valid_handler = task_handler<task_type, config_t>;

		static constexpr std::string_view name() {
			return option_type::name_value.view();
		}
		static constexpr std::string_view description() {
			return option_type::description_value.view();
		}

		static constexpr bool match(std::string_view arg) {
			return option_type::match(arg);
		}
	};
}

#endif // MGMAKE_TASK_TASK_HXX