#pragma once

#ifndef MGMAKE_DETAIL_STATIC_STRING_HXX
#define MGMAKE_DETAIL_STATIC_STRING_HXX

namespace mgmake::detail {
	// Compile-time string type
	template<std::size_t N>
	struct static_string {
		std::array<char, N> m_data{};

		constexpr static_string() = default;
		// Constexpr constructor to allow implicit conversion from string literals
		constexpr static_string(const char (&str)[N]) {
			for (std::size_t i = 0; i < N; ++i) {
				m_data[i] = str[i];
			}
		}

		[[nodiscard]] constexpr std::string_view view() const {
			return m_data.data();
		}

		// Required for template parameter equivalence checks in C++20
		constexpr operator const char*() const { return m_data.data(); }

		constexpr operator std::string_view() const { return m_data.view(); }
	};

	// Concat 2 static strings
	template<size_t N1, size_t N2>
	constexpr auto operator+(const static_string<N1>& a, const static_string<N2>& b) {
		static_string<N1 + N2 - 1> result;

		for (size_t i = 0; i < N1 - 1; ++i)
			result.m_data[i] = a.m_data[i];

		for (size_t i = 0; i < N2; ++i)
			result.m_data[i + N1 - 1] = b.m_data[i];

		return result;
	}
}

#endif