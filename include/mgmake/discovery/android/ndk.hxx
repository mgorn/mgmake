#pragma once

#ifndef MGMAKE_DISCOVERY_ANDROID_NDK_HXX
#define MGMAKE_DISCOVERY_ANDROID_NDK_HXX

#include "../providers.hxx"

#include <functional>
#include <ranges>

// Android NDK helpers translate mgmake target options into NDK host tags, ABI names, and sysroot flags.

namespace mgmake::discovery {
	[[nodiscard]] inline std::string android_host_tag() {
#if defined(_WIN32)
		return "windows-x86_64";
#elif defined(__APPLE__)
		#if defined(__aarch64__)
			return "darwin-arm64";
		#else
			return "darwin-x86_64";
		#endif // defined(__aarch64__)
#else
		return "linux-x86_64";
#endif // defined(_WIN32)
	}

	namespace android {
		[[nodiscard]] inline std::string triple_for_abi(std::string_view abi) {
			if (abi == "arm64-v8a") return "aarch64-linux-android";
			if (abi == "armeabi-v7a") return "armv7a-linux-androideabi";
			if (abi == "x86") return "i686-linux-android";
			if (abi == "x86_64") return "x86_64-linux-android";
			return {};
		}

		inline void add_android_sdk_ndk_roots(
			std::vector<std::filesystem::path>& roots
		) {
			for (std::string_view variable : {"ANDROID_HOME", "ANDROID_SDK_ROOT"}) {
				auto sdk = discovery::getenv_path(variable);

				if (!sdk) {
					continue;
				}

				auto ndk_dir = *sdk / "ndk";
				std::error_code ec;

				if (!std::filesystem::exists(ndk_dir, ec)) {
					continue;
				}

				for (const auto& entry : std::filesystem::directory_iterator(ndk_dir, ec)) {
					if (entry.is_directory()) {
						roots.emplace_back(entry.path());
					}
				}
			}

			std::ranges::sort(roots, std::greater<>{});
		}

		[[nodiscard]] inline std::vector<std::string> target_sysroot_args(
			const std::filesystem::path& ndk,
			std::string_view host_tag,
			std::string_view abi,
			int api
		) {
			const auto triple = triple_for_abi(abi);

			if (triple.empty() || api <= 0) {
				return {};
			}

			const auto sysroot = ndk / "toolchains" / "llvm" / "prebuilt"
				/ std::string{host_tag} / "sysroot";

			return {
				"--target=" + triple + std::to_string(api),
				"--sysroot=" + sysroot.string()
			};
		}
	}

	inline void add_android_ndk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> roots;
		if (!ctx.options().m_android_ndk.empty()) roots.emplace_back(ctx.options().m_android_ndk);
		if (auto root = getenv_path("ANDROID_NDK_ROOT")) roots.emplace_back(*root);
		if (auto root = getenv_path("ANDROID_NDK_HOME")) roots.emplace_back(*root);
		android::add_android_sdk_ndk_roots(roots);

		for (const auto& root : roots) {
			auto bin = root / "toolchains" / "llvm" / "prebuilt" / android_host_tag() / "bin";
			add_candidates_from_dirs(
				out,
				ctx.request(),
				req,
				{bin},
				tool_provider::android_ndk,
				270,
				"Android NDK LLVM toolchain",
				true,
				ctx.m_mode,
				root
			);
		}
	}

	inline void add_emscripten_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("EMSDK")) {
			dirs.emplace_back(*root / "upstream" / "emscripten");
			dirs.emplace_back(*root / "upstream" / "bin");
		}
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::emscripten_sdk, 280, "Emscripten SDK root", true, ctx.m_mode);
	}

	inline void add_embedded_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;
		if (auto root = getenv_path("MGMK_EMBEDDED_SDK")) dirs.emplace_back(*root / "bin");
		if (auto root = getenv_path("ARM_GCC_ROOT")) dirs.emplace_back(*root / "bin");
		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::embedded_sdk, 290, "embedded SDK root", true, ctx.m_mode);
	}
}

#endif // MGMAKE_DISCOVERY_ANDROID_NDK_HXX
