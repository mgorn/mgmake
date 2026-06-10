#pragma once

#ifndef MGMK_SPEC_LIBRARY_IMPL_HXX
#define MGMK_SPEC_LIBRARY_IMPL_HXX

#include "library.hxx"
#include "project.hxx"

#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	inline dag::target spec::library::graph(
		dag::graph& result,
		const build::request& req,
		const spec::project& proj,
		std::set<dag::target::id> target_dependencies
	) const {
		(void)result;
		(void)req;
		(void)proj;

		mgmkassert(not m_name.empty(), "mgmake spec: library target has no name");

		mgmkassert(
			m_kind == spec::library::kind::interface,
			std::string{"mgmake spec: lowering non-interface library '"} + m_name + "' is not implemented yet"
		);

		return dag::target{
			m_name,
			{},
			std::move(target_dependencies)
		};
	}
}

#endif
