#pragma once

#ifndef MGMAKE_META_TYPE_BUILDER_HXX
#define MGMAKE_META_TYPE_BUILDER_HXX

#include "../meta/static_string.hxx"
#include "../meta/type_map.hxx"
#include "../meta/type_value.hxx"

namespace mgmake::meta {
    template<typename storage_t = type_map<>>
    struct type_builder {
		using storage_type = storage_t;
		
        template<static_string key_v, bool check_v = true>
        using get = typename storage_t::template at<type_value<key_v>, check_v>;

        template<static_string key_v, typename value_t>
        using set = type_builder<typename storage_t::template emplace<type_value<key_v>, value_t>>;

        template<template<typename> typename consumer_t>
        using build = consumer_t<storage_t>;
    };

	template<typename builder_t>
	concept has_builder_alias = requires {
		typename builder_t::build;
	};
	template<auto builder_v>
	concept has_builder_fn = requires {
		{ builder_v.build(); }
	};
}

// When defining builder fields, ensure `builder_t` is the name of the `meta::type_builder`
#define MGMAKE_TYPE_BUILDER_TYPE_FIELD(wrapper_t, alias_t) \
	MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS( \
		wrapper_t, \
		alias_t, \
		::mgmake::meta::static_string{#alias_t} \
	)

#define MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(wrapper_t, alias_t, key_v) \
	template<typename alias_t##_t> \
	using alias_t = wrapper_t< \
		typename builder_t::template set<key_v, alias_t##_t> \
	>

#define MGMAKE_TYPE_BUILDER_VALUE_FIELD(wrapper_t, alias_t, value_t) \
	MGMAKE_TYPE_BUILDER_VALUE_FIELD_AS( \
		wrapper_t, \
		alias_t, \
		value_t, \
		::mgmake::meta::static_string{#alias_t} \
	)

#define MGMAKE_TYPE_BUILDER_VALUE_FIELD_AS(wrapper_t, alias_t, value_t, key_v) \
	MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS( \
		wrapper_t, \
		alias_t##_type, \
		key_v \
	); \
	template<value_t alias_t##_v> \
	using alias_t = alias_t##_type< \
		::mgmake::meta::type_value<alias_t##_v> \
	>

#define MGMAKE_TYPE_CONSUMER_TYPE_FIELD(alias_t, default_t) \
	MGMAKE_TYPE_CONSUMER_TYPE_FIELD_AS(alias_t, ::mgmake::meta::static_string{#alias_t}, default_t)

#define MGMAKE_TYPE_CONSUMER_TYPE_FIELD_AS(alias_t, key_v, default_t) \
	using alias_t##_type = typename storage_t::template at< \
		::mgmake::meta::type_value<key_v>, \
		false \
	>; \
	using alias_t = std::conditional_t< \
		std::same_as<alias_t##_type, void>, \
		default_t, \
		alias_t##_type \
	>

#define MGMAKE_TYPE_CONSUMER_VALUE_FIELD(alias_t, default_v) \
	MGMAKE_TYPE_CONSUMER_VALUE_FIELD_AS( \
		alias_t, \
		::mgmake::meta::static_string{#alias_t}, \
		default_v \
	)

#define MGMAKE_TYPE_CONSUMER_VALUE_FIELD_AS(alias_t, key_v, default_v) \
	MGMAKE_TYPE_CONSUMER_TYPE_FIELD_AS( \
		alias_t##_type, \
		key_v, \
		::mgmake::meta::type_value<default_v> \
	); \
	static inline constexpr auto alias_t##_value = alias_t##_type::value

#endif // MGMAKE_META_TYPE_BUILDER_HXX