#pragma once

#ifndef MGMAKE_SPEC_TOOLCHAIN_HXX
#define MGMAKE_SPEC_TOOLCHAIN_HXX

#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct toolchain_impl : public meta::type_builder<toolchain_impl, storage_t>, meta::named {
		using builder_type = meta::type_builder<toolchain_impl, storage_t>;

		template<meta::static_string value_v>
		consteval auto cc() const {
			return builder_type::template set_str<"cc", value_v>();
		}
		consteval auto cc() const {
			return builder_type::template get_str<"cc">();
		}

		template<meta::static_string value_v>
		consteval auto cxx() const {
			return builder_type::template set_str<"cxx", value_v>();
		}
		consteval auto cxx() const {
			return builder_type::template get_str<"cxx">();
		}

		template<meta::static_string value_v>
		consteval auto ar() const {
			return builder_type::template set_str<"ar", value_v>();
		}
		consteval auto ar() const {
			return builder_type::template get_str<"ar">();
		}

		template<meta::static_string value_v>
		consteval auto linker() const {
			return builder_type::template set_str<"linker", value_v>();
		}
		consteval auto linker() const {
			return builder_type::template get_str<"linker">();
		}
	};
	static constexpr auto toolchain = toolchain_impl<>{};
}

#endif // MGMAKE_SPEC_TOOLCHAIN_HXX