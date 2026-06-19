#pragma once

#ifndef MGMK_PREP_CONTEXT_IMPL_HXX
#define MGMK_PREP_CONTEXT_IMPL_HXX

#include "context.hxx"
#include "../detail/assert.hxx"
#include "../discovery/tool_role.hxx"
#include "../ext/fetch.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "../ext/cmake/file_api.hxx"
#include "../ext/cmake.hxx"
#endif
#include "../spec/project.hxx"
#include "../sys/command_line.hxx"
#include "../sys/file_command.hxx"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::prep {
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

#ifdef MGMK_ENABLE_EXT_CMAKE
	[[nodiscard]] inline std::filesystem::path cmake_source_dir(
		const build::request& req,
		std::string_view name
	) {
		return fetch_root(req) / "src" / std::string{name};
	}

	[[nodiscard]] inline std::filesystem::path cmake_build_dir(
		const build::request& req,
		std::string_view name
	) {
		return fetch_root(req) / "build" / std::string{name};
	}

	[[nodiscard]] inline std::filesystem::path cmake_install_dir(
		const build::request& req,
		std::string_view name
	) {
		return fetch_root(req) / "install" / std::string{name};
	}

	[[nodiscard]] inline sys::command_line cmake_write_query_command(
		const std::filesystem::path& query_path
	) {
		const auto query_dir = query_path.parent_path();
		const auto query_text = ext::cmake_file_api::codemodel_query_text();
#if defined(MGMK_PLATFORM_WINDOWS)
		return sys::shell_command(
			"if not exist " + sys::shell_path(query_dir) + " mkdir " + sys::shell_path(query_dir) +
			" & echo " + query_text + " > " + sys::shell_path(query_path)
		);
#else
		return sys::shell_command(
			"mkdir -p " + sys::shell_path(query_dir) +
			" && printf %s " + sys::shell_escape(query_text) +
			" > " + sys::shell_path(query_path)
		);
#endif
	}

	[[nodiscard]] inline std::filesystem::path cmake_configure_output(
		const std::filesystem::path& build_dir
	) {
		return build_dir / "CMakeCache.txt";
	}

	[[nodiscard]] inline sys::command_line cmake_configure_command(
		const build::request& req,
		const ext::cmake& cmake_project,
		const std::filesystem::path& source_dir,
		const std::filesystem::path& build_dir,
		const std::filesystem::path& install_dir
	) {
		sys::command_line command;
		command.m_args.emplace_back(req.tool_path(discovery::tool_role::cmake, "cmake").string());
		command.m_args.emplace_back("-S");
		command.m_args.emplace_back(source_dir.string());
		command.m_args.emplace_back("-B");
		command.m_args.emplace_back(build_dir.string());
		command.m_args.emplace_back("-DCMAKE_INSTALL_PREFIX=" + install_dir.string());

		if (!cmake_project.m_generator.empty()) {
			command.m_args.emplace_back("-G");
			command.m_args.emplace_back(cmake_project.m_generator);
		}

		if (!cmake_project.m_build_config.empty()) {
			command.m_args.emplace_back("-DCMAKE_BUILD_TYPE=" + cmake_project.m_build_config);
		}

		for (const auto& [key, value] : cmake_project.m_defines) {
			command.m_args.emplace_back("-D" + key + "=" + value);
		}

		for (const auto& arg : cmake_project.m_args) {
			command.m_args.emplace_back(arg);
		}

		return command;
	}
#endif

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

		mgmkassert(false, "mgmake prep: archive format auto-detect could not infer archive type from URL '" + archive.m_url + "'");
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

		mgmkassert(downloader != nullptr, "mgmake prep: archive fetch requires curl or wget");

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
				mgmkassert(false, "mgmake prep: unsupported archive downloader");
				break;
		}

		return command;
	}


	[[nodiscard]] inline std::filesystem::path fetch_complete_marker(
		const std::filesystem::path& source_dir
	) {
		return source_dir / ".mgmake-fetch-complete";
	}

	[[nodiscard]] inline sys::command_line git_clone_command(
		const std::string& git_path,
		const ext::git_fetch& git,
		const std::filesystem::path& source_dir
	) {
		sys::command_line command{};
		command.m_args.emplace_back(git_path);
		command.m_args.emplace_back("clone");

		if (git.m_shallow) {
			command.m_args.emplace_back("--depth");
			command.m_args.emplace_back("1");
		}

		if (git.m_submodules) {
			command.m_args.emplace_back("--recurse-submodules");
		}

		if (!git.m_ref.empty()) {
			command.m_args.emplace_back("--branch");
			command.m_args.emplace_back(git.m_ref);
		}

		command.m_args.emplace_back(git.m_url);
		command.m_args.emplace_back(source_dir.string());
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
				mgmkassert(false, "mgmake prep: archive format auto-detect must be resolved before extraction");
				break;
		}

		return command;
	}

	inline context::context(
		prep::result& result,
		const build::request& req,
		const spec::project& project
	)
		: m_result{result}
		, m_req{req}
		, m_project{project}
		, m_emit{result.m_dag}
		, m_fetches(project.m_fetches.size())
#ifdef MGMK_ENABLE_EXT_CMAKE
		, m_cmake_projects(project.m_cmake_projects.size())
#endif
	{}

	inline const prep::fetched& context::fetch(ext::fetch::id id) {
		mgmkassert(id < m_project.m_fetches.size(), "mgmake prep: invalid fetch id");

		if (m_fetches.at(id).has_value()) {
			return m_fetches.at(id).value();
		}

		const auto& fetch = m_project.m_fetches.at(id);
		m_fetches.at(id) = fetch_value(fetch);
		return m_fetches.at(id).value();
	}

	inline prep::fetched context::fetch_value(const ext::fetch& fetch) {
		mgmkassert(!fetch.m_name.empty(), "mgmake prep: fetch has no name");

		if (auto existing = m_named_fetches.find(fetch.m_name); existing != m_named_fetches.end()) {
			return existing->second;
		}

		mgmkassert(
			!m_active_fetches.contains(fetch.m_name),
			"mgmake prep: cyclic fetch dependency involving '" + fetch.m_name + "'"
		);
		m_active_fetches.emplace(fetch.m_name);

		prep::fetched result{};

		if (const auto* git = std::get_if<ext::git_fetch>(&fetch.m_data)) {
			result = git_fetch(fetch, *git);
		} else if (const auto* archive = std::get_if<ext::archive_fetch>(&fetch.m_data)) {
			result = archive_fetch(fetch, *archive);
		} else if (const auto* local = std::get_if<ext::local_fetch>(&fetch.m_data)) {
			result = local_fetch(fetch, *local);
		} else {
			mgmkassert(false, "mgmake prep: unsupported fetch kind for '" + fetch.m_name + "'");
		}

		m_active_fetches.erase(fetch.m_name);
		m_named_fetches.emplace(fetch.m_name, result);
		m_result.m_fetches.insert_or_assign(fetch.m_name, result);
		return result;
	}


#ifdef MGMK_ENABLE_EXT_CMAKE
	inline const prep::cmake_project& context::cmake(ext::cmake::id id) {
		mgmkassert(id < m_project.m_cmake_projects.size(), "mgmake prep: invalid CMake project id");

		if (m_cmake_projects.at(id).has_value()) {
			return m_cmake_projects.at(id).value();
		}

		const auto& cmake_project = m_project.m_cmake_projects.at(id);
		m_cmake_projects.at(id) = cmake_value(cmake_project);
		return m_cmake_projects.at(id).value();
	}

	inline prep::cmake_project context::cmake_value(
		const ext::cmake& cmake_project
	) {
		mgmkassert(!cmake_project.m_name.empty(), "mgmake prep: CMake project has no name");
		mgmkassert(cmake_project.m_source.has_value(), "mgmake prep: CMake project '" + cmake_project.m_name + "' has no source");

		const auto fetched = fetch_value(cmake_project.m_source.value());
		const auto source_dir = fetched.m_source_dir;
		const auto build_dir = cmake_build_dir(request(), cmake_project.m_name);
		const auto install_dir = cmake_install_dir(request(), cmake_project.m_name);
		const auto query_path = ext::cmake_file_api::query_file(build_dir);
		const auto configure_output = cmake_configure_output(build_dir);

		const auto query_id = m_emit.generated(query_path);
		const auto configure_id = m_emit.generated(configure_output);

		m_emit.action(
			"Write CMake File API query " + cmake_project.m_name,
			"Writes CMake File API query for external project '" + cmake_project.m_name + "'.",
			{fetched.m_stamp},
			{query_id},
			cmake_write_query_command(query_path)
		);

		m_emit.action(
			"Configure CMake project " + cmake_project.m_name,
			"Configures external CMake project '" + cmake_project.m_name + "'.",
			{query_id},
			{configure_id},
			cmake_configure_command(
				request(),
				cmake_project,
				source_dir,
				build_dir,
				install_dir
			)
		);

		dag::target dag_target{
			"ext:cmake:configure:" + cmake_project.m_name,
			{configure_id},
			{fetched.m_target}
		};
		m_emit.target(dag_target);

		prep::cmake_project result{};
		result.m_source_dir = source_dir;
		result.m_build_dir = build_dir;
		result.m_install_dir = install_dir;
		result.m_usage_root = cmake_project.m_install
			? ext::output_root::install_dir
			: ext::output_root::build_dir;

		m_result.m_cmake_projects.insert_or_assign(cmake_project.m_name, result);
		return result;
	}
#endif

	inline prep::fetched context::git_fetch(
		const ext::fetch& fetch,
		const ext::git_fetch& git
	) {
		const auto src_dir = fetch_source_dir(request(), fetch.m_name);
		const auto complete_marker = fetch_complete_marker(src_dir);
		const auto stamp_id = m_emit.generated(complete_marker);


		const auto* git_tool = request().discovered_tool(discovery::tool_role::git);
		const auto git_path = git_tool ? git_tool->path_string() : std::string{"git"};

		m_emit.action(
			"Clone fetch " + fetch.m_name,
			"Clones external git source '" + fetch.m_name + "'.",
			{},
			{stamp_id},
			git_clone_command(git_path, git, src_dir)
		);

		dag::target dag_target{
			"ext:fetch:" + fetch.m_name,
			{stamp_id},
			{}
		};

		return prep::fetched{
			.m_target = m_emit.target(dag_target),
			.m_stamp = stamp_id,
			.m_source_dir = src_dir
		};
	}

	inline prep::fetched context::archive_fetch(
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

		return prep::fetched{
			.m_target = m_emit.target(dag_target),
			.m_stamp = stamp_id,
			.m_source_dir = src_dir
		};
	}

	inline prep::fetched context::local_fetch(
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

		return prep::fetched{
			.m_target = m_emit.target(dag_target),
			.m_stamp = stamp_id,
			.m_source_dir = local.m_path
		};
	}

}

#endif
