#pragma once

#ifndef MGMAKE_CLI_OPTION_STORAGE_HXX
#define MGMAKE_CLI_OPTION_STORAGE_HXX

#include "../meta/static_dict.hxx"
#include "../meta/value_list.hxx"

#include <type_traits>
#include <utility>

namespace mgmake::cli {
	// Type list of all options for the build program
	template<typename opts_t = meta::value_list<>>
	struct option_storage {
		using list_type = opts_t;

		// The type_map for the option storage key/value pairs
		using storage_map_type = typename list_type::template fold<[]<typename state_t, auto opt_v>() consteval {
			// If the option uses storage, it has a storage pair
			if constexpr (opt_v.has_storage) {
				// Get the pair
				using pair_type = decltype(opt_v.storage_pair());
				// Get the key
				using key_type = pair_type::key_type;
				// Get the value type
				using value_type = pair_type::value_type;

				// If the value_type is void, then it is determined by another option (typically a storage option)
				if constexpr (not std::is_same_v<value_type, void>) {
					// When emplacing a key, ensure an existing key has the same value type.
					if constexpr (state_t::template has<key_type>()) {
						using in_map_type = typename state_t::template at<key_type>;
						static_assert(std::is_same_v<value_type, in_map_type>, "option storage key value type mismatch!");
						return std::type_identity<state_t>{};
					} else {
						return std::type_identity<typename state_t::template emplace_unique<key_type, value_type>>{};
					}
				} else {
					return std::type_identity<state_t>{};
				}
			} else {
				// The option doesn't have storage, ignore it
				return std::type_identity<state_t>{};
			}
		}, meta::type_map<>>;

		// Ensure deferred `void` storage pairs resolve to a concrete key supplied by another option.
		using storage_validation = typename list_type::template fold<[]<typename state_t, auto opt_v>() consteval {
			if constexpr (opt_v.has_storage) {
				using pair_type = decltype(opt_v.storage_pair());
				if constexpr (std::is_same_v<typename pair_type::value_type, void>) {
					static_assert(storage_map_type::template has<typename pair_type::key_type>(), "void option storage value requires another option to define the key's value type");
				}
			}
			return std::type_identity<state_t>{};
		}, meta::type_list<>>;
		static_assert(std::is_same_v<storage_validation, meta::type_list<>>);

		// The storage type for the option values
		using storage_type = meta::static_dict<storage_map_type>;

		template<meta::static_string key_v>
		static consteval decltype(auto) has() {
			return storage_type::template has<key_v>();
		}

		template<meta::static_string key_v>
		constexpr decltype(auto) get() {
			return m_storage.template get<key_v>();
		}
		template<meta::static_string key_v>
		constexpr decltype(auto) get() const {
			return m_storage.template get<key_v>();
		}
		template<meta::static_string key_v>
		constexpr void set(auto&& value) {
			return m_storage.template set<key_v>(std::forward<decltype(value)>(value));
		}

		// Overloads for option types
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		static inline consteval decltype(auto) has() {
			return has<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr decltype(auto) get() {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template get<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr decltype(auto) get() const {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template get<opt_v.storage_key()>();
		}
		template<auto opt_v> requires requires { opt_v.storage_key(); }
		constexpr void set(auto&& value) {
			static_assert(has<opt_v>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template set<opt_v.storage_key()>(std::forward<decltype(value)>(value));
		}

		constexpr option_storage() {
			list_type::for_each([&]<auto opt_v> constexpr {
				if constexpr (opt_v.has_storage) {
					if constexpr (not std::is_same_v<decltype(opt_v.default_value()), std::nullopt_t>) {
						constexpr auto key = opt_v.storage_key();
						set<key>(opt_v.default_value());
					}
				}
			});
		}
		
	private:
		storage_type m_storage{};
	};
}

#endif // MGMAKE_CLI_OPTION_STORAGE_HXX
