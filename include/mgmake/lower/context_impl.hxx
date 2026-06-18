#pragma once

#ifndef MGMK_LOWER_CONTEXT_IMPL_HXX
#define MGMK_LOWER_CONTEXT_IMPL_HXX

#include "context.hxx"
#include "objects.hxx"
#include "../build/artifact_names.hxx"
#include "../build/target.hxx"
#include "../detail/assert.hxx"
#include "../discovery/tool_role.hxx"
#include "../ext/fetch.hxx"
#include "../spec/project.hxx"
#include "../sys/command_line.hxx"
#include "../sys/file_command.hxx"

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::lower {
	[[nodiscard]] inline std::filesystem::path fetch_root(const build::request& req) {
		return req.build_dir() / "ext";
	}

	[[nodiscard]] inline std::filesystem::path fetch_source_dir(
		const build::request& req,
		std::string_view name
	) {
		return fetch_root(req) / "src" / std::string{name};
	}

	[[nodiscard]] inline std::filesystem::path fetch_tmp_dir(
		const build::request& req,
		std::string_view name
	) {
		return fetch_root(req) / "tmp" / std::string{name};
	}

	[[nodiscard]] inline std::filesystem::path fetch_stamp(
		const build::request& req,
		std::string_view name,
		std::string_view suffix
	) {
		return fetch_root(req) / "stamp" / (std::string{name} + "." + std::string{suffix});
	}

	[[nodiscard]] inline std::filesystem::path archive_extension(ext::archive_format format) {
		switch (format) {
			case ext::archive_format::zip: return ".zip";
			case ext::archive_format::tar: return ".tar";
			case ext::archive_format::tar_gz: return ".tar.gz";
			case ext::archive_format::tar_xz: return ".tar.xz";
			case ext::archive_format::auto_detect: return ".archive";
		}

		return ".archive";
	}

	[[nodiscard]] inline std::filesystem::path fetch_archive_path(
		const build::request& req,
		std::string_view name,
		ext::archive_format format
	) {
		return fetch_root(req) / "archive" / (std::string{name} + archive_extension(format).string());
	}

	[[nodiscard]] inline bool string_ends_with(std::string_view text, std::string_view suffix) noexcept {
		return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
	}

	[[nodiscard]] inline ext::archive_format resolve_archive_format(
		const ext::archive_fetch& archive
	) {
		if (archive.m_format != ext::archive_format::auto_detect) {
			return archive.m_format;
		}

		std::string url = archive.m_url;
		std::ranges::transform(url, url.begin(), [](unsigned char ch) {
			return static_cast<char>(std::tolower(ch));
		});

		if (string_ends_with(url, ".zip")) {
			return ext::archive_format::zip;
		}

		if (string_ends_with(url, ".tar")) {
			return ext::archive_format::tar;
		}

		if (string_ends_with(url, ".tar.gz") || string_ends_with(url, ".tgz")) {
			return ext::archive_format::tar_gz;
		}

		if (string_ends_with(url, ".tar.xz") || string_ends_with(url, ".txz")) {
			return ext::archive_format::tar_xz;
		}

		mgmkassert(false, "mgmake lower: archive format auto-detect could not infer archive type from URL '" + archive.m_url + "'");
		return ext::archive_format::auto_detect;
	}

	[[nodiscard]] inline sys::command_line archive_download_command(
		const build::request& req,
		const std::filesystem::path& archive_path,
		std::string_view url
	) {
		const auto* downloader = req.discovered_tool_any({
			discovery::tool_role::curl,
			discovery::tool_role::wget
		});

		mgmkassert(downloader != nullptr, "mgmake lower: archive fetch requires curl or wget");

		sys::command_line command{};
		command.m_args.emplace_back(downloader->path_string());

		switch (downloader->m_role) {
			case discovery::tool_role::curl:
				command.m_args.emplace_back("-L");
				command.m_args.emplace_back("-o");
				command.m_args.emplace_back(archive_path.string());
				command.m_args.emplace_back(std::string{url});
				break;

			case discovery::tool_role::wget:
				command.m_args.emplace_back("-O");
				command.m_args.emplace_back(archive_path.string());
				command.m_args.emplace_back(std::string{url});
				break;

			default:
				mgmkassert(false, "mgmake lower: unsupported archive downloader");
				break;
		}

		return command;
	}

	[[nodiscard]] inline sys::command_line archive_extract_command(
		const build::request& req,
		ext::archive_format format,
		const std::filesystem::path& archive_path,
		const std::filesystem::path& tmp_dir
	) {
		sys::command_line command{};

		switch (format) {
			case ext::archive_format::zip: {
				const auto unzip = req.tool_path(discovery::tool_role::unzip, "unzip");
				command.m_args.emplace_back(unzip.string());
				command.m_args.emplace_back(archive_path.string());
				command.m_args.emplace_back("-d");
				command.m_args.emplace_back(tmp_dir.string());
				break;
			}

			case ext::archive_format::tar:
			case ext::archive_format::tar_gz:
			case ext::archive_format::tar_xz: {
				const auto tar = req.tool_path(discovery::tool_role::tar, "tar");
				command.m_args.emplace_back(tar.string());

				switch (format) {
					case ext::archive_format::tar: command.m_args.emplace_back("-xf"); break;
					case ext::archive_format::tar_gz: command.m_args.emplace_back("-xzf"); break;
					case ext::archive_format::tar_xz: command.m_args.emplace_back("-xJf"); break;
					default: break;
				}

				command.m_args.emplace_back(archive_path.string());
				command.m_args.emplace_back("-C");
				command.m_args.emplace_back(tmp_dir.string());
				break;
			}

			case ext::archive_format::auto_detect:
				mgmkassert(false, "mgmake lower: archive format auto-detect must be resolved before extraction");
				break;
		}

		return command;
	}

	inline context::context(
		dag::graph& graph,
		const build::request& req,
		const spec::project& project
	)
		: m_req{req}
		, m_project{project}
		, m_emit{graph}
		, m_libraries(project.m_libraries.size())
		, m_fetches(project.m_fetches.size()) {}

	inline const lower::fetched& context::lower_fetch(ext::fetch::id id) {
		mgmkassert(id < m_project.m_fetches.size(), "mgmake lower: invalid fetch id");

		if (m_fetches.at(id).has_value()) {
			return m_fetches.at(id).value();
		}

		const auto& fetch = m_project.m_fetches.at(id);
		m_fetches.at(id) = lower_fetch_value(fetch);
		return m_fetches.at(id).value();
	}

	inline lower::fetched context::lower_fetch_value(const ext::fetch& fetch) {
		mgmkassert(!fetch.m_name.empty(), "mgmake lower: fetch has no name");

		if (auto existing = m_named_fetches.find(fetch.m_name); existing != m_named_fetches.end()) {
			return existing->second;
		}

		mgmkassert(
			!m_active_fetches.contains(fetch.m_name),
			"mgmake lower: cyclic fetch dependency involving '" + fetch.m_name + "'"
		);
		m_active_fetches.emplace(fetch.m_name);

		lower::fetched result{};

		if (const auto* git = std::get_if<ext::git_fetch>(&fetch.m_data)) {
			result = lower_git_fetch(fetch, *git);
		} else if (const auto* archive = std::get_if<ext::archive_fetch>(&fetch.m_data)) {
			result = lower_archive_fetch(fetch, *archive);
		} else if (const auto* local = std::get_if<ext::local_fetch>(&fetch.m_data)) {
			result = lower_local_fetch(fetch, *local);
		} else {
			mgmkassert(false, "mgmake lower: unsupported fetch kind for '" + fetch.m_name + "'");
		}

		m_active_fetches.erase(fetch.m_name);
		m_named_fetches.emplace(fetch.m_name, result);
		return result;
	}

	inline lower::fetched context::lower_git_fetch(
		const ext::fetch& fetch,
		const ext::git_fetch& git
	) {
		const auto src_dir = fetch_source_dir(request(), fetch.m_name);
		const auto prepare_stamp = fetch_stamp(request(), fetch.m_name, "prepare");
		const auto final_stamp = fetch_stamp(request(), fetch.m_name, "fetch");

		const auto prepare_id = m_emit.generated(prepare_stamp);
		const auto source_id = m_emit.generated(src_dir);
		const auto stamp_id = m_emit.generated(final_stamp);

		m_emit.action(
			"Prepare fetch " + fetch.m_name,
			"Prepares external fetch '" + fetch.m_name + "'.",
			{},
			{prepare_id},
			sys::reset_directory_stamp_command(src_dir, prepare_stamp)
		);

		const auto* git_tool = request().discovered_tool(discovery::tool_role::git);
		const auto git_path = git_tool ? git_tool->path_string() : std::string{"git"};

		sys::command_line clone{};
		clone.m_args.emplace_back(git_path);
		clone.m_args.emplace_back("clone");

		if (git.m_shallow) {
			clone.m_args.emplace_back("--depth");
			clone.m_args.emplace_back("1");
		}

		if (git.m_submodules) {
			clone.m_args.emplace_back("--recurse-submodules");
		}

		if (!git.m_ref.empty()) {
			clone.m_args.emplace_back("--branch");
			clone.m_args.emplace_back(git.m_ref);
		}

		clone.m_args.emplace_back(git.m_url);
		clone.m_args.emplace_back(src_dir.string());

		m_emit.action(
			"Clone fetch " + fetch.m_name,
			"Clones external git source '" + fetch.m_name + "'.",
			{prepare_id},
			{source_id},
			std::move(clone)
		);

		m_emit.action(
			"Stamp fetch " + fetch.m_name,
			"Marks external fetch '" + fetch.m_name + "' complete.",
			{source_id},
			{stamp_id},
			sys::touch_command(final_stamp)
		);

		dag::target dag_target{
			"ext:fetch:" + fetch.m_name,
			{stamp_id},
			{}
		};

		return lower::fetched{
			.m_target = m_emit.target(std::move(dag_target)),
			.m_stamp = stamp_id,
			.m_source_dir = src_dir
		};
	}

	inline lower::fetched context::lower_archive_fetch(
		const ext::fetch& fetch,
		const ext::archive_fetch& archive
	) {
		const auto format = resolve_archive_format(archive);
		const auto src_dir = fetch_source_dir(request(), fetch.m_name);
		const auto tmp_dir = fetch_tmp_dir(request(), fetch.m_name);
		const auto archive_path = fetch_archive_path(request(), fetch.m_name, format);
		const auto prepare_stamp = fetch_stamp(request(), fetch.m_name, "prepare");
		const auto final_stamp = fetch_stamp(request(), fetch.m_name, "fetch");

		const auto prepare_id = m_emit.generated(prepare_stamp);
		const auto archive_id = m_emit.generated(archive_path);
		const auto tmp_id = m_emit.generated(tmp_dir);
		const auto stamp_id = m_emit.generated(final_stamp);

		m_emit.action(
			"Prepare fetch " + fetch.m_name,
			"Prepares external archive fetch '" + fetch.m_name + "'.",
			{},
			{prepare_id},
			sys::reset_directory_stamp_command(tmp_dir, prepare_stamp)
		);

		m_emit.action(
			"Download fetch " + fetch.m_name,
			"Downloads external archive source '" + fetch.m_name + "'.",
			{prepare_id},
			{archive_id},
			archive_download_command(request(), archive_path, archive.m_url)
		);

		m_emit.action(
			"Extract fetch " + fetch.m_name,
			"Extracts external archive source '" + fetch.m_name + "'.",
			{archive_id, prepare_id},
			{tmp_id},
			archive_extract_command(request(), format, archive_path, tmp_dir)
		);

		const auto normalized_from = archive.m_strip_prefix.empty()
			? tmp_dir
			: tmp_dir / archive.m_strip_prefix;

		m_emit.action(
			"Normalize fetch " + fetch.m_name,
			"Normalizes external archive source '" + fetch.m_name + "'.",
			{tmp_id},
			{stamp_id},
			sys::normalize_directory_stamp_command(normalized_from, src_dir, final_stamp)
		);

		dag::target dag_target{
			"ext:fetch:" + fetch.m_name,
			{stamp_id},
			{}
		};

		return lower::fetched{
			.m_target = m_emit.target(std::move(dag_target)),
			.m_stamp = stamp_id,
			.m_source_dir = src_dir
		};
	}

	inline lower::fetched context::lower_local_fetch(
		const ext::fetch& fetch,
		const ext::local_fetch& local
	) {
		const auto stamp_path = fetch_stamp(request(), fetch.m_name, "fetch");
		const auto stamp_id = m_emit.generated(stamp_path);

		m_emit.action(
			"Validate fetch " + fetch.m_name,
			"Validates local external source '" + fetch.m_name + "'.",
			{},
			{stamp_id},
			sys::validate_path_command(local.m_path, stamp_path)
		);

		dag::target dag_target{
			"ext:fetch:" + fetch.m_name,
			{stamp_id},
			{}
		};

		return lower::fetched{
			.m_target = m_emit.target(std::move(dag_target)),
			.m_stamp = stamp_id,
			.m_source_dir = local.m_path
		};
	}

	inline lower::usage context::use_libraries(
		const std::set<std::string>& libraries,
		std::string_view owner_name
	) {
		lower::usage result{};

		for (const auto& library_name : libraries) {
			const auto linked_id = m_project.find_library(library_name);

			mgmkassert(
				linked_id.has_value(),
				"mgmake lower: target '" + std::string{owner_name} +
					"' links unknown library '" + library_name + "'"
			);

			const lower::target& dep = lower_library(linked_id.value());

			if (dep.m_dag_target.has_value()) {
				result.m_dag_dependencies.emplace(dep.m_dag_target.value());
			}

			result.m_link_inputs.insert(
				result.m_link_inputs.end(),
				dep.m_linkable_artifacts.begin(),
				dep.m_linkable_artifacts.end()
			);

			result.m_include_dirs.insert_range(dep.m_include_dirs);
		}

		return result;
	}

	inline const lower::target& context::lower_library(spec::library::id id) {
		mgmkassert(
			id < m_libraries.size(),
			"mgmake lower: invalid library id"
		);

		if (m_libraries.at(id).has_value()) {
			return m_libraries.at(id).value();
		}

		const auto& lib = m_project.m_libraries.at(id);

		mgmkassert(not lib.m_name.empty(), "mgmake lower: library target has no name");

		mgmkassert(
			not m_active_libraries.contains(id),
			"mgmake lower: cyclic library dependency involving '" + lib.m_name + "'"
		);

		m_active_libraries.emplace(id);

		lower::usage usage = use_libraries(
			lib.linked_libraries(),
			lib.m_name
		);

		switch (lib.m_kind) {
			case spec::library::kind::interface:
				m_libraries.at(id) = lower_interface_library(lib, std::move(usage));
				break;

			case spec::library::kind::static_lib:
				m_libraries.at(id) = lower_static_library(lib, std::move(usage));
				break;

			case spec::library::kind::shared_lib:
				m_libraries.at(id) = lower_shared_library(lib, std::move(usage));
				break;
		}

		m_active_libraries.erase(id);

		return m_libraries.at(id).value();
	}

	inline lower::target context::lower_interface_library(
		const spec::library& lib,
		lower::usage usage
	) {
		mgmkassert(
			lib.m_sources.empty(),
			"mgmake lower: interface library '" + lib.m_name + "' cannot have sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);
		auto link_inputs = std::move(usage.m_link_inputs);

		dag::target dag_target{
			lib.m_name,
			{},
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts = std::move(link_inputs);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline lower::target context::lower_static_library(
		const spec::library& lib,
		lower::usage usage
	) {
		const auto& tc = toolchain();

		mgmkassert(
			not lib.m_sources.empty(),
			"mgmake lower: static library '" + lib.m_name + "' has no sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(lib, include_dirs);

		std::filesystem::path archive_path;

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc:
				archive_path = request().build_dir() / "lib" / ("lib" + lib.m_name + ".a");
				break;

			case build::toolchain::dialect::msvc:
				archive_path = request().build_dir() / "lib" / (lib.m_name + ".lib");
				break;
		}

		auto archive_id = m_emit.generated(archive_path);

		sys::command_line command{};
		command.m_args.emplace_back(tc.ar());

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc: {
				if (tc.archive_flags().empty()) {
					command.m_args.emplace_back("rcs");
				}
				else {
					for (const auto& flag : tc.archive_flags()) {
						command.m_args.emplace_back(flag);
					}
				}

				command.m_args.emplace_back(archive_path.string());

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(m_emit.path(object_id).string());
				}

				break;
			}

			case build::toolchain::dialect::msvc: {
				for (const auto& flag : tc.archive_flags()) {
					command.m_args.emplace_back(flag);
				}

				command.m_args.emplace_back(std::string{"/OUT:"} + archive_path.string());

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(m_emit.path(object_id).string());
				}

				break;
			}
		}

		m_emit.action(
			std::string{"Build static library "} + lib.m_name,
			std::string{"Builds static library target '"} + lib.m_name + "'.",
			object_ids,
			{ archive_id },
			std::move(command)
		);

		dag::target dag_target{
			lib.m_name,
			{ archive_id },
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts.emplace_back(archive_id);
		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline lower::target context::lower_shared_library(
		const spec::library& lib,
		lower::usage usage
	) {
		const auto& tc = toolchain();

		mgmkassert(
			not lib.m_sources.empty(),
			"mgmake lower: shared library '" + lib.m_name + "' has no sources"
		);

		mgmkassert(
			tc.dialect() == build::toolchain::dialect::gcc,
			"mgmake lower: shared library lowering is currently only implemented for GCC-like toolchains"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(lib, include_dirs);

		const auto platform = request().target_platform();
		std::filesystem::path shared_path =
			request().build_dir() /
			"lib" /
			(
				std::string{ build::shared_library_prefix(platform) } +
				lib.m_name +
				std::string{ build::shared_library_extension(platform) }
			);

		auto shared_id = m_emit.generated(shared_path);

		sys::command_line command{};
		command.m_args.emplace_back(tc.linker());

		build::append_target_args(command, tc, request());

		for (const auto& arg : request().link_prefix_args()) {
			command.m_args.emplace_back(arg);
		}

		const auto shared_flag = build::shared_library_link_flag(platform);

		mgmkassert(
			!shared_flag.empty(),
			"mgmake lower: shared library lowering is not supported for requested target platform"
		);

		command.m_args.emplace_back(shared_flag);

		for (auto object_id : object_ids) {
			command.m_args.emplace_back(m_emit.path(object_id).string());
		}

		for (auto link_input : usage.m_link_inputs) {
			command.m_args.emplace_back(m_emit.path(link_input).string());
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		command.m_args.emplace_back("-o");
		command.m_args.emplace_back(shared_path.string());

		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_link_inputs.begin(), usage.m_link_inputs.end());

		m_emit.action(
			std::string{"Build shared library "} + lib.m_name,
			std::string{"Builds shared library target '"} + lib.m_name + "'.",
			std::move(inputs),
			{ shared_id },
			std::move(command)
		);

		dag::target dag_target{
			lib.m_name,
			{ shared_id },
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(std::move(dag_target));
		lowered.m_linkable_artifacts.emplace_back(shared_id);
		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		return lowered;
	}

	inline void context::lower_executable(spec::executable::id id) {
		mgmkassert(
			id < m_project.m_executables.size(),
			"mgmake lower: invalid executable id"
		);

		const auto& exe = m_project.m_executables.at(id);
		const auto& tc = toolchain();

		mgmkassert(not exe.m_name.empty(), "mgmake lower: executable target has no name");
		mgmkassert(
			not exe.m_sources.empty(),
			"mgmake lower: executable target '" + exe.m_name + "' has no sources"
		);

		lower::usage usage = use_libraries(
			exe.linked_libraries(),
			exe.m_name
		);

		auto include_dirs = exe.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(exe, include_dirs);
		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_link_inputs.begin(), usage.m_link_inputs.end());

		std::filesystem::path output =
			request().build_dir() /
			(
				exe.m_name +
				std::string{ build::executable_extension(request().target_platform()) }
			);

		auto output_id = m_emit.generated(output);

		sys::command_line command{};
		command.m_args.emplace_back(tc.cxx());

		build::append_target_args(command, tc, request());

		for (const auto& arg : request().link_prefix_args()) {
			command.m_args.emplace_back(arg);
		}

		for (auto object_id : object_ids) {
			command.m_args.emplace_back(m_emit.path(object_id).string());
		}

		for (auto link_input : usage.m_link_inputs) {
			command.m_args.emplace_back(m_emit.path(link_input).string());
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc:
				command.m_args.emplace_back("-o");
				command.m_args.emplace_back(output.string());
				break;

			case build::toolchain::dialect::msvc:
				command.m_args.emplace_back(std::string{"/Fe"} + output.string());
				break;
		}

		m_emit.action(
			std::string{"Build executable "} + exe.m_name,
			std::string{"Builds executable target '"} + exe.m_name + "'.",
			std::move(inputs),
			{ output_id },
			std::move(command)
		);

		dag::target dag_target{
			exe.m_name,
			{ output_id },
			std::move(usage.m_dag_dependencies)
		};

		m_emit.target(std::move(dag_target));
	}
}

#endif
