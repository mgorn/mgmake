#pragma once

#ifndef MGMK_SPEC_LIBRARY_HXX
#define MGMK_SPEC_LIBRARY_HXX

#include "../build/request.hxx"
#include "../dag/graph.hxx"
#include "../dag/target.hxx"
#include "target.hxx"

#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project;

	struct library : public target<library> {
		using id = std::vector<library>::size_type;

		enum struct kind {
			static_lib, // k prefix bc static is a keyword
			shared_lib,
			interface
		} m_kind;

		library(std::string_view name, kind k) : target<library>{ std::string{ name } }, m_kind{k} {}

		dag::target graph(
			dag::graph& result,
			const build::request& req,
			const spec::project& proj,
			std::set<dag::target::id> target_dependencies
		) const;
	};
}

#endif
