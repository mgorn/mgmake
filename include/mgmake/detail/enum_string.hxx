#pragma once

#ifndef MGMAKE_DETAIL_ENUM_STRING_HXX
#define MGMAKE_DETAIL_ENUM_STRING_HXX

#include "static_string.hxx"

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace mgmake::detail {
    template <typename E>
    concept enum_type = std::is_enum_v<E>;

    template <enum_type E>
    [[nodiscard]] inline constexpr auto enum_underlying(E value) noexcept {
        return static_cast<std::underlying_type_t<E>>(value);
    }

    template <enum_type E>
    [[nodiscard]] inline constexpr bool enum_underlying_is_negative(E value) noexcept {
        using U = std::underlying_type_t<E>;

        if constexpr (std::is_signed_v<U>) {
            return enum_underlying(value) < 0;
        } else {
            return false;
        }
    }

    template <enum_type E>
    struct enum_view_entry {
        E m_value{};
        std::string_view m_name{};
    };

    template <auto Value, static_string Name> requires enum_type<std::remove_cv_t<decltype(Value)>>
    struct enum_entry {
        using value_type = std::remove_cv_t<decltype(Value)>;

        static constexpr value_type m_value = Value;
        static constexpr auto m_name = Name;

        [[nodiscard]] static constexpr value_type value() noexcept {
            return m_value;
        }

        [[nodiscard]] static constexpr std::string_view name() noexcept {
            return m_name.view();
        }

        [[nodiscard]] static consteval std::size_t name_size() noexcept {
            return m_name.size();
        }

        [[nodiscard]] static consteval bool name_empty() noexcept {
            return m_name.empty();
        }
    };

    template <enum_type E, typename... Entries>
    struct enum_table {
        using value_type = E;

        static_assert(
            (std::same_as<typename Entries::value_type, E> && ...),
            "mgmake enum_table entries must all use the table enum type"
        );

        [[nodiscard]] static consteval std::size_t size() noexcept {
            return sizeof...(Entries);
        }

        [[nodiscard]] static constexpr auto values() noexcept {
            return std::array<E, sizeof...(Entries)>{
                Entries::value()...
            };
        }

        [[nodiscard]] static constexpr auto names() noexcept {
            return std::array<std::string_view, sizeof...(Entries)>{
                Entries::name()...
            };
        }

        [[nodiscard]] static constexpr auto entries() noexcept {
            return std::array<enum_view_entry<E>, sizeof...(Entries)>{
                enum_view_entry<E>{ Entries::value(), Entries::name() }...
            };
        }

        [[nodiscard]] static constexpr bool contains_value(E value) noexcept {
            return ((Entries::value() == value) || ...);
        }

        [[nodiscard]] static constexpr bool contains_name(std::string_view name) noexcept {
            return ((Entries::name() == name) || ...);
        }

        [[nodiscard]] static constexpr std::optional<std::string_view> to_string_opt(
            E value
        ) noexcept {
            std::optional<std::string_view> result{};

            (void)((Entries::value() == value
                ? (result = Entries::name(), true)
                : false) || ...);

            return result;
        }

        [[nodiscard]] static constexpr std::string_view to_string(
            E value,
            std::string_view fallback = "unknown"
        ) noexcept {
            const auto result = to_string_opt(value);

            if (result.has_value()) {
                return *result;
            }

            return fallback;
        }

        [[nodiscard]] static constexpr std::optional<E> from_string(
            std::string_view name
        ) noexcept {
            std::optional<E> result{};

            (void)((Entries::name() == name
                ? (result = Entries::value(), true)
                : false) || ...);

            return result;
        }

        [[nodiscard]] static constexpr E from_string_or(
            std::string_view name,
            E fallback
        ) noexcept {
            const auto result = from_string(name);

            if (result.has_value()) {
                return *result;
            }

            return fallback;
        }

        template <typename Fn>
        static constexpr void for_each_name(Fn&& fn) {
            (fn(Entries::name()), ...);
        }

        template <typename Fn>
        static constexpr void for_each_entry(Fn&& fn) {
            (fn(Entries::value(), Entries::name()), ...);
        }

        [[nodiscard]] static consteval bool has_no_empty_names() noexcept {
            return ((!Entries::name_empty()) && ...);
        }

        [[nodiscard]] static consteval bool has_no_duplicate_names() noexcept {
            constexpr auto table_names = names();

            for (std::size_t i = 0; i < table_names.size(); ++i) {
                for (std::size_t j = i + 1; j < table_names.size(); ++j) {
                    if (table_names[i] == table_names[j]) {
                        return false;
                    }
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool has_no_duplicate_values() noexcept {
            constexpr auto table_values = values();

            for (std::size_t i = 0; i < table_values.size(); ++i) {
                for (std::size_t j = i + 1; j < table_values.size(); ++j) {
                    if (table_values[i] == table_values[j]) {
                        return false;
                    }
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool covers_zero_based_count(E count) noexcept {
            if (enum_underlying_is_negative(count)) {
                return false;
            }

            const auto count_value = static_cast<std::size_t>(enum_underlying(count));

            if (sizeof...(Entries) != count_value) {
                return false;
            }

            constexpr auto table_values = values();

            for (std::size_t expected = 0; expected < count_value; ++expected) {
                bool found = false;

                for (const auto value : table_values) {
                    if (enum_underlying_is_negative(value)) {
                        return false;
                    }

                    if (static_cast<std::size_t>(enum_underlying(value)) == expected) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] static consteval bool is_canonical() noexcept {
            return has_no_empty_names()
                && has_no_duplicate_names()
                && has_no_duplicate_values();
        }

        [[nodiscard]] static consteval bool is_zero_based_count_canonical(
            E count
        ) noexcept {
            return is_canonical()
                && covers_zero_based_count(count);
        }

        [[nodiscard]] static consteval bool is_display_aliases() noexcept {
            return has_no_empty_names()
                && has_no_duplicate_names();
        }

        [[nodiscard]] static consteval bool is_parse_aliases() noexcept {
            return has_no_duplicate_names();
        }

        template <static_string Separator = ", ">
        [[nodiscard]] static consteval auto choices_static_string() noexcept {
            constexpr std::size_t entry_count = sizeof...(Entries);
            constexpr std::size_t separator_size = Separator.size();

            constexpr std::size_t names_size =
                (Entries::name_size() + ... + std::size_t{ 0 });

            constexpr std::size_t separators_size =
                entry_count == 0
                    ? 0
                    : (entry_count - 1) * separator_size;

            constexpr std::size_t total_size = names_size + separators_size;

            static_string<total_size + 1> result{};

            std::size_t out = 0;
            std::size_t index = 0;

            auto append = [&](std::string_view text) constexpr {
                for (const char ch : text) {
                    result.m_data[out] = ch;
                    ++out;
                }
            };

            auto append_entry = [&](std::string_view name) constexpr {
                if (index != 0) {
                    append(Separator.view());
                }

                append(name);
                ++index;
            };

            (append_entry(Entries::name()), ...);

            result.m_data[total_size] = '\0';

            return result;
        }

        template <static_string Separator = ", ">
        [[nodiscard]] static std::string choices_string() {
            constexpr auto result = choices_static_string<Separator>();

            return std::string{ result.view() };
        }
    };
}

#endif // MGMAKE_DETAIL_ENUM_STRING_HXX
