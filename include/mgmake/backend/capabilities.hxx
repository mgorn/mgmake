#pragma once

#ifndef MGMAKE_BACKEND_CAPABILITIES_HXX
#define MGMAKE_BACKEND_CAPABILITIES_HXX

#include "../build/request.hxx"
#include "../cli/options.hxx"
#include "../dag/graph.hxx"
#include "../detail/hashes.hxx"

#include <concepts>
#include <expected>
#include <string>

namespace mgmake::backend {
	template <typename Backend>
	concept can_generate =
		requires(
			Backend backend,
			const cli::options& opts,
			const dag::graph& graph,
			const build::request& req
		) {
			backend.generate(opts, graph, req);
		};

	template <typename Backend>
	concept can_build =
		requires(
			Backend backend,
			const cli::options& opts,
			const dag::graph& graph,
			const build::request& req
		) {
			{ backend.build(opts, graph, req) } -> std::same_as<std::expected<void, std::string>>;
		};

	template <typename Backend>
	concept can_build_with_hashes =
		requires(
			Backend backend,
			const cli::options& opts,
			const dag::graph& graph,
			const build::request& req,
			mgmake::detail::hashes& hashes
		) {
			{ backend.build(opts, graph, req, hashes) } -> std::same_as<std::expected<void, std::string>>;
		};
}

#endif // MGMAKE_BACKEND_CAPABILITIES_HXX
