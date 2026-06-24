#pragma once

#ifndef MGMAKE_DISCOVERY_MACOS_XCRUN_HXX
#define MGMAKE_DISCOVERY_MACOS_XCRUN_HXX

#include "../providers.hxx"
#include "../validate.hxx"

namespace mgmake::discovery {
	inline void add_xcrun_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(__APPLE__)
		for (const auto& logical : candidate_names_for(ctx.request(), req.m_role, ctx.m_mode)) {
			sys::command_line command;
			command.m_args.emplace_back("xcrun");
			if (!ctx.options().m_apple_sdk.empty()) {
				command.m_args.emplace_back("--sdk");
				command.m_args.emplace_back(ctx.options().m_apple_sdk);
			}
			command.m_args.emplace_back("--find");
			command.m_args.emplace_back(logical);

			auto found = capture_command(command);
			if (found && !found->empty()) {
				std::string path = *found;
				while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) path.pop_back();
				if (is_launchable_file(path)) {
					out.push_back({req.m_role, logical, path, tool_provider::xcrun, "xcrun --find", 210});
				}
			}
		}
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(__APPLE__)
	}

	inline void add_homebrew_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(__APPLE__)
		std::vector<std::filesystem::path> dirs = {
			"/opt/homebrew/opt/llvm/bin",
			"/usr/local/opt/llvm/bin",
			"/opt/homebrew/bin",
			"/usr/local/bin"
		};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::homebrew, 220, "Homebrew tool root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(__APPLE__)
	}

	inline void add_macports_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(__APPLE__)
		add_candidates_from_dirs(out, ctx.request(), req, {"/opt/local/bin"}, tool_provider::macports, 230, "MacPorts tool root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(__APPLE__)
	}
}

#endif // MGMAKE_DISCOVERY_MACOS_XCRUN_HXX
