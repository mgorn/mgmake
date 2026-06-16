#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_FAMILY_HXX
#define MGMAKE_DISCOVERY_TOOL_FAMILY_HXX

namespace mgmake::discovery {
	enum struct tool_family {
		unknown,
		gcc,
		clang,
		apple_clang,
		msvc,
		clang_cl,
		llvm_binutils,
		gnu_binutils,
		msvc_binutils,
		mingw,
		cygwin,
		android_clang,
		emscripten,
		cuda,
		hip,
		count
	};

	enum struct linker_flavor {
		unknown,
		compiler_driver,
		gnu_ld,
		lld,
		lld_link,
		msvc_link,
		apple_ld,
		count
	};

	enum struct object_format {
		unknown,
		coff,
		elf,
		mach_o,
		wasm,
		count
	};
}

#endif
