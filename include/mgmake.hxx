#pragma once

#include <array>
#include <cstddef>

namespace mgmake {
	namespace detail {
		template<typename T>
		decltype(auto) poof() {
			static constexpr T value;
			return value;
		}

		template<std::size_t N>
		struct StaticString {
			std::array<char, N> mData{};

			// Constexpr constructor to allow implicit conversion from string literals
			constexpr StaticString(const char (&str)[N]) {
				for (std::size_t i = 0; i < N; ++i) {
					mData[i] = str[i];
				}
			}

			// Required for template parameter equivalence checks in C++20
			constexpr operator const char*() const { return mData.data(); }
		};
	}

	template<detail::StaticString name_v = "", auto cb = nullptr>
	struct Project {

	};


	template<detail::StaticString name_v = "", >
	struct TargetImpl {
		template<detail::StaticString new_name_v = "">
		consteval decltype(auto) name() const {
			return detail::poof<TargetImpl<new_name_v>>();
		}
	};
	static constexpr TargetImpl Target{};
}
namespace mgmk = mgmake;

#define BUILD_ENTRY(ProjectType) \
int main() { \
	ProjectType build; \
	return 0; \
}