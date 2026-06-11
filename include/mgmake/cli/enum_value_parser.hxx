#pragma once

#ifndef MGMAKE_CLI_ENUM_VALUE_PARSER_HXX
#define MGMAKE_CLI_ENUM_VALUE_PARSER_HXX

#include "action.hxx"
#include "backend.hxx"
#include "value_parser.hxx"

#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace mgmake::cli {
	template <typename ParseTable, typename ChoiceTable = ParseTable>
	struct enum_value_parser {
		using value_type = typename ParseTable::value_type;

		[[nodiscard]] static constexpr bool parse(
			std::string_view text,
			value_type& out
		) noexcept {
			const auto parsed = ParseTable::from_string(text);

			if (!parsed.has_value()) {
				return false;
			}

			out = *parsed;
			return true;
		}

		template <typename Fn>
		static constexpr void for_each_choice(Fn&& fn) {
			ChoiceTable::for_each_name(std::forward<Fn>(fn));
		}

		[[nodiscard]] static std::string choices_string() {
			return ChoiceTable::choices_string();
		}

		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown value '{}'; expected one of: {}",
				text,
				ChoiceTable::choices_string()
			);
		}
	};

	template <>
	struct value_parser<backend_kind> :
		enum_value_parser<backend_kind_parse_names, backend_kind_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown backend '{}'; expected one of: {}",
				text,
				backend_kind_names::choices_string()
			);
		}
	};

	template <>
	struct value_parser<action_kind> :
		enum_value_parser<action_kind_parse_names, action_kind_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown action '{}'; expected one of: {}",
				text,
				action_kind_names::choices_string()
			);
		}
	};
}

#endif
