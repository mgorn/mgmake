#pragma once

#ifndef MGMAKE_META_TYPE_BUILDER_HXX
#define MGMAKE_META_TYPE_BUILDER_HXX

#include "../meta/static_string.hxx"
#include "../meta/type_map.hxx"
#include "../meta/type_or.hxx"
#include "../meta/type_value.hxx"

namespace mgmake::meta {
    template<template<typename = type_map<>> typename impl_t, typename storage_t = type_map<>>
    struct type_builder {
		using storage_type = storage_t;

		template<static_string key_v>
		static consteval bool has() {
			return storage_type::template has<type_value<key_v>>();
		}

        template<static_string key_v, bool check_v = true>
        using get_type = typename storage_t::template at<type_value<key_v>, check_v>;
		template<static_string key_v, typename default_t>
		using get_type_or = type_or_t<get_type<key_v, false>, default_t>;
        template<static_string key_v>
		static consteval auto get_value() {
			return get_type<key_v>::value;
		}
		template<static_string key_v, auto default_v = nullptr>
		static consteval auto get_value_or() {
			if constexpr (has<key_v>()) {
				return get_value<key_v>();
			} else {
				return default_v;
			}
		}
		template<static_string key_v>
		static consteval auto get_str() {
			return get_value_or<key_v, static_string{ "" }>();
		}

        template<static_string key_v, typename value_t>
        using set_type = impl_t<typename storage_t::template emplace<type_value<key_v>, value_t>>;
		template<static_string key_v, auto value_v>
		[[nodiscard]] static consteval auto set_value() {
			return set_type<key_v, type_value<value_v>>{};
		}
		template<static_string key_v, static_string str_v>
		[[nodiscard]] static consteval auto set_str() {
			return set_type<key_v, type_value<str_v>>{};
		}
    };
}

#endif // MGMAKE_META_TYPE_BUILDER_HXX
