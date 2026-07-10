#pragma once

#ifndef MGMAKE_CLI_ACTION_HXX
#define MGMAKE_CLI_ACTION_HXX

#include "../detail/enum_string.hxx"
#include "../detail/type_list.hxx"

#include <optional>
#include <string_view>
#include <utility>

// Action names form the public command verbs accepted by the entry point.

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

	template<action_kind kind_v>
	struct action {};
	template<> struct action<action_kind::build> {
		static constexpr static_string name = "build";
		static constexpr std::array<static_string> aliases = { name };
		static constexpr static_string help_entry = "Build the project. This is the default command.";
	};
	template<> struct action<action_kind::generate> {
		static constexpr static_string name = "generate";
		static constexpr std::array<static_string> aliases = { name, "gen" };
		static constexpr static_string help_entry = "Generate backend build files.";
	};
	template<> struct action<action_kind::clean> {
		static constexpr static_string name = "clean";
		static constexpr std::array<static_string> aliases = { name };
		static constexpr static_string help_entry = "Remove generated build output.";
	};
	template<> struct action<action_kind::run> {
		static constexpr static_string name = "run";
		static constexpr std::array<static_string> aliases = { name };
		static constexpr static_string help_entry = "Build and run a target.";
	};
	template<> struct action<action_kind::tools> {
		static constexpr static_string name = "tools";
		static constexpr std::array<static_string> aliases = { name, "toolchains", "toolchain-info" };
		static constexpr static_string help_entry = "Show discovered build tools and discovery diagnostics.";
	};
	template<> struct action<action_kind::graph> {
		static constexpr static_string name = "graph";
		static constexpr std::array<static_string> aliases = { name };
		static constexpr static_string help_entry = "Write graph visualization files.";
	};
	template<> struct action<action_kind::help> {
		static constexpr static_string name = "help";
		static constexpr std::array<static_string> aliases = { name };
		static constexpr static_string help_entry = "Show this help text.";
	};
	template<> struct action<action_kind::version> {
		static constexpr static_string name = "version";
		static constexpr std::array<static_string> aliases = { name };
		static constexpr static_string help_entry = "Show version information.";
	};


	using actions = detail::type_list<
		action<action_kind::build>,
		action<action_kind::generate>,
		action<action_kind::clean>,
		action<action_kind::run>,
		action<action_kind::tools>,
		action<action_kind::graph>,
		action<action_kind::help>,
		action<action_kind::version>,
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
