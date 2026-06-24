#pragma once

#ifndef MGMAKE_DETAIL_ENUM_FLAG_HXX
#define MGMAKE_DETAIL_ENUM_FLAG_HXX

#include "enum.hxx"

#include <bitset>
#include <concepts>
#include <cstddef>

// enum_flag wraps bitmask-style enums while preserving type safety around underlying integer operations.

namespace mgmake::detail {
    template <typename enum_t>
    struct enum_flag_traits;

    template <enum_type enum_t> requires requires { enum_t::count; }
    struct enum_flag_traits<enum_t> {
        static constexpr std::size_t count =
            static_cast<std::size_t>(enum_underlying(enum_t::count));
    };

    template <enum_type enum_t>
    struct enum_flag {
        using value_type = enum_t;
        using traits_type = enum_flag_traits<enum_t>;

        static constexpr std::size_t count = traits_type::count;

        static_assert(count > 0, "mgmake enum_flag requires at least one flag");

        std::bitset<count> m_flags{};

        constexpr enum_flag() noexcept = default;

        constexpr enum_flag(enum_t flag) {
            set(flag);
        }

        template <typename... flags_t>
            requires((std::same_as<flags_t, enum_t> && ...))
        constexpr enum_flag(enum_t first, flags_t... rest) {
            set(first);
            (set(rest), ...);
        }

        [[nodiscard]] static constexpr enum_flag none() noexcept {
            return {};
        }

        [[nodiscard]] static constexpr enum_flag all_flags() {
            enum_flag result{};

            result.m_flags.set();

            return result;
        }

        [[nodiscard]] static constexpr std::size_t index(enum_t flag) noexcept {
            return static_cast<std::size_t>(enum_underlying(flag));
        }

        [[nodiscard]] static constexpr bool valid(enum_t flag) noexcept {
            if (enum_underlying_is_negative(flag)) {
                return false;
            }

            return index(flag) < count;
        }

        constexpr enum_flag& set(enum_t flag, bool value = true) {
            m_flags.set(index(flag), value);

            return *this;
        }

        constexpr enum_flag& reset(enum_t flag) {
            m_flags.reset(index(flag));

            return *this;
        }

        constexpr enum_flag& toggle(enum_t flag) {
            m_flags.flip(index(flag));

            return *this;
        }

        constexpr enum_flag& set(enum_flag flags) noexcept {
            m_flags |= flags.m_flags;

            return *this;
        }

        constexpr enum_flag& reset(enum_flag flags) noexcept {
            m_flags &= ~flags.m_flags;

            return *this;
        }

        constexpr enum_flag& toggle(enum_flag flags) noexcept {
            m_flags ^= flags.m_flags;

            return *this;
        }

        [[nodiscard]] constexpr bool has(enum_t flag) const {
            return m_flags.test(index(flag));
        }

        [[nodiscard]] constexpr bool contains(enum_flag flags) const noexcept {
            return (m_flags & flags.m_flags) == flags.m_flags;
        }

        [[nodiscard]] constexpr bool any(enum_flag flags) const noexcept {
            return (m_flags & flags.m_flags).any();
        }

        [[nodiscard]] constexpr bool any() const noexcept {
            return m_flags.any();
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return m_flags.none();
        }

        [[nodiscard]] constexpr bool all() const noexcept {
            return m_flags.all();
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return any();
        }

        [[nodiscard]] constexpr enum_flag operator|(enum_flag other) const noexcept {
            enum_flag result{};

            result.m_flags = m_flags | other.m_flags;

            return result;
        }

        [[nodiscard]] constexpr enum_flag operator&(enum_flag other) const noexcept {
            enum_flag result{};

            result.m_flags = m_flags & other.m_flags;

            return result;
        }

        [[nodiscard]] constexpr enum_flag operator^(enum_flag other) const noexcept {
            enum_flag result{};

            result.m_flags = m_flags ^ other.m_flags;

            return result;
        }

        [[nodiscard]] constexpr enum_flag operator~() const noexcept {
            enum_flag result{};

            result.m_flags = ~m_flags;

            return result;
        }

        constexpr enum_flag& operator|=(enum_flag other) noexcept {
            m_flags |= other.m_flags;

            return *this;
        }

        constexpr enum_flag& operator&=(enum_flag other) noexcept {
            m_flags &= other.m_flags;

            return *this;
        }

        constexpr enum_flag& operator^=(enum_flag other) noexcept {
            m_flags ^= other.m_flags;

            return *this;
        }

        [[nodiscard]] friend constexpr bool operator==(
            enum_flag lhs,
            enum_flag rhs
        ) noexcept = default;
    };

    template <enum_type enum_t>
    [[nodiscard]] inline constexpr enum_flag<enum_t> operator|(
        enum_flag<enum_t> lhs,
        enum_t rhs
    ) {
        lhs.set(rhs);

        return lhs;
    }

    template <enum_type enum_t>
    [[nodiscard]] inline constexpr enum_flag<enum_t> operator|(
        enum_t lhs,
        enum_flag<enum_t> rhs
    ) {
        rhs.set(lhs);

        return rhs;
    }

    template <enum_type enum_t>
    [[nodiscard]] inline constexpr enum_flag<enum_t> operator&(
        enum_flag<enum_t> lhs,
        enum_t rhs
    ) {
        return lhs & enum_flag<enum_t>{ rhs };
    }

    template <enum_type enum_t>
    [[nodiscard]] inline constexpr enum_flag<enum_t> operator&(
        enum_t lhs,
        enum_flag<enum_t> rhs
    ) {
        return enum_flag<enum_t>{ lhs } & rhs;
    }
}

#endif // MGMAKE_DETAIL_ENUM_FLAG_HXX
