#pragma once

#ifndef MGMAKE_META_TYPE_BUILDER_HXX
#define MGMAKE_META_TYPE_BUILDER_HXX

#include "../meta/static_string.hxx"
#include "../meta/type_map.hxx"
#include "../meta/type_value.hxx"

namespace mgmake::meta {
    template<typename storage_t = type_map<>>
    struct type_builder {
        template<static_string key_v>
        using get = typename storage_t::template at<type_value<key_v>>;

        template<static_string key_v, auto value_v>
        using set = type_builder<typename storage_t::template emplace<type_value<key_v>, type_value<value_v>>>;

        template<template<typename> typename consumer_t>
        using build = typename consumer_t<storage_t>::build;
    };
}

// When defining builder fields, ensure `builder_t` is the name of the `meta::type_builder`
#define MGMAKE_META_TYPE_BUILDER_FIELD(wrapper_t, alias_t, ...) \
	MGMAKE_META_TYPE_BUILDER_FIELD_AS(wrapper_t, alias_t, ::mgmake::meta::static_string{ #alias_t }, __VA_ARGS__)

#define MGMAKE_META_TYPE_BUILDER_FIELD_AS(wrapper_t, alias_t, key_v, ...) \
	template<__VA_ARGS__ alias_t##_v> \
	using alias_t = wrapper_t< \
		typename builder_t::template set<key_v, alias_t##_v> \
	>

// When defining consumers, ensure `storage_t` is the name of the `meta::type_map`
#define MGMAKE_META_TYPE_CONSUMER_FIELD(alias_t, default_v) \
    MGMAKE_META_TYPE_CONSUMER_FIELD_AS(alias_t, ::mgmake::meta::static_string{ #alias_t }, default_v)

#define MGMAKE_META_TYPE_CONSUMER_FIELD_AS(alias_t, key_v, default_v) \
    using alias_t##_type = typename storage_t::template at< \
        ::mgmake::meta::type_value<key_v>, \
        false \
    >; \
    static constexpr auto alias_t##_value = \
        ::mgmake::meta::type_value_or<alias_t##_type, default_v>::value

#endif // MGMAKE_META_TYPE_BUILDER_HXX