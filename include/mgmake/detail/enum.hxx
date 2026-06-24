#pragma once

#ifndef MGMAKE_DETAIL_ENUM_HXX
#define MGMAKE_DETAIL_ENUM_HXX

#include <type_traits>

// Shared enum constraints and helpers keep enum-string and enum-flag utilities consistent.

namespace mgmake::detail {
    template <typename enum_t>
    concept enum_type = std::is_enum_v<enum_t>;

    template <enum_type enum_t>
    using enum_underlying_t = std::underlying_type_t<enum_t>;

    template <enum_type enum_t>
    [[nodiscard]] inline constexpr enum_underlying_t<enum_t> enum_underlying(
        enum_t value
    ) noexcept {
        return static_cast<enum_underlying_t<enum_t>>(value);
    }

    template <enum_type enum_t>
    [[nodiscard]] inline constexpr bool enum_underlying_is_negative(
        enum_t value
    ) noexcept {
        using underlying_t = enum_underlying_t<enum_t>;

        if constexpr (std::is_signed_v<underlying_t>) {
            return enum_underlying(value) < 0;
        } else {
            return false;
        }
    }
}

#endif // MGMAKE_DETAIL_ENUM_HXX
