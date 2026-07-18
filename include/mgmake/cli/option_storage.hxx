#pragma once

#ifndef MGMAKE_CLI_OPTION_STORAGE_HXX
#define MGMAKE_CLI_OPTION_STORAGE_HXX

#include "../meta/static_dict.hxx"
#include "../meta/type_list.hxx"

namespace mgmake::cli {
	// Type list of all options for the build program
	template<typename opts_t = meta::type_list<>>
	struct option_storage {
		using list_type = opts_t;

	private:
		// When emplacing a key: this ensures if the key already exists, the types match
		// If they don't, a static_assert is thrown
		// If they do, the key was already added and can be skipped
		template<typename key_t, typename value_t, typename map_t>
		struct emplace_key_checked {
			using type = std::invoke_result_t<decltype([] consteval {
				// If the key exists
				if constexpr (map_t::template has<key_t>()) {
					// Get the type
					using in_map_type = typename map_t::template at<key_t>;
					// Assert they are the same
					static_assert(std::is_same_v<value_t, in_map_type>, "option storage key value type mismatch!");
					return std::type_identity<map_t>{};
				} else {
					return std::type_identity<typename map_t::template emplace_unique<key_t, value_t>>{};
				}
			})>::type;
		};
		//template<typename key_t, typename value_t, typename map_t>
		//using emplace_key_checked_t = emplace_key_checked<key_t, value_t, map_t>::type;

	public:
		// The type_map for the option storage key/value pairs
		using storage_map_type = typename list_type::template fold<[]<typename state_t, typename opt_t>() consteval {
			// If the option uses storage, it has a storage pair
			if constexpr (opt_t::has_storage) {
				// Get the pair
				using pair_type = opt_t::storage_pair;
				// Get the key
				using key_type = pair_type::key_type;
				// Get the value type
				using value_type = pair_type::value_type;

				// If the value_type is void, then it is determined by another option (typically a storage option)
				if constexpr (not std::is_same_v<value_type, void>) {
					// Check the key value & emplace it
					return std::type_identity<typename emplace_key_checked<key_type, value_type, state_t>::type>{};
				} else {
					// TODO: assert the key exists after constructing the full map
					return std::type_identity<state_t>{};
				}

			} else {
				// The option doesn't have storage, ignore it
				return std::type_identity<state_t>{};
			}
		}, meta::type_map<>>;

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
			return m_storage.template set<key_v>(value);
		}

		// Overloads for option types
		template<typename opt_t>
		static inline consteval decltype(auto) has() {
			return has<opt_t::storage_key()>();
		}
		template<typename opt_t>
		constexpr decltype(auto) get() {
			static_assert(has<opt_t>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template get<opt_t::storage_key()>();
		}
		template<typename opt_t>
		constexpr decltype(auto) get() const {
			static_assert(has<opt_t>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template get<opt_t::storage_key()>();
		}
		template<typename opt_t>
		constexpr void set(auto&& value) {
			static_assert(has<opt_t>(), "Missing storage key for option (was the option added to your options in your config?)");
			return this->template set<opt_t::storage_key()>(value);
		}

		// Value storage
		storage_type m_storage{};
	};
}

#endif // MGMAKE_CLI_OPTION_STORAGE_HXX