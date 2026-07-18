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
		MGMAKE_TYPE_CONSUMER_TYPE_FIELD(storage_pair, void);
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD_AS(parse, meta::static_string{ "parse_value" }, false);
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(task, false);
	    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(flag, true);

		static inline constexpr bool has_storage = not std::is_same_v<storage_pair, void>;
		// The key for the storage value, else the option name
		static inline constexpr decltype(auto) storage_key() {
			if constexpr (has_storage) {
				return storage_pair::key_type::value;
			} else {
				return name_value;
			}
		}
		// The value for the storage, else void
		using storage_value_type = std::invoke_result_t<decltype([] consteval {
			if constexpr (has_storage) {
				return std::type_identity<typename storage_pair::value_type>{};
			}
		})>::type;

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
 
		static inline constexpr std::expected<void, std::string> handle_parse(auto& opts, std::string_view arg, std::string_view value) {
			mgmkassert(match(arg), "handling a switch with the incorrect arg");
			mgmkassert(parse_value, "handling a normal switch as a value parse switch");

			if constexpr (parse_value) {
				// parse the storage_value_type
				using vp = value_parser<storage_value_type>;
				auto result = vp::parse(value);
				if (not result.has_value()) {
					return std::unexpected(std::format("Error parsing value for arg '{}': {}", arg, result.error()));
				}

				// assign
				opts.template set<storage_key()>(result.value());
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
			opts.template set<"task">(detail::index_bit(matches));
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
		// Takes a `meta::type_pair<meta::static_string, value_type>` for the option value storage
		// This is what adds the key and value type to the `option_storage`
        MGMAKE_TYPE_BUILDER_TYPE_FIELD(option_builder, storage_pair);
		// If the option parses a value (`--switch=value` or `--switch value`) and stores it
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, parse_value, bool);
		// Set the option to set a specific value & assigns the storage pair
		template<meta::static_string key_v, auto value_v = std::nullopt>
        using set = callback<[](auto& opts) {
			static_assert(not std::is_same_v<decltype(value_v), std::nullopt_t>, "No value passed to `option::set<>` (Do we actually need to set the value to nullopt?)");
			opts.template set<key_v>(value_v);
		}>::template storage_pair<typename meta::type_pair<meta::type_value<key_v>, std::remove_cvref_t<decltype(value_v)>>>;
		// Set the option to parse a value & assigns the storage pair
		template<meta::static_string key_v, typename parse_t>
		using parse = parse_value<true>::template storage_pair<typename meta::type_pair<meta::type_value<key_v>, parse_t>>;
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, task, bool);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(option_builder, flag, bool); // aka switch
		// The option is only for reserving a key/value in storage
		// this disables task and flag
		template<meta::static_string key_v, typename value_t>
		using storage = typename task<false>::template flag<false>::template storage_pair<typename meta::type_pair<meta::type_value<key_v>, value_t>>;

        using build = typename builder_type::template build<option_impl>;
    };
    // default builder alias
    using option = option_builder<>;
}

#endif // MGMAKE_CLI_OPTION_HXX