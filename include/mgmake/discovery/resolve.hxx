#pragma once

#ifndef MGMAKE_DISCOVERY_RESOLVE_HXX
#define MGMAKE_DISCOVERY_RESOLVE_HXX

#include "backend_requirement.hxx"
#include "android/ndk.hxx"
#include "diagnostic.hxx"
#include "environment_provider.hxx"
#include "providers.hxx"
#include "resolved_toolchain.hxx"
#include "validate.hxx"

#include <expected>
#include <string>

namespace mgmake::discovery {
	[[nodiscard]] inline discovery::mode effective_discovery_mode(
		const cli::options& opts,
		const build::toolchain& tc
	) {
		if (opts.m_tool_discovery == discovery::mode::automatic) {
			return tc.discovery_mode();
		}

		return opts.m_tool_discovery;
	}

	[[nodiscard]] inline std::expected<resolved_tool, std::string> resolve_tool(
		context& ctx,
		const tool_requirement& req
	) {
		diagnostic diag;
		diag.m_role = req.m_role;
		diag.m_toolchain = std::string{ctx.toolchain().name()};
		diag.m_logical_name = req.m_logical_name;
		diag.m_needed_because = req.m_needed_because;

		for (const auto& candidate : candidates_for(ctx, req)) {
			ctx.m_searched.push_back({
				.m_candidate = candidate,
				.m_status = "checking"
			});

			diag.m_searched.emplace_back(
				std::string{name(candidate.m_provider)} + ": " + candidate.m_path.string()
			);

			auto validated = validate_candidate(ctx, req, candidate);

			if (validated) {
				ctx.m_resolved_tools.emplace_back(*validated);
				ctx.m_searched.back().m_status = "accepted";
				return validated;
			}

			ctx.m_searched.back().m_status = "rejected";
			ctx.m_rejected.push_back({
				.m_candidate = candidate,
				.m_reason = validated.error()
			});
			diag.m_rejected.emplace_back(candidate.m_path.string() + ": " + validated.error());

			if (candidate.m_authoritative) {
				diag.m_fixes = fixes_for(req);
				return std::unexpected{diag.format_missing_tool()};
			}
		}

		if (req.required()) {
			diag.m_fixes = fixes_for(req);
			return std::unexpected{diag.format_missing_tool()};
		}

		return std::unexpected{std::string{}};
	}

	[[nodiscard]] inline cache_entry make_cache_entry(
		const build::request& req,
		const resolved_tool& tool
	) {
		cache_entry entry;
		entry.m_toolchain = std::string{req.toolchain().name()};
		entry.m_host = sys::g_host_target;
		entry.m_target = req.target();
		entry.m_host_key = target_key(entry.m_host);
		entry.m_target_key = target_key(entry.m_target);
		entry.m_role = tool.m_role;
		entry.m_logical_name = tool.m_logical_name;
		entry.m_path = tool.m_path;
		entry.m_provider = tool.m_provider;
		entry.m_version = tool.m_version;
		return entry;
	}

	inline void apply_resolved_toolchain(
		build::request& req,
		const resolved_toolchain& tc
	) {
		req.m_resolved_toolchain = tc;

		if (const auto* cc = tc.find(tool_role::c_compiler)) {
			req.m_tc.cc(cc->path_string());
		}

		if (const auto* cxx = tc.find(tool_role::cxx_compiler)) {
			req.m_tc.cxx(cxx->path_string());
		}

		if (const auto* ar = tc.find(tool_role::archiver)) {
			req.m_tc.ar(ar->path_string());
		} else if (const auto* lib = tc.find(tool_role::librarian)) {
			req.m_tc.ar(lib->path_string());
		}

		if (const auto* linker = tc.find(tool_role::linker)) {
			req.m_tc.linker(linker->path_string());
		} else if (const auto* shared = tc.find(tool_role::shared_linker)) {
			req.m_tc.linker(shared->path_string());
		}

		for (const auto& tool : tc.m_tools) {
			switch (tool.m_role) {
				case tool_role::c_compiler:
				case tool_role::cxx_compiler:
				case tool_role::archiver:
				case tool_role::librarian:
				case tool_role::linker:
				case tool_role::shared_linker:
					break;

				default:
					req.m_tc.tool(tool.m_role, tool.path_string());
					break;
			}
		}
	}

	[[nodiscard]] inline std::expected<resolved_toolchain, std::string> resolve_toolchain(
		const cli::options& opts,
		const build::request& req,
		const spec::project& project
	) {
		const auto discovery_mode = effective_discovery_mode(opts, req.toolchain());

		resolved_toolchain resolved_tc;
		resolved_tc.m_name = std::string{req.toolchain().name()};
		resolved_tc.m_requested_name = std::string{req.toolchain().name()};
		resolved_tc.m_mode = discovery_mode;
		resolved_tc.m_host = sys::g_host_target;
		resolved_tc.m_target = req.target();

		if (discovery_mode == mode::disabled) {
			return resolved_tc;
		}

		context ctx;
		ctx.m_options = &opts;
		ctx.m_project = &project;
		ctx.m_request = &req;
		ctx.m_use_cache = !opts.m_no_tool_cache;
		ctx.m_refresh_cache = opts.m_refresh_tools;
		ctx.m_verbose = opts.m_verbose;
		ctx.m_show_search = opts.m_show_tool_search;
		ctx.m_mode = discovery_mode;

		if (ctx.m_use_cache) {
			ctx.m_cache = load_cache(req);
		}

		for (const auto& requirement : required_tools(opts, req, project)) {
			auto tool = resolve_tool(ctx, requirement);

			if (!tool) {
				if (requirement.required()) {
					return std::unexpected{tool.error()};
				}

				continue;
			}

			resolved_tc.m_tools.emplace_back(*tool);

			if (ctx.m_use_cache) {
				ctx.m_cache.put(make_cache_entry(req, *tool));
			}
		}

		for (const auto& tool : resolved_tc.m_tools) {
			if (tool.m_provider != tool_provider::android_ndk || !tool.m_provider_root.has_value()) {
				continue;
			}

			auto args = android::target_sysroot_args(
				*tool.m_provider_root,
				android_host_tag(),
				opts.m_android_abi,
				opts.m_android_api
			);

			resolved_tc.m_compile_prefix_args.insert(
				resolved_tc.m_compile_prefix_args.end(),
				args.begin(),
				args.end()
			);
			resolved_tc.m_link_prefix_args.insert(
				resolved_tc.m_link_prefix_args.end(),
				args.begin(),
				args.end()
			);
			break;
		}

		resolved_tc.m_searched = std::move(ctx.m_searched);
		resolved_tc.m_rejected = std::move(ctx.m_rejected);

		build::request bridge = req;
		apply_resolved_toolchain(bridge, resolved_tc);
		resolved_tc.m_environment = discover_tool_environment(opts, bridge, project);

		if (ctx.m_use_cache) {
			save_cache(req, ctx.m_cache);
		}

		return resolved_tc;
	}

	[[nodiscard]] inline std::expected<build::request, std::string> resolve_request(
		const cli::options& opts,
		const build::request& req,
		const spec::project& project
	) {
		auto toolchain = resolve_toolchain(opts, req, project);

		if (!toolchain) {
			return std::unexpected{toolchain.error()};
		}

		build::request resolved = req;
		apply_resolved_toolchain(resolved, *toolchain);
		return resolved;
	}

	[[nodiscard]] inline std::expected<resolved_tool, std::string> resolve_backend_tool(
		const cli::options& opts,
		const build::request& req,
		backend_tool_requirement requirement
	) {
		const auto discovery_mode = effective_discovery_mode(opts, req.toolchain());

		if (discovery_mode == mode::disabled) {
			return resolved_tool{
				.m_role = requirement.m_role,
				.m_logical_name = requirement.m_logical_name,
				.m_path = requirement.m_logical_name,
				.m_provider = tool_provider::explicit_path,
				.m_reason = "tool discovery disabled"
			};
		}

		context ctx;
		ctx.m_options = &opts;
		ctx.m_request = &req;
		ctx.m_use_cache = !opts.m_no_tool_cache;
		ctx.m_refresh_cache = opts.m_refresh_tools;
		ctx.m_verbose = opts.m_verbose;
		ctx.m_show_search = opts.m_show_tool_search;
		ctx.m_mode = discovery_mode;

		if (ctx.m_use_cache) {
			ctx.m_cache = load_cache(req);
		}

		tool_requirement req_tool;
		req_tool.m_role = requirement.m_role;
		req_tool.m_logical_name = std::move(requirement.m_logical_name);
		req_tool.m_needed_because = std::move(requirement.m_needed_because);

		auto resolved = resolve_tool(ctx, req_tool);

		if (resolved && ctx.m_use_cache) {
			ctx.m_cache.put(make_cache_entry(req, *resolved));
			save_cache(req, ctx.m_cache);
		}

		return resolved;
	}
}

#endif
