#pragma once

#ifndef MGMAKE_SPEC_TOOLCHAIN_HXX
#define MGMAKE_SPEC_TOOLCHAIN_HXX

#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct toolchain_impl : public meta::type_builder<toolchain_impl, storage_t>, meta::named {
		template<meta::static_string value_v>
		consteval auto cc() const {
			return set_str<"cc", value_v>();
		}
		consteval meta::static_string cc() const {
			return get_str<"cc">();
		}

		template<meta::static_string value_v>
		consteval auto cxx() const {
			return set_str<"cxx", value_v>();
		}
		consteval meta::static_string cxx() const {
			return get_str<"cxx">();
		}

		template<meta::static_string value_v>
		consteval auto ar() const {
			return set_str<"ar", value_v>();
		}
		consteval meta::static_string ar() const {
			return get_str<"ar">();
		}

		template<meta::static_string value_v>
		consteval auto linker() const {
			return set_str<"linker", value_v>();
		}
		consteval meta::static_string linker() const {
			return get_str<"linker">();
		}
	};
	using toolchain = toolchain_builder<>;
}

#endif // MGMAKE_SPEC_TOOLCHAIN_HXX