#pragma once

#ifndef MGMAKE_BACKEND_CAPABILITIES_HXX
#define MGMAKE_BACKEND_CAPABILITIES_HXX

#include "../build/request.hxx"
#include "../dag/graph.hxx"

#include <concepts>
#include <expected>
#include <string>

namespace mgmake::backend {
	template <typename Backend>
	concept can_generate =
		requires(
			Backend backend,
			const dag::graph& graph,
			const build::request& req
		) {
			backend.generate(graph, req);
		};

	template <typename Backend>
	concept can_build =
		requires(
			Backend backend,
			const dag::graph& graph,
			const build::request& req
		) {
			{ backend.build(graph, req) } -> std::same_as<std::expected<void, std::string>>;
		};
}

#endif
