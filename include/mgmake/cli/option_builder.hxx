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

// Option builder describes one command-line option and the target field or callback it writes.
// The parser stores these lightweight descriptors instead of hard-coding every option branch.

namespace mgmake::cli {
	template <detail::static_string... value_vs>
	struct choice_list {
		[[nodiscard]] static consteval bool empty() noexcept {
			return sizeof...(value_vs) == 0;
		}

		[[nodiscard]] static constexpr bool contains(std::string_view value) noexcept {
			return ((value == value_vs.view()) || ...);
		}

		template <typename fn_t>
		static constexpr void for_each(fn_t&& fn) {
			(fn(value_vs.view()), ...);
		}
	};
}

namespace mgmake::cli::detail {
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

	template <typename T>
	struct option_value_traits {
		using value_type = T;
	};

	template <typename T, typename Alloc>
	struct option_value_traits<std::vector<T, Alloc>> {
		using value_type = T;
	};

	template <typename T>
	using option_value_t = typename option_value_traits<T>::value_type;

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

	template<
		typename member_t = Member,
		mgmake::detail::static_string long_name_v = "",
		char short_name_v = '\0',
		option_mode mode_v = option_mode::deduce,
		mgmake::detail::static_string value_name_v = "",
		mgmake::detail::static_string description_v = "",
		typename choices_t = choice_list<>,
		auto callback_v = nullptr
	>
	struct option_builder {
		using member_type = member_t;
		static constexpr auto long_name_value = long_name_v;
		static constexpr char short_name_value = short_name_v;
		static constexpr option_mode mode_value = mode_v;
		static constexpr auto value_name_value = value_name_v;
		static constexpr auto description_value = description_v;
		using choices_type = choices_t;
		static constexpr auto callback_value = callback_v;

		[[nodiscard]] static constexpr std::string_view long_name_view() noexcept {
			return long_name_value.view();
		}

		[[nodiscard]] static constexpr std::string_view value_name_view() noexcept {
			return value_name_value.view();
		}

		[[nodiscard]] static constexpr std::string_view description_view() noexcept {
			return description_value.view();
		}

		template <typename new_member_t = Member>
		using field = option_builder<new_member_t, long_name_value, short_name_value, mode_value, value_name_value, description_value, choices_type, callback_value>;

		template <mgmake::detail::static_string new_long_name_v>
		using long_name = option_builder<member_type, new_long_name_v, short_name_value, mode_value, value_name_value, description_value, choices_type, callback_value>;

		template <mgmake::detail::static_string new_name_v>
		using name = long_name<new_name_v>;

		template <char new_short_name_v>
		using short_name = option_builder<member_type, long_name_value, new_short_name_v, mode_value, value_name_value, description_value, choices_type, callback_value>;

		template<option_mode new_mode_v>
		using mode = option_builder<member_type, long_name_value, short_name_value, new_mode_v, value_name_value, description_value, choices_type, callback_value>;
		using flag = mode<option_mode::flag>;
		using value = mode<option_mode::value>;
		using append = mode<option_mode::append>;

		template <mgmake::detail::static_string new_value_name_v>
		using value_name = option_builder<member_type, long_name_value, short_name_value, mode_value, new_value_name_v, description_value, choices_type, callback_value>;

		template <mgmake::detail::static_string new_description_v>
		using description = option_builder<member_type, long_name_value, short_name_value, mode_value, value_name_value, new_description_v, choices_type, callback_value>;

		template <mgmake::detail::static_string... new_choices_v, typename new_choice_list_t = choice_list<new_choices_v>>
		using choices = option_builder<member_type, long_name_value, short_name_value, mode_value, value_name_value, description_value, new_choice_list_t, callback_value>;

		template <auto new_callback_v>
		using callback = option_builder<member_type, long_name_value, short_name_value, mode_value, value_name_value, description_value, choices_type, new_callback_v>;

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
	using value_option = typename option_builder<>::template field<Field>
		::template long_name<LongName>
		::template short_name<ShortName>
		::value;

	template <auto Field, mgmake::detail::static_string LongName, char ShortName = '\0'>
	using flag_option = typename option_builder<>::template field<Field>
		::template long_name<LongName>
		::template short_name<ShortName>
		::flag;

	template <auto Field, mgmake::detail::static_string LongName, char ShortName = '\0'>
	using append_option = typename option_builder<>::template field<Field>
		::template long_name<LongName>
		::template short_name<ShortName>
		::append;

	template <mgmake::detail::static_string LongName, char ShortName, auto Callback>
	using callback_option = typename option_builder<>::template long_name<LongName>
		::template short_name<ShortName>
		::template callback<Callback>;
}

namespace mgmake::cli::detail {
	template <typename member_t, option_mode mode_v>
	struct actual_option_mode {
		static constexpr option_mode value = mode_v;
	};

	template <typename member_t = Member>
	struct actual_option_mode<Field, option_mode::deduce> {
		static_assert(member_t::valid, "option_mode::deduce requires a field; use .flag, .value, .append, or .callback explicitly");

		using field_type = member_t::value_type;

		static constexpr option_mode value = [] {
			if constexpr (std::same_as<field_type, bool>) {
				return option_mode::flag;
			} else if constexpr (is_vector_v<field_type>) {
				return option_mode::append;
			} else {
				return option_mode::value;
			}
		}();
	};
}

namespace mgmake::cli {
	template<
		typename member_t = Member,
		mgmake::detail::static_string long_name_v = "",
		char short_name_v = '\0',
		option_mode mode_v = option_mode::deduce,
		mgmake::detail::static_string value_name_v = "",
		mgmake::detail::static_string description_v = "",
		typename choices_t = choice_list<>,
		auto callback_v = nullptr
	>
	struct option_impl<option_builder<member_t, long_name_v, short_name_v, mode_v, value_name_v, description_v, choices_t, callback_v>> {
		static_assert(mode_v == option_mode::callback || member_t::valid, "non-callback CLI options require a field");
		static_assert(mode_v != option_mode::callback || callback, "callback CLI options require a callback");

		static std::expected<bool,std::string> try_parse(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			constexpr option_mode actual_mode = detail::actual_option_mode<member_t, mode_v>::value;

			if constexpr (actual_mode == option_mode::callback) {
				return try_parse_callback(opts, arg);
			} else if constexpr (actual_mode == option_mode::flag) {
				return try_parse_flag(opts, arg);
			} else if constexpr (actual_mode == option_mode::value) {
				return try_parse_value(opts, args, index, arg);
			} else if constexpr (actual_mode == option_mode::append) {
				return try_parse_append(opts, args, index, arg);
			} else {
				static_assert(actual_mode != option_mode::deduce, "Actual mode should be deduced by now. Check actual_option_mode");
			}
		}

		static std::expected<bool,std::string> try_parse_callback(options& opts, std::string_view arg) {
			if (detail::matches_long<long_name_v>(arg) || detail::matches_short<short_name_v>(arg)) {
				callback_v(opts);
				return true;
			}

			return false;
		}

		static std::expected<bool,std::string> try_parse_flag(options& opts, std::string_view arg) {
			if (detail::matches_long<LongName>(arg) || detail::matches_short<ShortName>(arg)) {
				member_t::set(opts, true);
				return true;
			}

			return false;
		}

		static std::expected<bool,std::string> try_parse_value(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			using value_type = member_t::value_type;

			if (detail::starts_with_long_equals<long_name_v>(arg)) {
				constexpr auto name = long_name_v.view();
				auto value = arg.substr(3 + name.size());

				if (value.empty()) {
					return std::unexpected(std::format("missing value after '--{}='", name));
				}

				return parse_and_assign<value_type>(opts, value);
			}

			if (detail::matches_long<long_name_v>(arg) || detail::matches_short<short_name_v>(arg)) {
				if (index + 1 >= args.size()) {
					return std::unexpected(std::format("missing value after '{}'", arg));
				}

				++index;
				return parse_and_assign<value_type>(opts, args[index]);
			}

			if (detail::starts_with_short_value<short_name_v>(arg)) {
				return parse_and_assign<value_type>(opts, arg.substr(2));
			}

			return false;
		}

		static std::expected<bool,std::string> try_parse_append(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			using vector_type = member_t::value_type;
			using value_type = vector_type::value_type;

			if (detail::starts_with_long_equals<long_name_v>(arg)) {
				constexpr auto name = long_name_v.view();
				std::string_view value = arg.substr(3 + name.size());

				if (value.empty()) {
					return std::unexpected(std::format("missing value after '--{}='", name));
				}

				return parse_and_append<value_type>(opts, value);
			}

			if (detail::matches_long<long_name_v>(arg) || detail::matches_short<short_name_v>(arg)) {
				if (index + 1 >= args.size()) {
					return std::unexpected(std::format("missing value after '{}'", arg));
				}

				++index;
				return parse_and_append<value_type>(opts, args[index]);
			}

			if (detail::starts_with_short_value<ShortName>(arg)) {
				return parse_and_append<value_type>(opts, arg.substr(2));
			}

			return false;
		}

		template <typename value_t>
		static std::expected<bool,std::string> parse_and_assign(options& opts, std::string_view text) {
			if constexpr (not choices_t::empty()) {
				if (not choices_t::contains(text)) {
					return std::unexpected(std::format("invalid value '{}' for option '--{}'", text, long_name_v.view()));
				}
			}

			if (auto value = value_parser<value_t>::parse(text)) {
				member_t::set(opts, value.value());
				return true;
			} else {
				return std::unexpected(value.error());
			}
		}

		template <typename value_t>
		static std::expected<bool,std::string> parse_and_append(options& opts, std::string_view text) {
			if constexpr (not choices_t::empty()) {
				if (not choices_t::contains(text)) {
					return std::unexpected(std::format("invalid value '{}' for option '--{}'", text, LongName.view()));
				}
			}

			if (auto value = value_parser<value_t>::parse(text)) {
				member_t::get(opts).emplace_back(std::move(value.value()));
				return true;
			} else {
				return std::unexpected(value.error());
			}

		}
	};
}

#endif // MGMAKE_CLI_OPTION_BUILDER_HXX
