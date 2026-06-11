#pragma once

#ifndef MGMAKE_CLI_OPTION_BUILDER_HXX
#define MGMAKE_CLI_OPTION_BUILDER_HXX

#include "option_parse_result.hxx"
#include "options.hxx"
#include "value_parser.hxx"
#include "enum_value_parser.hxx"
#include "util.hxx"
#include "../detail/static_string.hxx"

#include <concepts>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace mgmake::cli {
	template <detail::static_string... Values>
	struct choice_list {
		[[nodiscard]] static consteval bool empty() noexcept {
			return sizeof...(Values) == 0;
		}

		[[nodiscard]] static constexpr bool contains(std::string_view value) noexcept {
			return ((value == Values.view()) || ...);
		}

		template <typename Fn>
		static constexpr void for_each(Fn&& fn) {
			(fn(Values.view()), ...);
		}
	};
}

namespace mgmake::cli::detail {
	template <typename T>
	struct member_pointer_traits;

	template <typename Object, typename Value>
	struct member_pointer_traits<Value Object::*> {
		using object_type = Object;
		using value_type = Value;
	};

	template <auto Member>
	using member_value_t = typename member_pointer_traits<decltype(Member)>::value_type;

	template <typename T>
	struct vector_traits {
		static constexpr bool is_vector = false;
	};

	template <typename T, typename Alloc>
	struct vector_traits<std::vector<T, Alloc>> {
		static constexpr bool is_vector = true;
		using value_type = T;
	};

	template <typename T>
	inline constexpr bool is_vector_v = vector_traits<T>::is_vector;

	template <typename Parser>
	concept has_for_each_choice = requires {
		Parser::for_each_choice([](std::string_view) {});
	};

	template <typename Parser>
	concept has_choices_string = requires {
		{ Parser::choices_string() } -> std::same_as<std::string>;
	};

	template <mgmake::detail::static_string Long>
	[[nodiscard]] constexpr bool matches_long(std::string_view arg) {
		constexpr auto name = Long.view();

		return arg.size() == name.size() + 2
			&& arg[0] == '-'
			&& arg[1] == '-'
			&& arg.substr(2) == name;
	}

	template <mgmake::detail::static_string Long>
	[[nodiscard]] constexpr bool starts_with_long_equals(std::string_view arg) {
		constexpr auto name = Long.view();

		return arg.size() >= name.size() + 3
			&& arg[0] == '-'
			&& arg[1] == '-'
			&& arg.substr(2, name.size()) == name
			&& arg[2 + name.size()] == '=';
	}

	template <char Short>
	[[nodiscard]] constexpr bool matches_short(std::string_view arg) {
		if constexpr (Short == '\0') {
			return false;
		} else {
			return arg.size() == 2 && arg[0] == '-' && arg[1] == Short;
		}
	}

	template <char Short>
	[[nodiscard]] constexpr bool starts_with_short_value(std::string_view arg) {
		if constexpr (Short == '\0') {
			return false;
		} else {
			return arg.size() > 2 && arg[0] == '-' && arg[1] == Short;
		}
	}

	struct no_field_t {};
	inline constexpr no_field_t no_field{};

	struct no_callback_t {};
	inline constexpr no_callback_t no_callback{};
}

namespace mgmake::cli {
	enum struct option_mode {
		deduce,
		flag,
		value,
		append,
		callback
	};

	template <typename Option>
	struct option_impl;

	template <
		auto Field = detail::no_field,
		mgmake::detail::static_string LongName = "",
		char ShortName = '\0',
		option_mode Mode = option_mode::deduce,
		mgmake::detail::static_string ValueName = "",
		mgmake::detail::static_string Description = "",
		typename Choices = choice_list<>,
		auto Callback = detail::no_callback
	>
	struct option_builder {
		template <auto NewField>
		using field = option_builder<NewField, LongName, ShortName, Mode, ValueName, Description, Choices, Callback>;

		template <mgmake::detail::static_string NewName>
		using long_name = option_builder<Field, NewName, ShortName, Mode, ValueName, Description, Choices, Callback>;

		template <char NewShortName>
		using short_name = option_builder<Field, LongName, NewShortName, Mode, ValueName, Description, Choices, Callback>;

		using flag = option_builder<Field, LongName, ShortName, option_mode::flag, ValueName, Description, Choices, Callback>;
		using value = option_builder<Field, LongName, ShortName, option_mode::value, ValueName, Description, Choices, Callback>;
		using append = option_builder<Field, LongName, ShortName, option_mode::append, ValueName, Description, Choices, Callback>;

		template <mgmake::detail::static_string NewValueName>
		using value_name = option_builder<Field, LongName, ShortName, Mode, NewValueName, Description, Choices, Callback>;

		template <mgmake::detail::static_string NewDescription>
		using description = option_builder<Field, LongName, ShortName, Mode, ValueName, NewDescription, Choices, Callback>;

		template <mgmake::detail::static_string... NewChoices>
		using choices = option_builder<Field, LongName, ShortName, Mode, ValueName, Description, choice_list<NewChoices...>, Callback>;

		template <auto NewCallback>
		using callback = option_builder<Field, LongName, ShortName, option_mode::callback, ValueName, Description, Choices, NewCallback>;

		static option_parse_result try_parse(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			return option_impl<option_builder>::try_parse(opts, args, index, arg);
		}
	};

	template <auto Field, mgmake::detail::static_string LongName, char ShortName = '\0'>
	using value_option =
		typename option_builder<>::template field<Field>
			::template long_name<LongName>
			::template short_name<ShortName>
			::value;

	template <auto Field, mgmake::detail::static_string LongName, char ShortName = '\0'>
	using flag_option =
		typename option_builder<>::template field<Field>
			::template long_name<LongName>
			::template short_name<ShortName>
			::flag;

	template <auto Field, mgmake::detail::static_string LongName, char ShortName = '\0'>
	using append_option =
		typename option_builder<>::template field<Field>
			::template long_name<LongName>
			::template short_name<ShortName>
			::append;

	template <mgmake::detail::static_string LongName, char ShortName, auto Callback>
	using callback_option =
		typename option_builder<>::template long_name<LongName>
			::template short_name<ShortName>
			::template callback<Callback>;

	template <
		auto Field,
		mgmake::detail::static_string LongName,
		char ShortName,
		option_mode Mode,
		mgmake::detail::static_string ValueName,
		mgmake::detail::static_string Description,
		typename Choices,
		auto Callback
	>
	struct option_impl<
		option_builder<Field, LongName, ShortName, Mode, ValueName, Description, Choices, Callback>
	> {
		static option_parse_result try_parse(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			if constexpr (Mode == option_mode::callback) {
				return try_parse_callback(opts, arg);
			} else if constexpr (Mode == option_mode::flag) {
				return try_parse_flag(opts, arg);
			} else if constexpr (Mode == option_mode::value) {
				return try_parse_value(opts, args, index, arg);
			} else if constexpr (Mode == option_mode::append) {
				return try_parse_append(opts, args, index, arg);
			} else {
				static_assert(Mode != option_mode::deduce);
			}
		}

		static option_parse_result try_parse_callback(options& opts, std::string_view arg) {
			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				Callback(opts);
				return option_parse_result::success();
			}

			return option_parse_result::no_match();
		}

		static option_parse_result try_parse_flag(options& opts, std::string_view arg) {
			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				opts.*Field = true;
				return option_parse_result::success();
			}

			return option_parse_result::no_match();
		}

		static option_parse_result try_parse_value(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			using value_type = detail::member_value_t<Field>;

			if (detail::starts_with_long_equals<LongName>(arg)) {
				constexpr auto name = LongName.view();
				std::string_view value = arg.substr(3 + name.size());

				if (value.empty()) {
					return option_parse_result::failure(
						std::format("missing value after '--{}='", name)
					);
				}

				return parse_and_assign<value_type>(opts, value);
			}

			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				if (index + 1 >= args.size()) {
					return option_parse_result::failure(
						std::format("missing value after '{}'", arg)
					);
				}

				++index;
				return parse_and_assign<value_type>(opts, args[index]);
			}

			if (detail::starts_with_short_value<ShortName>(arg)) {
				return parse_and_assign<value_type>(opts, arg.substr(2));
			}

			return option_parse_result::no_match();
		}

		static option_parse_result try_parse_append(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			using vector_type = detail::member_value_t<Field>;
			using value_type = typename detail::vector_traits<vector_type>::value_type;

			if (detail::starts_with_long_equals<LongName>(arg)) {
				constexpr auto name = LongName.view();
				std::string_view value = arg.substr(3 + name.size());

				if (value.empty()) {
					return option_parse_result::failure(
						std::format("missing value after '--{}='", name)
					);
				}

				return parse_and_append<value_type>(opts, value);
			}

			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				if (index + 1 >= args.size()) {
					return option_parse_result::failure(
						std::format("missing value after '{}'", arg)
					);
				}

				++index;
				return parse_and_append<value_type>(opts, args[index]);
			}

			if (detail::starts_with_short_value<ShortName>(arg)) {
				return parse_and_append<value_type>(opts, arg.substr(2));
			}

			return option_parse_result::no_match();
		}

		template <typename Value>
		static option_parse_result parse_and_assign(options& opts, std::string_view text) {
			if constexpr (!Choices::empty()) {
				if (!Choices::contains(text)) {
					return option_parse_result::failure(
						std::format("invalid value '{}' for option '--{}'", text, LongName.view())
					);
				}
			}

			Value value{};

			if (!value_parser<Value>::parse(text, value)) {
				return option_parse_result::failure(value_parser<Value>::error(text));
			}

			opts.*Field = std::move(value);
			return option_parse_result::success();
		}

		template <typename Value>
		static option_parse_result parse_and_append(options& opts, std::string_view text) {
			if constexpr (!Choices::empty()) {
				if (!Choices::contains(text)) {
					return option_parse_result::failure(
						std::format("invalid value '{}' for option '--{}'", text, LongName.view())
					);
				}
			}

			Value value{};

			if (!value_parser<Value>::parse(text, value)) {
				return option_parse_result::failure(value_parser<Value>::error(text));
			}

			(opts.*Field).emplace_back(std::move(value));
			return option_parse_result::success();
		}
	};
}

#endif
