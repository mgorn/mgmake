#pragma once

#ifndef MGMAKE_CLI_OPTION_HXX
#define MGMAKE_CLI_OPTION_HXX

#include "value_parser.hxx"

#include "../detail/assert.hxx"
#include "../detail/index_bit.hxx"
#include "../meta/builder_mixin.hxx"
#include "../meta/member_access.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/type_traits.hxx"

#include <expected>

namespace mgmake::cli {
    // Actual configurable option impl
    template<typename storage_t = meta::type_map<>>
    struct option_impl : public meta::type_builder<option_impl, storage_t>, public meta::named<option_impl<storage_t>> {
		using builder_type = meta::type_builder<option_impl, storage_t>;

		using builder_type::get_type_or;

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto alias() {
			return builder_type::template set_str<"alias", value_v>();
		}
		static consteval auto alias() {
			return builder_type::template get_str<"alias">();
		}

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto description() {
			return builder_type::template set_str<"description", value_v>();
		}
		static consteval auto description() {
			return builder_type::template get_str<"description">();
		}

		template<char value_v>
		[[nodiscard]] static consteval auto short_name() {
			return builder_type::template set_value<"short_name", value_v>();
		}
		static consteval char short_name() {
			return builder_type::template get_value_or<"short_name", '\0'>();
		}

		template<auto value_v>
		[[nodiscard]] static consteval auto callback() {
			return builder_type::template set_value<"callback", value_v>();
		}
		static consteval auto callback() {
			return builder_type::template get_value_or<"callback", nullptr>();
		}

		// Takes a `meta::type_pair<meta::static_string, value_type>` for the option value storage
		// This is what adds the key and value type to the `option_storage`
		template<typename pair_t>
		[[nodiscard]] static consteval auto storage_pair() -> typename builder_type::template set_type<"storage_pair", pair_t> {
			return {};
		}
		static consteval auto storage_pair() -> typename builder_type::template get_type<"storage_pair", false> {
			return {};
		}

		// Set the option to set a specific value & assigns the storage pair
		template<meta::static_string key_v, auto value_v = std::nullopt>
        [[nodiscard]] static consteval auto set() {
			return callback<[](auto& opts) {
				static_assert(not std::is_same_v<decltype(value_v), std::nullopt_t>, "No value passed to `option::set<>` (Do we actually need to set the value to nullopt?)");
				opts.template set<key_v>(value_v);
			}>().template storage_pair<typename meta::type_pair<meta::type_value<key_v>, std::remove_cvref_t<decltype(value_v)>>>();
		}

		// If the option parses a value (`--switch=value` or `--switch value`) and stores it
		template<bool value_v>
		[[nodiscard]] static consteval auto parses() {
			return builder_type::template set_value<"parses", value_v>();
		}
		static consteval bool parses() {
			return builder_type::template get_value_or<"parses", false>();
		}

		// Set the option to parse a value & assigns the storage pair
		template<meta::static_string key_v, typename parse_t>
		[[nodiscard]] static consteval auto parse() {
			return parses<true>().template storage_pair<typename meta::type_pair<meta::type_value<key_v>, parse_t>>();
		}

		template<bool value_v>
		[[nodiscard]] static consteval auto task() {
			return builder_type::template set_value<"task", value_v>();
		}
		static consteval bool task() {
			return builder_type::template get_value_or<"task", false>();
		}

		// aka switch
		template<bool value_v>
		[[nodiscard]] static consteval auto flag() {
			return builder_type::template set_value<"flag", value_v>();
		}
		static consteval bool flag() {
			return builder_type::template get_value_or<"flag", true>();
		}

		// The option is only for reserving a key/value in storage
		// this disables task and flag
		template<meta::static_string key_v, typename value_t>
		[[nodiscard]] static consteval auto storage() {
			return task<false>()
				.template flag<false>()
				.template storage_pair<typename meta::type_pair<meta::type_value<key_v>, value_t>>();
		}

		static inline constexpr bool has_storage = not std::is_same_v<decltype(storage_pair()), void>;
		// The key for the storage value, else the option name
		static inline constexpr decltype(auto) storage_key() {
			if constexpr (has_storage) {
				return decltype(storage_pair())::key_type::value;
			} else {
				return option_impl{}.name();
			}
		}
		// The value for the storage, else void
		using storage_value_type = std::invoke_result_t<decltype([] consteval {
			if constexpr (has_storage) {
				return std::type_identity<typename decltype(storage_pair())::value_type>{};
			} else {
				return std::type_identity<void>{};
			}
		})>::type;

		static inline constexpr bool match(std::string_view arg) {
			if (arg.empty()) {
				return false;
			}

			// If the option is a task
			if constexpr (task()) {
				if (arg == option_impl{}.name()) {
					return true;
				}
			}

			// Parse as switch
			if constexpr (flag()) {
				if (arg.starts_with("--")) {
					return match_long(arg.substr(2));
				} else if (arg.starts_with("-")) {
					return match_short(arg.substr(1));
				}
			}
			return false;
		}

		static inline constexpr bool match_long_name(std::string_view arg, std::string_view name) {
			return arg == name or (arg.size() > name.size() and arg.starts_with(name) and arg[name.size()] == '=');
		}

		static inline constexpr bool match_long(std::string_view arg) {
			if (match_long_name(arg, option_impl{}.name())) {
				return true;
			}

			if constexpr (not alias().empty()) {
				return match_long_name(arg, alias());
			}
			return false;
		}

		static inline constexpr bool match_short(std::string_view arg) {
			// handle short = val (e.g. -g=ninja/-g ninja or smth)
			return arg.size() == 1 and arg.front() == short_name();
		}

		static inline constexpr bool is_callback = not std::is_same_v<std::decay_t<decltype(callback())>, std::nullptr_t>;
		static inline constexpr std::expected<void, std::string> handle_callback(auto& opts, std::string_view arg) {
			mgmkassert(is_callback, "option_impl::handle_callback called for non-callback option");
			mgmkassert(match(arg), "handling a callback for the incorrect arg");

			if constexpr (is_callback) {
				callback()(opts);
				return {};
			} else {
				return std::unexpected("option_impl::handle_callback called for an option with no callback");
			}
		}
 
		static inline constexpr std::expected<void, std::string> handle_parse(auto& opts, std::string_view arg, std::string_view value) {
			mgmkassert(match(arg), "handling a switch with the incorrect arg");
			mgmkassert(parses(), "handling a normal switch as a value parse switch");

			if constexpr (parses()) {
				// parse the storage_value_type
				using vp = value_parser<storage_value_type>;
				auto result = vp::parse(value);
				if (not result.has_value()) {
					return std::unexpected(std::format("Error parsing value for arg '{}': {}", arg, result.error()));
				}

				// If its a vector, emplace back
				if constexpr (meta::is_vector_v<storage_value_type>) {
					opts.template get<storage_key()>().emplace_back(result.value());
				} else {
					// otherwise just assign
					opts.template set<storage_key()>(result.value());
				}
			}
			return {};
		}

		template<typename dispatcher_t>
		static inline constexpr std::expected<void, std::string> handle_task(auto& opts, std::string_view arg) {
			mgmkassert(match(arg), "handling a task with the incorrect arg");
			mgmkassert(task(), "handling a normal switch as a task");

			auto matches = dispatcher_t::match(arg);
			if (not matches.any()) {
				return std::unexpected(std::format("Unknown task '{}' (cli::option_impl::handle_task no match from dispatcher_t::match for arg)", arg));
			}
			opts.template set<"task">(detail::index_bit(matches));
			return {};
		}
    };
    // default builder alias
    static constexpr auto option = option_impl<>{};
}

#endif // MGMAKE_CLI_OPTION_HXX