#pragma once

#ifndef MGMAKE_TOOL_DEFAULT_TOOLCHAINS_HXX
#define MGMAKE_TOOL_DEFAULT_TOOLCHAINS_HXX

#include "toolchain.hxx"

namespace mgmake::tool {
	static constexpr auto tc_llvm = toolchain
		.name<"LLVM">()
		.cc<"clang">()
		.cxx<"clang++">()
		.archiver<"llvm-ar">()
		.ranlib<"llvm-ranlib">()
		.linker<"clang++">()
		.shared_linker<"clang++">()
		.resource_compiler<"llvm-rc">()
        .strip<"llvm-strip">()
        .objcopy<"llvm-objcopy">()
        .objdump<"llvm-objdump">()
        .nm<"llvm-nm">()
        .readelf<"llvm-readelf">();
	
	static constexpr auto tc_clang = tc_llvm.name<"Clang">();
	static constexpr auto tc_clang_cl = tc_llvm.name<"Clang-CL">()
		.cc<"clang-cl">()
		.cxx<"clang-cl">()
		.linker<"clang-cl">()
		.shared_linker<"clang-cl">();

	using default_toolchains = meta::value_list<
		tc_llvm,
		tc_clang,
		tc_clang_cl
	>;
}

#endif // MGMAKE_TOOL_DEFAULT_TOOLCHAINS_HXX
