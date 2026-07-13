#pragma once

#ifndef MGMAKE_CLI_OPTION_HXX
#define MGMAKE_CLI_OPTION_HXX

#include "value_parser.hxx"

#include "../detail/assert.hxx"
#include "../meta/member_access.hxx"
#include "../meta/type_builder.hxx"

#include <expected>

namespace mgmake::cli {
    enum struct option_mode {
        deduce, // Auto deduce the mode based on the definition
        flag, // bool toggle on/off flag
        assign, // set text, int value, etc. assign the option value
        append, // append to the option list value. E.g. std::vector<std::string>
        callback // invoke a callback function when the option is passed
    };

    // Actual option impl, consume the configuration in the type map
    template<typename storage_t = meta::type_map<>>
    struct option_impl {
        MGMAKE_META_TYPE_CONSUMER_FIELD(name, meta::static_string{ "" });
        MGMAKE_META_TYPE_CONSUMER_FIELD(description, meta::static_string{ "" });
	    MGMAKE_META_TYPE_CONSUMER_FIELD(short_name, '\0');
	    MGMAKE_META_TYPE_CONSUMER_FIELD(mode, option_mode::deduce);
	    MGMAKE_META_TYPE_CONSUMER_FIELD(callback, nullptr);
	    using assign_type = typename storage_t::template at<meta::type_value<meta::static_string{ "assign" }>, false>;
        MGMAKE_META_TYPE_CONSUMER_FIELD(assign_hint, meta::static_string{ "value" });
	    using set_type = typename storage_t::template at<meta::type_value<meta::static_string{ "set" }>, false>;
	    MGMAKE_META_TYPE_CONSUMER_FIELD(action, false);
	    MGMAKE_META_TYPE_CONSUMER_FIELD(flag, true);

		static inline constexpr bool match(std::string_view arg) {
			if (arg.empty()) {
				return false;
			}

			// If the option is an action
			if constexpr (action_value) {
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
    };

    // Build a compile-time map for the option settings
    template<typename builder_t = meta::type_builder<>>
    struct option_builder {
        using builder_type = builder_t;

        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, name, meta::static_string);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, description, meta::static_string);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, short_name, char);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, mode, option_mode);
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, callback, auto);
		// option accepts a value (`--switch=value` or `--switch value`) and assigns its value to the option member
		// pass a `meta::member_access<>` for the member to assign.
		template<typename member_t = meta::member_access<>>
        using assign = option_builder<typename builder_type::template set<"assign", member_t>>;
        MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, assign_hint, meta::static_string);
		// Sets the value at the member to the default value.
		template<typename member_t = meta::member_access<>, auto value_v = nullptr>
        using set = callback<[](auto& opts) {
			member_t::set(opts, value_v);
		}>;
		MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, action, bool);
		MGMAKE_META_TYPE_BUILDER_FIELD(option_builder, flag, bool); // aka switch

        using build = typename builder_t::template build<option_impl>;
    };
    // default builder alias
    using option = option_builder<>;
}

#endif // MGMAKE_CLI_OPTION_HXX