#pragma once

#ifndef MGMAKE_DISCOVERY_SOURCE_ROLE_HXX
#define MGMAKE_DISCOVERY_SOURCE_ROLE_HXX

#include "tool_role.hxx"

#include <filesystem>

namespace mgmake::discovery {
	[[nodiscard]] inline tool_role source_tool_role(
		const std::filesystem::path& source
	) {
		const auto ext = source.extension().string();

		if (ext == ".c") {
			return tool_role::c_compiler;
		}

		if (ext == ".s" || ext == ".S" || ext == ".asm") {
			return tool_role::assembler;
		}

		if (ext == ".rc") {
			return tool_role::resource_compiler;
		}

		if (ext == ".idl") {
			return tool_role::midl_compiler;
		}

		return tool_role::cxx_compiler;
	}
}

#endif
