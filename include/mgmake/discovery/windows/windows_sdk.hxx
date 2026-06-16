#pragma once

#ifndef MGMAKE_DISCOVERY_WINDOWS_WINDOWS_SDK_HXX
#define MGMAKE_DISCOVERY_WINDOWS_WINDOWS_SDK_HXX

#include "../providers.hxx"

namespace mgmake::discovery {
	inline void add_windows_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;

		if (auto pf86 = getenv_path("ProgramFiles(x86)")) {
			auto kits = *pf86 / "Windows Kits" / "10" / "bin";
			std::error_code ec;

			if (std::filesystem::exists(kits, ec)) {
				for (const auto& version : std::filesystem::directory_iterator(kits, ec)) {
					if (!version.is_directory()) continue;
					dirs.emplace_back(version.path() / "x64");
					dirs.emplace_back(version.path() / "x86");
					dirs.emplace_back(version.path() / "arm64");
				}
			}
		}

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::windows_sdk, 160, "Windows SDK tool directory");
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_msys2_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs = {
			"C:/msys64/ucrt64/bin",
			"C:/msys64/mingw64/bin",
			"C:/msys64/clang64/bin",
			"C:/msys64/mingw32/bin",
			"C:/msys64/clangarm64/bin"
		};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::msys2, 180, "MSYS2 toolchain root");
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_mingw_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_MINGW_ROOT")) dirs.emplace_back(*root / "bin");
		dirs.emplace_back("C:/MinGW/bin");
		dirs.emplace_back("C:/mingw64/bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::mingw, 190, "MinGW root");
#else
		(void)ctx; (void)req; (void)out;
#endif
	}

	inline void add_cygwin_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs = {"C:/cygwin64/bin", "C:/cygwin/bin"};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::cygwin, 200, "Cygwin root");
#else
		(void)ctx; (void)req; (void)out;
#endif
	}
}

#endif
