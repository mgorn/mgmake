#pragma once

#ifndef MGMAKE_SPEC_TOOLCHAIN_HXX
#define MGMAKE_SPEC_TOOLCHAIN_HXX

#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct toolchain_impl : public meta::type_builder<toolchain_impl, storage_t>, public meta::named<toolchain_impl<storage_t>> {
		using builder_type = meta::type_builder<toolchain_impl, storage_t>;

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto cc() {
			return builder_type::template set_str<"cc", value_v>();
		}
		static consteval auto cc() {
			return builder_type::template get_str<"cc">();
		}

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto cxx() {
			return builder_type::template set_str<"cxx", value_v>();
		}
		static consteval auto cxx() {
			return builder_type::template get_str<"cxx">();
		}

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto ar() {
			return builder_type::template set_str<"ar", value_v>();
		}
		static consteval auto ar() {
			return builder_type::template get_str<"ar">();
		}

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto linker() {
			return builder_type::template set_str<"linker", value_v>();
		}
		static consteval auto linker() {
			return builder_type::template get_str<"linker">();
		}
	};
	static constexpr auto toolchain = toolchain_impl<>{};
}

#endif // MGMAKE_SPEC_TOOLCHAIN_HXX
