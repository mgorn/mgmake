#pragma once

#ifndef MGMAKE_META_STATIC_STRING_HXX
#define MGMAKE_META_STATIC_STRING_HXX

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <string_view>

// static_string carries compile-time strings through templates without relying on runtime storage.

namespace mgmake::meta {
    template<std::size_t N>
    struct static_string {
        std::array<char, N> m_data{};

        constexpr static_string() = default;

        // Allows implicit conversion from string literals.
        constexpr static_string(const char (&str)[N]) {
            for (std::size_t i = 0; i < N; ++i) {
                m_data[i] = str[i];
            }
        }

        [[nodiscard]] static consteval std::size_t size() noexcept {
            return N - 1;
        }

        [[nodiscard]] static consteval bool empty() noexcept {
            return size() == 0;
        }

        [[nodiscard]] constexpr std::string_view view() const noexcept {
            return { m_data.data(), size() };
        }

        constexpr operator std::string_view() const noexcept {
            return view();
        }

		template<std::unsigned_integral hash_t = std::size_t>
        [[nodiscard]] constexpr hash_t hash() const noexcept {
            constexpr hash_t offset_basis = sizeof(hash_t) <= 4
				? static_cast<hash_t>(2166136261u)
				: static_cast<hash_t>(14695981039346656037ull);

            constexpr hash_t prime = sizeof(hash_t) <= 4
				? static_cast<hash_t>(16777619u)
				: static_cast<hash_t>(1099511628211ull);

            auto result = offset_basis;
            for (const auto c : view()) {
                result ^= static_cast<hash_t>(static_cast<uint8_t>(c));
                result *= prime;
            }

            return result;
        }
    };

    template<std::size_t N1, std::size_t N2>
    constexpr bool operator==(const static_string<N1>& a, const static_string<N2>& b) noexcept {
        if constexpr (N1 != N2) {
            return false;
        } else {
            for (std::size_t i = 0; i < N1; ++i) {
                if (a.m_data[i] != b.m_data[i]) {
                    return false;
                }
            }

            return true;
        }
    }

    template<std::size_t N1, std::size_t N2>
    constexpr auto operator+(const static_string<N1>& a, const static_string<N2>& b) {
        static_string<N1 + N2 - 1> result;

        for (std::size_t i = 0; i < N1 - 1; ++i) {
            result.m_data[i] = a.m_data[i];
        }
        for (std::size_t i = 0; i < N2; ++i) {
            result.m_data[i + N1 - 1] = b.m_data[i];
        }

        return result;
    }
}

namespace std {
    template<std::size_t N>
    struct hash<::mgmake::meta::static_string<N>> {
        [[nodiscard]] constexpr std::size_t operator()(
            const ::mgmake::meta::static_string<N>& value
        ) const noexcept {
            return value.hash();
        }
    };

	template<std::size_t N>
    struct formatter<::mgmake::meta::static_string<N>, char> : formatter<std::string_view, char> {
        using base_type = formatter<std::string_view, char>;

        template<typename format_context_t>
        constexpr auto format(const ::mgmake::meta::static_string<N>& value, format_context_t& context) const {
            return base_type::format(value.view(), context);
        }
    };
}

#endif // MGMAKE_META_STATIC_STRING_HXX