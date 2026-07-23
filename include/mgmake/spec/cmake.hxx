#pragma once

#ifndef MGMAKE_SPEC_CMAKE_HXX
#define MGMAKE_SPEC_CMAKE_HXX

#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct cmake_target_impl : public meta::type_builder<cmake_target_impl, storage_t>, public meta::named<cmake_target_impl<storage_t>> {
		using builder_type = meta::type_builder<cmake_target_impl, storage_t>;

		template<auto cmake_v>
		[[nodiscard]] static consteval auto project() {
			return builder_type::template set_value<"project", cmake_v>();
		}
		static consteval auto project() {
			return builder_type::template get_value<"project">();
		}
	};

	template<typename storage_t = meta::type_map<>>
	struct cmake_impl : public meta::type_builder<cmake_impl, storage_t>, public meta::named<cmake_impl<storage_t>> {
		using builder_type = meta::type_builder<cmake_impl, storage_t>;

		template<auto fetch_v>
		[[nodiscard]] static consteval auto fetch() {
			return builder_type::template set_value<"fetch", fetch_v>();
		}
		static consteval auto fetch() {
			return builder_type::template get_value_or<"fetch", nullptr>();
		}

		template<typename cmake_vars_t = meta::type_map<>>
		[[nodiscard]] static consteval auto set_cmake_vars() -> builder_type::template set_type<"cmake_vars", cmake_vars_t> {
			return {};
		}
		template<meta::static_string var_v, meta::static_string val_v>
		[[nodiscard]] static consteval auto define() -> decltype(set_cmake_vars<typename meta::type_or_t<
				typename builder_type::template get_type<"cmake_vars", false>,
				meta::type_map<>
			>
			::template emplace_unique<meta::type_value<var_v>, meta::type_value<val_v>>>()) {
			return {};
		}

		template<bool install_v = true>
		[[nodiscard]] static consteval auto set_install() {
			return builder_type::template set_value<"install", install_v>();
		}
		[[nodiscard]] static consteval auto install() {
			return set_install<true>();
		}
		static consteval bool installs() {
			return builder_type::template get_value_or<"install", false>();
		}

		[[nodiscard]] static consteval auto target() {
			return cmake_target_impl<>{}.project<cmake_impl{}>();
		}
		[[nodiscard]] static consteval auto library() {
			return target();
		}
	};

	static constexpr auto cmake = cmake_impl<>{};
}

#endif // MGMAKE_SPEC_CMAKE_HXX
