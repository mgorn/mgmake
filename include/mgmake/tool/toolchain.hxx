#pragma once

#ifndef MGMAKE_TOOL_TOOLCHAIN_HXX
#define MGMAKE_TOOL_TOOLCHAIN_HXX

#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"
#include "../tool/default_tools.hxx"

namespace mgmake::tool {
	template<typename storage_t = meta::type_map<>>
	struct toolchain_impl : public meta::type_builder<toolchain_impl, storage_t>, public meta::named<toolchain_impl<storage_t>> {
		using builder_type = meta::type_builder<toolchain_impl, storage_t>;

		// Override the default name for the tool for this toolchain
		// e.g. llvm toolchain would have "clang++" as logical for "cxx_tool"
		template<auto tool_v, meta::static_string logical_v>
		[[nodiscard]] static consteval auto tool() {
			// Use the tool's cli name (e.g. "cxx_tool" -> "cxx")
			return builder_type::template set_str<tool_v.cli(), logical_v>();
		}
		template<auto tool_v>
		[[nodiscard]] static consteval auto tool() {
			return builder_type::template get_str<tool_v.cli()>();
		}

		template<auto tool_v>
		[[nodiscard]] static consteval auto has_tool() {
			return builder_type::template has<tool_v.cli()>();
		}

#define MGMK_TOOL_TOOLCHAIN_TOOL_FNS(TOOL) \
		template<meta::static_string value_v> \
		[[nodiscard]] static consteval auto TOOL() { \
			return tool<tool::TOOL##_tool, value_v>(); \
		} \
		[[nodiscard]] static consteval auto TOOL() { \
			return tool<tool::TOOL##_tool>(); \
		}

		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(cc);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(cxx);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(objc);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(objcxx);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(assembler);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(cuda_compiler);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(hip_compiler);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(resource_compiler);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(midl_compiler);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(archiver);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(ranlib);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(librarian);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(linker);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(shared_linker);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(dll);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(manifest);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(strip);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(objcopy);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(objdump);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(nm);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(readelf);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(debug_symbol);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(lipo);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(install_name);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(codesign);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(ninja);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(make);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(msbuild);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(xcodebuild);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(cmake);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(pkg_config);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(git);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(curl);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(wget);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(unzip);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(tar);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(exe_wrapper);
		MGMK_TOOL_TOOLCHAIN_TOOL_FNS(emulator);
		
#undef MGMK_TOOL_TOOLCHAIN_TOOL_FNS
	};
	static constexpr auto toolchain = toolchain_impl<>{};
}

#endif // MGMAKE_TOOL_TOOLCHAIN_HXX
