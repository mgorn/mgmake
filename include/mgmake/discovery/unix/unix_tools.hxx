#pragma once

#ifndef MGMAKE_DISCOVERY_UNIX_UNIX_TOOLS_HXX
#define MGMAKE_DISCOVERY_UNIX_UNIX_TOOLS_HXX

#include "../providers.hxx"

namespace mgmake::discovery {
	inline void add_unix_system_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(MGMK_PLATFORM_POSIX)
		std::vector<std::filesystem::path> dirs = {"/usr/local/bin", "/usr/bin", "/bin", "/opt/bin", "/opt/local/bin"};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::unix_system, 240, "Unix system tool root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_distro_llvm_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(MGMK_PLATFORM_POSIX)
		std::vector<std::filesystem::path> dirs;
		for (int version = 30; version >= 10; --version) {
			dirs.emplace_back("/usr/lib/llvm-" + std::to_string(version) + "/bin");
			dirs.emplace_back("/usr/local/llvm-" + std::to_string(version) + "/bin");
		}
		dirs.emplace_back("/opt/llvm/bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::distro_llvm, 250, "distro LLVM root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_distro_gcc_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(MGMK_PLATFORM_POSIX)
		std::vector<std::filesystem::path> dirs = {"/usr/bin", "/usr/local/bin"};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::distro_gcc, 260, "distro GCC root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif
	}
}

#endif
