#pragma once

#ifndef MGMAKE_SPEC_TOOLCHAIN_HXX
#define MGMAKE_SPEC_TOOLCHAIN_HXX

#include "../find/default_tools.hxx"
#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct toolchain_impl : public meta::type_builder<toolchain_impl, storage_t>, public meta::named<toolchain_impl<storage_t>> {
		using builder_type = meta::type_builder<toolchain_impl, storage_t>;

		// Override the default name for the tool for this toolchain
		// e.g. llvm toolchain would have "clang++" as logical for "cxx_tool"
		template<auto tool_v, meta::static_string logical_v>
		[[nodiscard]] static consteval auto tool() {
			// Use the tool's logical name (e.g. "cxx_tool" -> "cxx")
			return builder_type::template set_str<tool_v.logical(), logical_v>();
		}
		template<auto tool_v>
		[[nodiscard]] static consteval auto tool() {
			return builder_type::template get_str<tool_v.logical()>();
		}

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto cc() {
			return tool<find::cc_tool, value_v>();
		}
		static consteval auto cc() {
			return tool<find::cc_tool>();
		}

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto cxx() {
			return tool<find::cxx_tool, value_v>();
		}
		static consteval auto cxx() {
			return tool<find::cxx_tool>();
		}

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto ar() {
			return tool<find::archiver_tool, value_v>();
		}
		static consteval auto ar() {
			return tool<find::archiver_tool>();
		}

		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto linker() {
			return tool<find::linker_tool, value_v>();
		}
		static consteval auto linker() {
			return tool<find::linker_tool>();
		}
	};
	static constexpr auto toolchain = toolchain_impl<>{};
}

#endif // MGMAKE_SPEC_TOOLCHAIN_HXX
