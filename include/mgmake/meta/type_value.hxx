#pragma once

#ifndef MGMAKE_META_TYPE_VALUE_HXX
#define MGMAKE_META_TYPE_VALUE_HXX

// Wrap a comptime value as a type

namespace mgmake::meta {
    template<auto value_v>
    struct type_value {
        static inline constexpr auto value = value_v;
    };

    template<typename value_t, auto default_v>
    struct type_value_or {
        static constexpr auto value = value_t::value;
    };

    template<auto default_v>
    struct type_value_or<void, default_v> {
        static constexpr auto value = default_v;
    };
}

#endif // MGMAKE_META_TYPE_VALUE_HXX