#pragma once

#ifndef MGMAKE_BUILD_TOOLCHAIN_REGISTRY_HXX
#define MGMAKE_BUILD_TOOLCHAIN_REGISTRY_HXX

#include "toolchain.hxx"

#include <concepts>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::build {
	struct toolchain_registry {
		std::vector<toolchain> m_toolchains{};

		constexpr toolchain_registry() = default;

		constexpr toolchain_registry(std::initializer_list<toolchain> toolchains)
			: m_toolchains{ toolchains } {}

		constexpr explicit toolchain_registry(std::vector<toolchain> toolchains)
			: m_toolchains{ std::move(toolchains) } {}

		[[nodiscard]] constexpr const toolchain* find(std::string_view name) const noexcept {
			for (const auto& tc : m_toolchains) {
				if (tc.m_name == name) {
					return &tc;
				}
			}

			return nullptr;
		}

		[[nodiscard]] constexpr toolchain_registry emplace(toolchain tc) const {
			auto result = *this;
			result.m_toolchains.emplace_back(std::move(tc));
			return result;
		}

		[[nodiscard]] constexpr std::string choices_string() const {
			std::string result;

			for (std::size_t i = 0; i < m_toolchains.size(); ++i) {
				if (i != 0) {
					result += ", ";
				}

				result += m_toolchains[i].m_name;
			}

			return result;
		}
	};

	template <typename Registry>
	concept toolchain_registry_like =
		requires(const Registry& registry, std::string_view name) {
			{ registry.find(name) } -> std::same_as<const toolchain*>;
			{ registry.choices_string() } -> std::convertible_to<std::string>;
		};

	inline const auto default_toolchains = toolchain_registry{
		tc_clang_mg,
		tc_clang,
		tc_gcc,
		tc_msvc
	};
}

#endif // MGMAKE_BUILD_TOOLCHAIN_REGISTRY_HXX
