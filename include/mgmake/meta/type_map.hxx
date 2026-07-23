#pragma once

#ifndef MGMAKE_META_TYPE_MAP_HXX
#define MGMAKE_META_TYPE_MAP_HXX

#include "type_list.hxx"
#include "type_pair.hxx"

#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>

namespace mgmake::meta {
	template<typename storage_t = type_list<>>
	struct type_map;

	namespace type_map_detail {
		template<typename map_t, typename key_t, bool check, bool exists = map_t::template has<key_t>()>
		struct at_type;

		template<typename map_t, typename key_t, bool check>
		struct at_type<map_t, key_t, check, true> {
			using pair_type = typename map_t::storage_type::template type_at<map_t::template key_index<key_t>()>;
			using type = typename pair_type::value_type;
		};

		template<typename map_t, typename key_t, bool check>
		struct at_type<map_t, key_t, check, false> {
			static_assert((not check) or map_t::template has<key_t>(), "type_map::at<key_t> requires key_t to exist.");

			using type = void;
		};

		template<typename pair_t, typename key_t, typename value_t>
		using replace_pair = std::conditional_t<std::same_as<key_t, typename pair_t::key_type>, type_pair<key_t, value_t>, pair_t>;

		template<typename map_t, typename key_t, typename value_t>
		struct emplace_type;

		template<typename... pair_ts, typename key_t, typename value_t>
		struct emplace_type<type_map<type_list<pair_ts...>>, key_t, value_t> {
			using map_type = type_map<type_list<pair_ts...>>;
			using type = std::conditional_t<
				map_type::template has<key_t>(),
				type_map<type_list<replace_pair<pair_ts, key_t, value_t>...>>,
				type_map<typename map_type::storage_type::template append<type_pair<key_t, value_t>>>
			>;
		};

		template<typename map_t, typename key_t, typename value_t>
		struct emplace_unique_type {
			static_assert(not map_t::template has<key_t>(), "type_map::emplace_unique<key_t, value_t> cannot emplace a duplicate key.");

			using type = type_map<typename map_t::storage_type::template append<type_pair<key_t, value_t>>>;
		};
	}

	template<typename... pair_ts>
	struct type_map<type_list<pair_ts...>> {
		using storage_type = type_list<pair_ts...>;
		using key_types = type_list<typename pair_ts::key_type...>;
		static_assert((key_types::template unique<typename pair_ts::key_type>() and ...), "type_map storage cannot contain duplicate keys.");
		using value_types = type_list<typename pair_ts::value_type...>;
		
		static consteval std::size_t size() {
			return storage_type::size();
		}

		template<typename key_t>
		static consteval std::size_t key_index() {
			constexpr std::array<bool, size()> matches {
				std::same_as<key_t, typename pair_ts::key_type>...
			};

			for (std::size_t i = 0; i < matches.size(); ++i) {
				if (matches.at(i)) {
					return i;
				}
			}

			return size();
		}

		template<typename key_t>
		static consteval bool has() {
			return key_index<key_t>() < size();
		}

		template<typename key_t, bool check = true>
		using at = typename type_map_detail::at_type<type_map<type_list<pair_ts...>>, key_t, check>::type;

		template<typename key_t, typename value_t>
		using emplace = typename type_map_detail::emplace_type<type_map<type_list<pair_ts...>>, key_t, value_t>::type;

		template<typename key_t, typename value_t>
		using emplace_unique = typename type_map_detail::emplace_unique_type<type_map<type_list<pair_ts...>>, key_t, value_t>::type;
	};
}

#endif // MGMAKE_META_TYPE_MAP_HXX
