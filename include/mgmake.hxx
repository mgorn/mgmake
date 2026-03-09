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

			constexpr StaticString() = default;
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

		template<size_t N1, size_t N2>
		constexpr auto operator+(const StaticString<N1>& a, const StaticString<N2>& b) {
			StaticString<N1 + N2 - 1> result;

			for (size_t i = 0; i < N1 - 1; ++i)
				result.mData[i] = a.mData[i];

			for (size_t i = 0; i < N2; ++i)
				result.mData[i + N1 - 1] = b.mData[i];

			return result;
		}
	}

	struct Toolchain {
		std::string mCompiler = "clang++";
	};

	template<
		detail::StaticString... sources_v
	>
	struct ListImpl {
		template<detail::StaticString new_source_v>
		[[nodiscard]] constexpr auto add() const {
			return detail::poof<ListImpl<sources_v..., new_source_v>>();
		}

		[[nodiscard]] constexpr auto collect() const {
			if constexpr (sizeof...(sources_v) == 0) {
				return detail::StaticString{""};
			} else if constexpr (sizeof...(sources_v) == 1) {
				return (sources_v + ...);
			} else {
				constexpr detail::StaticString space{" "};
				return ((sources_v + space) + ...);
			}
		}
	};
	static constexpr ListImpl Sources{};
	static constexpr ListImpl Includes{};

	// Assertions to make sure sources are collected properly
	static_assert(Sources.add<"build.cxx">().collect().str() == "build.cxx");
	static_assert(Sources.add<"build.cxx">().add<"another.cxx">().collect().str() == "build.cxx another.cxx ");

	template<
		detail::StaticString name_v = "",
		auto sources_v = Sources,
		auto includes_v = Includes
	>
	struct TargetImpl {
		template<detail::StaticString new_name_v>
		using name = TargetImpl<new_name_v, sources_v, includes_v>;
		template<auto new_sources_v = Sources>
		using sources = TargetImpl<name_v, new_sources_v, includes_v>;
		template<auto new_includes_v = Includes>
		using includes = TargetImpl<name_v, sources_v, new_includes_v>;

		[[nodiscard]] static auto command(const auto& toolchain, const auto& project) {
			return std::format("{} -std={} -o {} {} -I {}", toolchain.mCompiler, project.standard_value.str(), name_v.str(), sources_v.collect().str(), includes_v.collect().str());
		}
		static auto build(const auto& toolchain, const auto& project) {
			auto cmd = command(toolchain, project);
			std::print("Invoking build command: {}\n", cmd);
			return system(cmd.c_str());
		}
	};
	using Target = TargetImpl<>;

	template<
		detail::StaticString name_v = "",
		detail::StaticString standard_v = "c++2c",
		typename... target_ts
	>
	struct ProjectImpl {
		template<detail::StaticString new_name_v>
		using name = ProjectImpl<new_name_v, standard_v, target_ts...>;

		template<detail::StaticString new_standard_v>
		using standard = ProjectImpl<name_v, new_standard_v, target_ts...>;

		template<typename new_target_t>
		using add_target = ProjectImpl<name_v, standard_v, target_ts..., new_target_t>;

		static constexpr auto name_value = name_v;
		static constexpr auto standard_value = standard_v;

		[[nodiscard]] auto build(const auto& toolchain) const {
			std::print("Building project '{}'\n", name_value.str());
			return (target_ts::build(toolchain, *this) | ...);
		}
	};
	using Project = ProjectImpl<>;
}
namespace mgmk = mgmake;

#define BUILD_ENTRY(ProjectType) \
int main() { \
	Toolchain toolchain; \
	ProjectType build; \
	return build.build(toolchain); \
}