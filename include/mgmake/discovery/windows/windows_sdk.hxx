#pragma once

#ifndef MGMAKE_DISCOVERY_WINDOWS_WINDOWS_SDK_HXX
#define MGMAKE_DISCOVERY_WINDOWS_WINDOWS_SDK_HXX

#include "../providers.hxx"

#include <functional>

// Windows SDK discovery locates SDK tools such as rc and mt when MSVC-style builds need them.

namespace mgmake::discovery {
	[[nodiscard]] inline bool is_windows_sdk_role(tool_role role) noexcept {
		switch (role) {
			case tool_role::resource_compiler:
			case tool_role::manifest_tool:
			case tool_role::midl_compiler:
				return true;

			default:
				return false;
		}
	}

	inline void add_windows_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		if (!is_windows_sdk_role(req.m_role)) {
			return;
		}

		std::vector<std::filesystem::path> roots;

		if (auto root = getenv_path("WindowsSdkDir")) {
			roots.emplace_back(*root);
		}

		if (auto pf86 = getenv_path("ProgramFiles(x86)")) {
			roots.emplace_back(*pf86 / "Windows Kits" / "10");
		}

		if (auto pf = getenv_path("ProgramFiles")) {
			roots.emplace_back(*pf / "Windows Kits" / "10");
		}

		std::vector<std::filesystem::path> dirs;

		for (const auto& root : roots) {
			auto kits = root / "bin";
			std::error_code ec;

			if (std::filesystem::exists(kits, ec)) {
				std::vector<std::filesystem::path> versions;

				for (const auto& version : std::filesystem::directory_iterator(kits, ec)) {
					if (!version.is_directory()) continue;
					versions.emplace_back(version.path());
				}

				std::ranges::sort(versions, std::greater<>{});

				for (const auto& version : versions) {
					dirs.emplace_back(version / "x64");
					dirs.emplace_back(version / "x86");
					dirs.emplace_back(version / "arm64");
				}
			}
		}

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::windows_sdk, 160, "Windows SDK tool directory", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(_WIN32)
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
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::msys2, 180, "MSYS2 toolchain root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(_WIN32)
	}

	inline void add_mingw_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_MINGW_ROOT")) dirs.emplace_back(*root / "bin");
		dirs.emplace_back("C:/MinGW/bin");
		dirs.emplace_back("C:/mingw64/bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::mingw, 190, "MinGW root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(_WIN32)
	}

	inline void add_cygwin_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
#if defined(_WIN32)
		std::vector<std::filesystem::path> dirs = {"C:/cygwin64/bin", "C:/cygwin/bin"};
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::cygwin, 200, "Cygwin root", true, ctx.m_mode);
#else
		(void)ctx; (void)req; (void)out;
#endif // defined(_WIN32)
	}
}

#endif // MGMAKE_DISCOVERY_WINDOWS_WINDOWS_SDK_HXX
