#pragma once

#ifndef MGMAKE_CLI_OPTION_HXX
#define MGMAKE_CLI_OPTION_HXX

#include "value_parser.hxx"

#include "../detail/assert.hxx"
#include "../detail/index_bit.hxx"
#include "../meta/member_access.hxx"
#include "../meta/type_builder.hxx"

#include <expected>

namespace mgmake::cli {
    // Actual option impl, consume the configuration in the type map
    template<typename storage_t = meta::type_map<>>
    struct option_impl {
        MGMAKE_TYPE_CONSUMER_VALUE_FIELD(name, meta::static_string{ "" });
        MGMAKE_TYPE_CONSUMER_VALUE_FIELD(description, meta::static_string{ "" });
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(short_name, '\0');
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(callback, nullptr);
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(assign, void);
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(task, false);
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(flag, true);

		static inline constexpr bool match(std::string_view arg) {
			if (arg.empty()) {
				return false;
			}

			// If the option is a task
			if constexpr (task_value) {
				if (arg == name_value) {
					return true;
				}
			}

			// Parse as switch
			if constexpr (flag_value) {
				if (arg.starts_with("--")) {
					return match_long(arg.substr(2));
				} else if (arg.starts_with("-")) {
					return match_short(arg.substr(1));
				}
			}
			return false;
		}

		static inline constexpr bool match_long(std::string_view arg) {
			return arg.starts_with(name_value);
		}

		static inline constexpr bool match_short(std::string_view arg) {
			// handle short = val (e.g. -g=ninja/-g ninja or smth)
			return arg.size() == 1 and arg.front() == short_name_value;
		}

		static inline constexpr bool is_callback = not std::is_same_v<std::decay_t<decltype(callback_value)>, std::nullptr_t>;
		static inline constexpr std::expected<void, std::string> handle_callback(auto& opts, std::string_view arg) {
			mgmkassert(is_callback, "option_impl::handle_callback called for non-callback option");
			mgmkassert(match(arg), "handling a callback for the incorrect arg");

			if constexpr (is_callback) {
				callback_value(opts);
				return {};
			} else {
				return std::unexpected("option_impl::handle_callback called for an option with no callback");
			}
		}

		static inline constexpr auto is_assign = [] -> bool {
			if constexpr(not std::is_same_v<assign_type, void>) {
				return assign_type::valid;
			}
			return false;
		}();
		static inline constexpr std::expected<void, std::string> handle_assign(auto& opts, std::string_view arg, std::string_view value) {
			mgmkassert(match(arg), "handling a switch with the incorrect arg");
			mgmkassert(is_assign, "handling a normal switch as a value assign switch");

			if constexpr (is_assign) {
				// What is the expected value type?
				using value_type = assign_type::value_type;

				// parse it
				using vp = value_parser<value_type>;
				auto result = vp::parse(value);
				if (not result.has_value()) {
					return std::unexpected(std::format("Error parsing value for arg '{}': {}", arg, result.error()));
				}

				// assign
				assign_type::set(opts, result.value());
			}
			return {};
		}

		template<typename dispatcher_t>
		static inline constexpr std::expected<void, std::string> handle_task(auto& opts, std::string_view arg) {
			mgmkassert(match(arg), "handling a task with the incorrect arg");
			mgmkassert(task_value, "handling a normal switch as a task");

			auto matches = dispatcher_t::match(arg);
			if (not matches.any()) {
				return std::unexpected(std::format("Unknown task '{}' (cli::option_impl::handle_task no match from dispatcher_t::match for arg)", arg));
			}
			opts.m_task = detail::index_bit(matches);
			return {};
		}
    };

    // Build a compile-time map for the option settings
    template<typename builder_t = meta::type_builder<>>
    struct option_builder {
        using builder_type = builder_t;

        MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, name, meta::static_string);
        MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, description, meta::static_string);
        MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, short_name, char);
        MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, callback, auto);
		// option accepts a value (`--switch=value` or `--switch value`) and assigns its value to the option member
		// pass a `meta::member_access<>` for the member to assign.
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(option_builder, assign);
		// Sets the value at the member to the default value.
		template<typename member_t = meta::member_access<>, auto value_v = nullptr>
        using set = callback<[](auto& opts) {
			member_t::set(opts, value_v);
		}>;
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, task, bool);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, flag, bool); // aka switch

        using build = typename builder_type::template build<option_impl>;
    };
    // default builder alias
    using option = option_builder<>;
}

#endif // MGMAKE_CLI_OPTION_HXX