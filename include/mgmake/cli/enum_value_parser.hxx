#pragma once

#ifndef MGMAKE_CLI_ENUM_VALUE_PARSER_HXX
#define MGMAKE_CLI_ENUM_VALUE_PARSER_HXX

#include "action.hxx"
#include "backend.hxx"
#include "value_parser.hxx"
#include "../discovery/mode.hxx"
#include "../sys/platform.hxx"

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

	template <>
	struct value_parser<discovery::mode> :
		enum_value_parser<discovery::mode_parse_names, discovery::mode_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown tool discovery mode '{}'; expected one of: {}",
				text,
				discovery::mode_names::choices_string()
			);
		}
	};

	template <>
	struct value_parser<sys::platform> :
		enum_value_parser<sys::platform_parse_names, sys::platform_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown platform '{}'; expected one of: {}",
				text,
				sys::platform_names::choices_string()
			);
		}
	};

	template <>
	struct value_parser<sys::arch> :
		enum_value_parser<sys::arch_parse_names, sys::arch_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown architecture '{}'; expected one of: {}",
				text,
				sys::arch_names::choices_string()
			);
		}
	};

	template <>
	struct value_parser<sys::abi> :
		enum_value_parser<sys::abi_parse_names, sys::abi_names>
	{
		[[nodiscard]] static std::string error(std::string_view text) {
			return std::format(
				"unknown ABI '{}'; expected one of: {}",
				text,
				sys::abi_names::choices_string()
			);
		}
	};
}

#endif // MGMAKE_CLI_ENUM_VALUE_PARSER_HXX
