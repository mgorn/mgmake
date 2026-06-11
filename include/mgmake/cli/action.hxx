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
		detail::enum_entry<action_kind::help, "help">,
		detail::enum_entry<action_kind::version, "version">
	>;

	static_assert(
		action_kind_parse_names::is_display_aliases(),
		"action_kind_parse_names must not contain duplicate or empty names"
	);

	template <
		action_kind Action,
		detail::static_string Description
	>
	struct action_help_entry {
		static constexpr action_kind m_action = Action;
		static constexpr auto m_description = Description;

		[[nodiscard]] static constexpr action_kind action() noexcept {
			return m_action;
		}

		[[nodiscard]] static constexpr std::string_view description() noexcept {
			return m_description.view();
		}
	};

	template <typename... Entries>
	struct action_help_table {
		template <typename Fn>
		static constexpr void for_each(Fn&& fn) {
			(fn(
				Entries::action(),
				action_name(Entries::action()),
				Entries::description()
			), ...);
		}
	};

	using action_help_entries = action_help_table<
		action_help_entry<
			action_kind::build,
			"Build the project. This is the default command."
		>,
		action_help_entry<
			action_kind::generate,
			"Generate backend build files."
		>,
		action_help_entry<
			action_kind::clean,
			"Remove generated build output."
		>,
		action_help_entry<
			action_kind::run,
			"Build and run a target."
		>,
		action_help_entry<
			action_kind::help,
			"Show this help text."
		>,
		action_help_entry<
			action_kind::version,
			"Show version information."
		>
	>;

	[[nodiscard]] inline constexpr std::string_view action_name(action_kind action) noexcept {
		return action_kind_names::to_string(action);
	}

	template <typename Fn>
	inline constexpr void for_each_action_help(Fn&& fn) {
		action_help_entries::for_each(std::forward<Fn>(fn));
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

#endif
