#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

#include "../detail/enum_string.hxx"

#include <optional>
#include <string_view>
#include <utility>

namespace mgmake::cli {
	enum struct action_kind {
		build,
		generate,
		clean,
		run,
		tools,
		graph,
		help,
		version,

		count
	};

	using action_kind_names = detail::enum_table<
		action_kind,
		detail::enum_entry<action_kind::build, "build">,
		detail::enum_entry<action_kind::generate, "generate">,
		detail::enum_entry<action_kind::clean, "clean">,
		detail::enum_entry<action_kind::run, "run">,
		detail::enum_entry<action_kind::tools, "tools">,
		detail::enum_entry<action_kind::graph, "graph">,
		detail::enum_entry<action_kind::help, "help">,
		detail::enum_entry<action_kind::version, "version">
	>;

	static_assert(
		action_kind_names::is_zero_based_count_canonical(action_kind::count),
		"action_kind_names must cover every action_kind value exactly once"
	);

	using action_kind_parse_names = detail::enum_table<
		action_kind,
		detail::enum_entry<action_kind::build, "build">,
		detail::enum_entry<action_kind::generate, "generate">,
		detail::enum_entry<action_kind::generate, "gen">,
		detail::enum_entry<action_kind::clean, "clean">,
		detail::enum_entry<action_kind::run, "run">,
		detail::enum_entry<action_kind::tools, "tools">,
		detail::enum_entry<action_kind::tools, "toolchains">,
		detail::enum_entry<action_kind::tools, "toolchain-info">,
		detail::enum_entry<action_kind::graph, "graph">,
		detail::enum_entry<action_kind::help, "help">,
		detail::enum_entry<action_kind::version, "version">
	>;

	static_assert(
		action_kind_parse_names::is_display_aliases(),
		"action_kind_parse_names must not contain duplicate or empty names"
	);

	using action_help_entries = detail::enum_table<
		action_kind,
		detail::enum_entry<
			action_kind::build,
			"Build the project. This is the default command."
		>,
		detail::enum_entry<
			action_kind::generate,
			"Generate backend build files."
		>,
		detail::enum_entry<
			action_kind::clean,
			"Remove generated build output."
		>,
		detail::enum_entry<
			action_kind::run,
			"Build and run a target."
		>,
		detail::enum_entry<
			action_kind::tools,
			"Show discovered build tools and discovery diagnostics."
		>,
		detail::enum_entry<
			action_kind::graph,
			"Write graph visualization files."
		>,
		detail::enum_entry<
			action_kind::help,
			"Show this help text."
		>,
		detail::enum_entry<
			action_kind::version,
			"Show version information."
		>
	>;

	static_assert(
		action_help_entries::has_no_empty_names()
			&& action_help_entries::has_no_duplicate_values()
			&& action_help_entries::covers_zero_based_count(action_kind::count),
		"action_help_entries must describe every action_kind value exactly once"
	);

	[[nodiscard]] inline constexpr std::string_view action_name(action_kind action) noexcept {
		return action_kind_names::to_string(action);
	}

	template <typename Fn>
	inline constexpr void for_each_action_help(Fn&& fn) {
		action_kind_names::for_each_entry([&](action_kind action, std::string_view name) {
			fn(action, name, action_help_entries::to_string(action, ""));
		});
	}

	[[nodiscard]] inline constexpr std::optional<action_kind> action_from_string(
		std::string_view text
	) noexcept {
		return action_kind_parse_names::from_string(text);
	}

	[[nodiscard]] inline constexpr bool parse_action(
		std::string_view text,
		action_kind& out
	) noexcept {
		const auto parsed = action_from_string(text);

		if (!parsed.has_value()) {
			return false;
		}

		out = *parsed;
		return true;
	}
}

#endif // MGMAKE_CLI_ACTION_HXX
