#pragma once

#ifndef MGMAKE_META_STATIC_DICT_HXX
#define MGMAKE_META_STATIC_DICT_HXX

#include "static_string.hxx"
#include "type_map.hxx"
#include "type_pair.hxx"
#include "type_value.hxx"

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mgmake::meta {
	//md
	// `map_t = meta::type_map<meta::type_list<meta::type_pair<meta::type_value<key>, value>>>`
	// `key` = A `meta::static_string` value wrapped as `meta::type_value<key>`
	// `value` = The type for the value at that key
	// The actual `value`s are runtime, `key`s are known at compile time
	template<typename map_t = meta::type_map<>>
	struct static_dict {
		using map_type = map_t;
		using pairs_type = typename map_type::storage_type;
		// The types for keys (the `meta::type_value`s of `meta::static_string`s)
		using key_types = typename map_type::key_types;
		// The types for the values
		using value_types = typename map_type::value_types;

		// Store the values in a `std::tuple`
		using storage_type = typename value_types::template apply<std::tuple>;

		// Does the key exist?
		template<meta::static_string key_v>
		static consteval bool has() {
			using key_t = meta::type_value<key_v>;
			return map_type::template has<key_t>();
		}

		// Get the storage index for a key
		template<meta::static_string key_v>
		static consteval std::size_t key_index() {
			static_assert(has<key_v>(), "static_dict key doesn't exist");
			using key_t = meta::type_value<key_v>;
			return map_type::template key_index<key_t>();
		}

		// Reference to the stored value
		template<meta::static_string key_v>
		constexpr decltype(auto) storage() {
			constexpr std::size_t index = key_index<key_v>();
			return std::get<index>(m_storage);
		}
		template<meta::static_string key_v>
		constexpr decltype(auto) storage() const {
			constexpr std::size_t index = key_index<key_v>();
			return std::get<index>(m_storage);
		}

		// Accessors
		template<meta::static_string key_v>
		constexpr decltype(auto) get() {
			return storage<key_v>();
		}
		template<meta::static_string key_v>
		constexpr decltype(auto) get() const {
			return storage<key_v>();
		}

		// Setters
		template<meta::static_string key_v>
		constexpr void set(auto&& value) {
			static_assert(std::is_assignable_v<decltype(storage<key_v>()), decltype(value)>, "Cannot assign value at key with given value");
			storage<key_v>() = std::forward<decltype(value)>(value);
		}

		storage_type m_storage{};

	private:
		template<typename>
		static constexpr bool invalid_entry_v = false;

		template<typename pair_t>
		struct entry_check {
			static_assert(invalid_entry_v<pair_t>, "static_dict keys must be meta::type_value<meta::static_string>");
		};

		template<meta::static_string key_v, typename value_t>
		struct entry_check<meta::type_pair<meta::type_value<key_v>, value_t>> {
			static_assert(std::is_default_constructible_v<value_t>, "static_dict value type must be default-constructible");
			using type = void;
		};

		using entry_checks = typename pairs_type::template fold<[]<typename state_t, typename pair_t>() consteval {
			using check_t = entry_check<pair_t>;

			// Forces validation of this key/value pair.
			static_assert(std::is_same_v<typename check_t::type, void>);

			return std::type_identity<state_t>{};
		}, meta::type_list<>>;
		static_assert(std::is_same_v<entry_checks, meta::type_list<>>);
	};
}

#endif // MGMAKE_META_STATIC_DICT_HXX