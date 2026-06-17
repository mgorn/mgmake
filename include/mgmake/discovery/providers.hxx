#pragma once

#ifndef MGMAKE_DISCOVERY_PROVIDERS_HXX
#define MGMAKE_DISCOVERY_PROVIDERS_HXX

#include "context.hxx"
#include "filesystem.hxx"
#include "tool_names.hxx"
#include "tool_requirement.hxx"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace mgmake::discovery {
	using candidate_list = std::vector<tool_candidate>;

	inline void add_candidate_if_found(
		candidate_list& out,
		const tool_requirement& req,
		std::string_view logical_name,
		const std::filesystem::path& dir,
		tool_provider provider,
		int priority,
		std::string reason,
		bool authoritative = false,
		std::optional<std::filesystem::path> provider_root = std::nullopt
	) {
		if (auto path = find_in_directory(dir, logical_name)) {
			out.push_back({
				.m_role = req.m_role,
				.m_logical_name = std::string{logical_name},
				.m_path = *path,
				.m_provider = provider,
				.m_reason = std::move(reason),
				.m_priority = priority,
				.m_authoritative = authoritative,
				.m_provider_root = std::move(provider_root)
			});
		}
	}

	inline void add_candidates_from_dirs(
		candidate_list& out,
		const build::request& build_req,
		const tool_requirement& req,
		const std::vector<std::filesystem::path>& dirs,
		tool_provider provider,
		int priority,
		std::string reason,
		bool include_target_prefixed = true,
		mode discovery_mode = mode::family_fallback,
		std::optional<std::filesystem::path> provider_root = std::nullopt
	) {
		std::vector<std::string> names;

		if (!req.m_logical_name.empty()) {
			names.emplace_back(req.m_logical_name);
		}

		for (auto name : candidate_names_for(build_req, req.m_role, discovery_mode)) {
			if (std::ranges::find(names, name) == names.end()) {
				names.emplace_back(std::move(name));
			}
		}

		if (include_target_prefixed) {
			for (auto name : target_prefixed_names_for(build_req, req.m_role, discovery_mode)) {
				if (std::ranges::find(names, name) == names.end()) {
					names.emplace_back(std::move(name));
				}
			}
		}

		for (const auto& name : names) {
			for (const auto& dir : dirs) {
				add_candidate_if_found(out, req, name, dir, provider, priority, reason, false, provider_root);
			}
		}
	}

	inline void add_explicit_path_candidates(context&, const tool_requirement& req, candidate_list& out) {
		if (!req.m_logical_name.empty() && is_explicit_path(req.m_logical_name)) {
			out.push_back({
				.m_role = req.m_role,
				.m_logical_name = req.m_logical_name,
				.m_path = std::filesystem::absolute(req.m_logical_name),
				.m_provider = tool_provider::explicit_path,
				.m_reason = "explicit tool path from selected toolchain",
				.m_priority = 0,
				.m_authoritative = true
			});
		}
	}

	inline void add_cli_override_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		const auto override = cli_override_for(ctx.options(), req.m_role);

		if (override.empty()) {
			return;
		}

		if (is_explicit_path(override)) {
			out.push_back({
				.m_role = req.m_role,
				.m_logical_name = override,
				.m_path = std::filesystem::absolute(override),
				.m_provider = tool_provider::cli_override,
				.m_reason = "CLI tool override",
				.m_priority = 10,
				.m_authoritative = true
			});
			return;
		}

		for (const auto& dir : path_entries()) {
			add_candidate_if_found(out, req, override, dir, tool_provider::cli_override, 10, "CLI tool override searched on PATH");
		}
	}

	inline void add_environment_override_candidates(context&, const tool_requirement& req, candidate_list& out) {
		const auto variable = environment_variable_for(req.m_role);

		if (variable.empty()) {
			return;
		}

		const auto value = getenv_string(variable);

		if (!value || value->empty()) {
			return;
		}

		if (is_explicit_path(*value)) {
			out.push_back({
				.m_role = req.m_role,
				.m_logical_name = *value,
				.m_path = std::filesystem::absolute(*value),
				.m_provider = tool_provider::environment_override,
				.m_reason = std::string{variable},
				.m_priority = 20,
				.m_authoritative = true
			});
			return;
		}

		for (const auto& dir : path_entries()) {
			add_candidate_if_found(out, req, *value, dir, tool_provider::environment_override, 20, "environment override " + std::string{variable});
		}
	}

	inline void add_cache_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		if (!ctx.m_use_cache || ctx.m_refresh_cache) {
			return;
		}

		if (auto entry = ctx.m_cache.find(
				ctx.toolchain().name(),
				sys::g_host_target,
				ctx.request().target(),
				req.m_role,
				req.m_logical_name
			)) {
			out.push_back({
				req.m_role,
				req.m_logical_name,
				entry->m_path,
				tool_provider::cache,
				"validated build directory cache entry",
				30
			});
		}
	}

	inline void add_root_candidates(
		context& ctx,
		const tool_requirement& req,
		candidate_list& out
	) {
		std::vector<std::filesystem::path> package_dirs;
		std::vector<std::filesystem::path> project_dirs;
		std::vector<std::filesystem::path> toolchain_dirs;
		std::vector<std::filesystem::path> sdk_dirs;
		std::vector<std::filesystem::path> sysroot_dirs;

		if (!ctx.options().m_package_toolchain_root.empty()) package_dirs.emplace_back(std::filesystem::path{ctx.options().m_package_toolchain_root} / "bin");
		if (ctx.toolchain().m_package_root.has_value()) package_dirs.emplace_back(std::filesystem::path{*ctx.toolchain().m_package_root} / "bin");
		if (!package_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, package_dirs, tool_provider::package_toolchain, 40, "package-provided toolchain root", true, ctx.m_mode);

		for (const auto& root : ctx.toolchain().m_search_roots) project_dirs.emplace_back(root);
		if (!project_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, project_dirs, tool_provider::project_toolchain_root, 50, "project toolchain search root", true, ctx.m_mode);

		if (!ctx.options().m_toolchain_root.empty()) toolchain_dirs.emplace_back(std::filesystem::path{ctx.options().m_toolchain_root} / "bin");
		if (ctx.toolchain().m_toolchain_root.has_value()) toolchain_dirs.emplace_back(std::filesystem::path{*ctx.toolchain().m_toolchain_root} / "bin");
		if (!toolchain_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, toolchain_dirs, tool_provider::toolchain_root, 60, "toolchain root", true, ctx.m_mode);

		if (!ctx.options().m_sdk_root.empty()) sdk_dirs.emplace_back(std::filesystem::path{ctx.options().m_sdk_root} / "bin");
		if (ctx.toolchain().m_sdk_root.has_value()) sdk_dirs.emplace_back(std::filesystem::path{*ctx.toolchain().m_sdk_root} / "bin");
		if (!sdk_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, sdk_dirs, tool_provider::sdk_root, 70, "SDK root", true, ctx.m_mode);

		if (!ctx.options().m_sysroot.empty()) sysroot_dirs.emplace_back(std::filesystem::path{ctx.options().m_sysroot} / "bin");
		if (ctx.toolchain().sysroot().has_value()) sysroot_dirs.emplace_back(std::filesystem::path{*ctx.toolchain().sysroot()} / "bin");
		if (!sysroot_dirs.empty()) add_candidates_from_dirs(out, ctx.request(), req, sysroot_dirs, tool_provider::sysroot_root, 80, "sysroot bin directory", true, ctx.m_mode);
	}

	inline void add_sibling_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;

		for (const auto& tool : ctx.m_resolved_tools) {
			if (tool.m_path.has_parent_path()) {
				dirs.emplace_back(tool.m_path.parent_path());
			}
		}

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::sibling, 90, "sibling of an already resolved tool", true, ctx.m_mode);
	}

	inline void add_target_prefix_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		for (const auto& prefixed_name : target_prefixed_names_for(ctx.request(), req.m_role, ctx.m_mode)) {
			for (const auto& dir : path_entries()) {
				add_candidate_if_found(out, req, prefixed_name, dir, tool_provider::target_prefix, 100, "target-prefixed tool on PATH");
			}
		}
	}

	inline void add_path_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		add_candidates_from_dirs(out, ctx.request(), req, path_entries(), tool_provider::path, 110, "PATH", true, ctx.m_mode);
	}

	inline void add_known_install_root_candidates(context& ctx, const tool_requirement& req, candidate_list& out) {
		std::vector<std::filesystem::path> dirs;

#if defined(_WIN32)
		dirs.emplace_back("C:/Program Files/LLVM/bin");
		dirs.emplace_back("C:/Program Files (x86)/LLVM/bin");
		dirs.emplace_back("C:/Tools/Ninja");
#elif defined(__APPLE__)
		dirs.emplace_back("/opt/homebrew/bin");
		dirs.emplace_back("/usr/local/bin");
		dirs.emplace_back("/opt/local/bin");
#else
		dirs.emplace_back("/usr/local/bin");
		dirs.emplace_back("/usr/bin");
		dirs.emplace_back("/bin");
		dirs.emplace_back("/opt/bin");
#endif

		add_candidates_from_dirs(out, ctx.request(), req, dirs, tool_provider::known_install_root, 120, "known install root", true, ctx.m_mode);
	}

	void add_windows_registry_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_vswhere_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_visual_studio_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_windows_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_standalone_llvm_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_msys2_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_mingw_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_cygwin_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_xcrun_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_homebrew_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_macports_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_unix_system_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_distro_llvm_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_distro_gcc_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_android_ndk_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_emscripten_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out);
	void add_embedded_sdk_candidates(context& ctx, const tool_requirement& req, candidate_list& out);

	[[nodiscard]] inline candidate_list candidates_for(context& ctx, const tool_requirement& req) {
		candidate_list out;

		add_explicit_path_candidates(ctx, req, out);
		add_cli_override_candidates(ctx, req, out);
		add_environment_override_candidates(ctx, req, out);
		add_cache_candidates(ctx, req, out);
		add_root_candidates(ctx, req, out);
		add_sibling_candidates(ctx, req, out);
		add_target_prefix_candidates(ctx, req, out);
		add_path_candidates(ctx, req, out);
		add_known_install_root_candidates(ctx, req, out);
		add_windows_registry_candidates(ctx, req, out);
		add_vswhere_candidates(ctx, req, out);
		add_visual_studio_candidates(ctx, req, out);
		add_windows_sdk_candidates(ctx, req, out);
		add_standalone_llvm_candidates(ctx, req, out);
		add_msys2_candidates(ctx, req, out);
		add_mingw_candidates(ctx, req, out);
		add_cygwin_candidates(ctx, req, out);
		add_xcrun_candidates(ctx, req, out);
		add_homebrew_candidates(ctx, req, out);
		add_macports_candidates(ctx, req, out);
		add_unix_system_candidates(ctx, req, out);
		add_distro_llvm_candidates(ctx, req, out);
		add_distro_gcc_candidates(ctx, req, out);
		add_android_ndk_candidates(ctx, req, out);
		add_emscripten_sdk_candidates(ctx, req, out);
		add_embedded_sdk_candidates(ctx, req, out);

		std::ranges::sort(out, {}, &tool_candidate::m_priority);
		return out;
	}
}

#endif
