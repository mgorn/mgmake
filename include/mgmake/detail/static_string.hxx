#pragma once

#ifndef MGMAKE_DETAIL_STATIC_STRING_HXX
#define MGMAKE_DETAIL_STATIC_STRING_HXX

#include <array>
#include <cstddef>
#include <string_view>

namespace mgmake::detail {
	template <std::size_t N>
	struct static_string {
		std::array<char, N> m_data{};

		constexpr static_string() = default;

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
			return std::string_view{ m_data.data(), size() };
		}

		constexpr operator const char*() const {
			return m_data.data();
		}

		constexpr operator std::string_view() const {
			return view();
		}
	};

	template <std::size_t N1, std::size_t N2>
	constexpr auto operator+(
		const static_string<N1>& a,
		const static_string<N2>& b
	) {
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

#endif
