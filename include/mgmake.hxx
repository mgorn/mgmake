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
		void build();
	};

	template<detail::StaticString... source_v>
	struct SourcesImpl {

	};

	template<detail::StaticString name_v = "", typename sources_t = SourcesImpl<>>
	struct TargetImpl {
		template<detail::StaticString new_name_v = "">
		consteval decltype(auto) name() const {
			return detail::poof<TargetImpl<new_name_v>>();
		}
		template<detail::StaticString... new_sources_v>
		consteval decltype(auto) sources() const {
			return detail::poof<TargetImpl<name_v, SourcesImpl<new_sources_v>...>>();
		}
	};
	static constexpr TargetImpl Target{};

	template<detail::StaticString name_v, auto cb>
	void Project<name_v, cb>::build() {

	}
}
namespace mgmk = mgmake;

#define BUILD_ENTRY(ProjectType) \
int main() { \
	ProjectType project; \
	project.build(); \
	return 0; \
}