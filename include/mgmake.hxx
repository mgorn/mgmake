#pragma once

#include <array>
#include <cstddef>
#include <print>
#include <string>

namespace mgmake {
	namespace detail {
		template<typename T>
		consteval decltype(auto) poof() {
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

			[[nodiscard]] constexpr std::string str() const {
				return mData.data();
			}

			// Required for template parameter equivalence checks in C++20
			constexpr operator const char*() const { return mData.data(); }

			constexpr operator std::string() const { return mData.data(); }
		};
	}

	template<detail::StaticString compiler_v = "clang">
	struct ToolchainImpl {
		template<detail::StaticString new_compiler_v = "clang">
		consteval decltype(auto) compiler() const {
			return detail::poof<ToolchainImpl<new_compiler_v>>();
		}

		[[nodiscard]] constexpr decltype(auto) clang() const {
			return detail::poof<ToolchainImpl<"clang">>();
		}
		[[nodiscard]] constexpr decltype(auto) gcc() const {
			return detail::poof<ToolchainImpl<"gcc">>();
		}

		[[nodiscard]] constexpr std::string_view get_compiler() const {
			return compiler_v.str();
		}

		template<auto sources_v>
		void compile_sources() {

		}
	};

	template<detail::StaticString name_v = "", auto target = nullptr>
	struct Project {
		void build() {
			std::print("Building project {}\n", name_v.str());
			if constexpr (not std::is_same_v<decltype(target), std::nullptr_t>) {
				std::print("Building target {}\n", target.get_name());
			}
		}
	};

	template<detail::StaticString... sources_v>
	struct SourcesImpl {
		template<detail::StaticString... new_sources_v>
		consteval decltype(auto) sources() const {
			return detail::poof<SourcesImpl<new_sources_v...>>();
		}
	};
	static constexpr SourcesImpl Sources{};

	template<auto name_v = nullptr, auto sources_v = Sources>
	struct TargetImpl {
		[[nodiscard]] constexpr decltype(auto) get_name() const {
			if constexpr (name_v != nullptr) {
				return name_v.str();
			}
			return std::string{"Anonymous Target"};
		}

		template<detail::StaticString new_name_v = "">
		consteval decltype(auto) name() const {
			return detail::poof<TargetImpl<new_name_v, sources_v>>();
		}

		template<auto new_sources_v = Sources>
		consteval decltype(auto) with() const {
			return detail::poof<TargetImpl<name_v, new_sources_v>>();
		}
	};
	static constexpr TargetImpl Target{};
}
namespace mgmk = mgmake;

#define BUILD_ENTRY(ProjectType) \
int main() { \
	ProjectType build; \
	build.build(); \
	return 0; \
}