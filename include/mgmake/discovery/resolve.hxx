#pragma once

#ifndef MGMAKE_DISCOVERY_RESOLVE_HXX
#define MGMAKE_DISCOVERY_RESOLVE_HXX

#include "backend_requirement.hxx"
#include "diagnostic.hxx"
#include "environment_provider.hxx"
#include "providers.hxx"
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
			diag.m_searched.emplace_back(
				std::string{name(candidate.m_provider)} + ": " + candidate.m_path.string()
			);

			auto validated = validate_candidate(ctx, req, candidate);

			if (validated) {
				ctx.m_resolved_tools.emplace_back(*validated);
				return validated;
			}

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

	[[nodiscard]] inline std::expected<build::request, std::string> resolve_request(
		const cli::options& opts,
		const build::request& req,
		const spec::project& project
	) {
		const auto discovery_mode = effective_discovery_mode(opts, req.toolchain());

		if (discovery_mode == mode::disabled) {
			return req;
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

		build::request resolved = req;

		for (const auto& requirement : required_tools(opts, req, project)) {
			auto tool = resolve_tool(ctx, requirement);

			if (!tool) {
				if (requirement.required()) {
					return std::unexpected{tool.error()};
				}

				continue;
			}

			resolved.m_discovered_tools.emplace_back(*tool);

			switch (tool->m_role) {
				case tool_role::c_compiler:
					resolved.m_tc.cc(tool->path_string());
					break;

				case tool_role::cxx_compiler:
					resolved.m_tc.cxx(tool->path_string());
					break;

				case tool_role::archiver:
				case tool_role::librarian:
					resolved.m_tc.ar(tool->path_string());
					break;

				case tool_role::linker:
				case tool_role::shared_linker:
					resolved.m_tc.linker(tool->path_string());
					break;

				default:
					resolved.m_tc.tool(tool->m_role, tool->path_string());
					break;
			}

			if (ctx.m_use_cache) {
				ctx.m_cache.put(make_cache_entry(req, *tool));
			}
		}

		resolved.m_tool_environment = discover_tool_environment(opts, resolved, project);

		if (ctx.m_use_cache) {
			save_cache(req, ctx.m_cache);
		}

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
