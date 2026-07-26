#pragma once

#ifndef MGMAKE_FIND_TOOLCHAIN_HXX
#define MGMAKE_FIND_TOOLCHAIN_HXX

#include "../find/default_tools.hxx"
#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"

namespace mgmake::find {
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

#define MGMK_FIND_TOOLCHAIN_TOOL_FNS(TOOL) \
		template<meta::static_string value_v> \
		[[nodiscard]] static consteval auto TOOL() { \
			return tool<find::TOOL##_tool, value_v>(); \
		} \
		[[nodiscard]] static consteval auto TOOL() { \
			return tool<find::TOOL##_tool>(); \
		}

		MGMK_FIND_TOOLCHAIN_TOOL_FNS(cc);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(cxx);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(objc);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(objcxx);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(assembler);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(cuda_compiler);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(hip_compiler);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(resource_compiler);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(midl_compiler);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(archiver);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(ranlib);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(librarian);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(linker);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(shared_linker);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(dll);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(manifest);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(strip);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(objcopy);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(objdump);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(nm);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(readelf);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(debug_symbol);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(lipo);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(install_name);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(codesign);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(ninja);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(make);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(msbuild);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(xcodebuild);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(cmake);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(pkg_config);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(git);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(curl);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(wget);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(unzip);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(tar);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(exe_wrapper);
		MGMK_FIND_TOOLCHAIN_TOOL_FNS(emulator);
		
#undef MGMK_FIND_TOOLCHAIN_TOOL_FNS
	};
	static constexpr auto toolchain = toolchain_impl<>{};
}

#endif // MGMAKE_FIND_TOOLCHAIN_HXX
