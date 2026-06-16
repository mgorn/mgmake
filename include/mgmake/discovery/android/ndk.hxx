#pragma once

#ifndef MGMAKE_DISCOVERY_ANDROID_NDK_HXX
#define MGMAKE_DISCOVERY_ANDROID_NDK_HXX

#include "../providers.hxx"

namespace mgmake::discovery {
	[[nodiscard]] inline std::string android_host_tag() {
#if defined(_WIN32)
		return "windows-x86_64";
#elif defined(__APPLE__)
		#if defined(__aarch64__)
			return "darwin-arm64";
		#else
			return "darwin-x86_64";
		#endif
#else
		return "linux-x86_64";
#endif
	}

	inline void add_android_ndk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> roots;
		if (!ctx.options().m_android_ndk.empty()) roots.emplace_back(ctx.options().m_android_ndk);
		if (auto root = getenv_path("ANDROID_NDK_ROOT")) roots.emplace_back(*root);
		if (auto root = getenv_path("ANDROID_NDK_HOME")) roots.emplace_back(*root);

		for (const auto& root : roots) {
			auto bin = root / "toolchains" / "llvm" / "prebuilt" / android_host_tag() / "bin";
			add_candidates_from_dirs(out, ctx.request(), req, {bin}, tool_provider::android_ndk, 270, "Android NDK LLVM toolchain");
		}
	}

	inline void add_emscripten_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("EMSDK")) {
			dirs.emplace_back(*root / "upstream" / "emscripten");
			dirs.emplace_back(*root / "upstream" / "bin");
		}
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::emscripten_sdk, 280, "Emscripten SDK root");
	}

	inline void add_embedded_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_EMBEDDED_SDK")) dirs.emplace_back(*root / "bin");
		if (auto root = getenv_path("ARM_GCC_ROOT")) dirs.emplace_back(*root / "bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::embedded_sdk, 290, "embedded SDK root");
	}
}

#endif
