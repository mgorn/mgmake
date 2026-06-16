#pragma once

#ifndef MGMAKE_DISCOVERY_WINDOWS_REGISTRY_HXX
#define MGMAKE_DISCOVERY_WINDOWS_REGISTRY_HXX

#include "../providers.hxx"

namespace mgmake::discovery {
	inline void add_windows_registry_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_LLVM_ROOT")) dirs.emplace_back(*root / "bin");
		dirs.emplace_back("C:/Program Files/LLVM/bin");
		dirs.emplace_back("C:/Program Files (x86)/LLVM/bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::windows_registry, 130, "Windows registry and registered install roots");
#else
		(void)ctx; (void)req; (void)out;
#endif
	}
}

#endif
