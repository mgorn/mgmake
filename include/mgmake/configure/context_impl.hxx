#pragma once

#ifndef MGMK_CONFIGURE_CONTEXT_IMPL_HXX
#define MGMK_CONFIGURE_CONTEXT_IMPL_HXX

#include "context.hxx"
#include "../detail/assert.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "cmake/command.hxx"
#include "cmake/paths.hxx"
#include "../ext/cmake/file_api.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE
#include "../spec/project.hxx"

// Configure emits actions that configure projects enough to produce metadata for prep.

namespace mgmake::configure {
	inline context::context(
		configure::result& result,
		const build::request& req,
		const spec::project& project,
		const acquire::result& acquired
	)
		: m_result{result}
		, m_req{req}
		, m_project{project}
		, m_acquired{acquired}
		, m_emit{result.m_graph}
#ifdef MGMK_ENABLE_EXT_CMAKE
		, m_cmake_projects(project.m_cmake_projects.size())
#endif // MGMK_ENABLE_EXT_CMAKE
	{}

#ifdef MGMK_ENABLE_EXT_CMAKE
	inline const configure::cmake::project& context::cmake(ext::cmake::project::id id) {
		mgmkassert(id < m_project.m_cmake_projects.size(), "mgmake configure: invalid CMake project id");

		if (m_cmake_projects.at(id).has_value()) {
			return m_cmake_projects.at(id).value();
		}

		const auto& cmake_project = m_project.m_cmake_projects.at(id);
		m_cmake_projects.at(id) = cmake_value(cmake_project);
		return m_cmake_projects.at(id).value();
	}

	inline configure::cmake::project context::cmake_value(
		const ext::cmake::project& cmake_project
	) {
		mgmkassert(!cmake_project.m_name.empty(), "mgmake configure: CMake project has no name");
		mgmkassert(cmake_project.m_source.has_value(), "mgmake configure: CMake project '" + cmake_project.m_name + "' has no source");

		const auto* fetched = m_acquired.find_fetch(cmake_project.m_source->m_name);

		mgmkassert(
			fetched != nullptr,
			"mgmake configure: source '" + cmake_project.m_source->m_name +
				"' for CMake project '" + cmake_project.m_name + "' was not acquired"
		);

		const auto source_dir = fetched->m_source_dir;
		const auto build_dir = configure::cmake::build_dir(request(), cmake_project.m_name);
		const auto install_dir = configure::cmake::install_dir(request(), cmake_project.m_name);
		const auto query_path = ext::cmake::file_api::query_file(build_dir);
		const auto configure_output = configure::cmake::configure_output(build_dir);

		const auto query_id = m_emit.generated(query_path);
		const auto configure_id = m_emit.generated(configure_output);

		// CMake must see the query file before configure so it writes codemodel replies.
		// This is a plan-time side effect for now; a later native write-file DAG action
		// can make this purely graph-driven.
		mgmkassert(
			ext::cmake::file_api::write_query_file(build_dir),
			"mgmake configure: failed to write CMake File API query for project '" + cmake_project.m_name + "'"
		);

		m_emit.action(
			"Configure CMake project " + cmake_project.m_name,
			"Configures CMake project '" + cmake_project.m_name + "'.",
			{query_id},
			{configure_id},
			configure::cmake::configure_command(
				request(),
				cmake_project,
				source_dir,
				build_dir,
				install_dir
			)
		);

		dag::target dag_target{
			"configure:cmake:" + cmake_project.m_name,
			{configure_id},
			{}
		};
		m_emit.target(dag_target);

		configure::cmake::project result{};
		result.m_name = cmake_project.m_name;
		result.m_source_dir = source_dir;
		result.m_build_dir = build_dir;
		result.m_install_dir = install_dir;
		result.m_usage_root = cmake_project.m_install
			? ext::path_root::install
			: ext::path_root::build;

		m_result.m_cmake_projects.insert_or_assign(cmake_project.m_name, result);
		return result;
	}
#endif // MGMK_ENABLE_EXT_CMAKE
}

#endif // MGMK_CONFIGURE_CONTEXT_IMPL_HXX
