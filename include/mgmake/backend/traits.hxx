#pragma once

#ifndef MGMAKE_BACKEND_TRAITS_HXX
#define MGMAKE_BACKEND_TRAITS_HXX

#include "../build/request.hxx"
#include "../dag/graph.hxx"

#include <expected>
#include <string>

namespace mgmake::backend {
	trait generate_backend {
		void generate(const dag::graph& graph, const build::request& req) const;
	};

	trait build_backend {
		std::expected<void, std::string> build(
			const dag::graph& graph,
			const build::request& req
		) const;
	};
}

#endif
