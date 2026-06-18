#pragma once

#ifndef MGMAKE_BACKEND_EXECUTE_HXX
#define MGMAKE_BACKEND_EXECUTE_HXX

#include "capabilities.hxx"
#include "registry.hxx"
#include "../build/request.hxx"
#include "../cli/action.hxx"
#include "../cli/backend.hxx"
#include "../cli/options.hxx"
#include "../dag/graph.hxx"

#include <expected>
#include <string>
#include <type_traits>

namespace mgmake::backend {
	template <cli::backend_kind Kind>
	[[nodiscard]] inline std::expected<void, std::string> generate(
		const cli::options& opts,
		const dag::graph& graph,
		const build::request& req
	) {
		using backend_type = backend::for_kind_t<Kind>;

		if constexpr (std::is_void_v<backend_type>) {
			return std::unexpected{
				"mgmake: backend '" +
				std::string{ cli::backend_name(Kind) } +
				"' is not implemented yet"
			};
		} else if constexpr (backend::can_generate<backend_type>) {
			backend_type{}.generate(opts, graph, req);
			return {};
		} else {
			return std::unexpected{
				"mgmake: backend '" +
				std::string{ cli::backend_name(Kind) } +
				"' does not support action 'generate'"
			};
		}
	}

	template <cli::backend_kind Kind>
	[[nodiscard]] inline std::expected<void, std::string> build(
		const cli::options& opts,
		const dag::graph& graph,
		const build::request& req
	) {
		using backend_type = backend::for_kind_t<Kind>;

		if constexpr (std::is_void_v<backend_type>) {
			return std::unexpected{
				"mgmake: backend '" +
				std::string{ cli::backend_name(Kind) } +
				"' is not implemented yet"
			};
		} else if constexpr (backend::can_build<backend_type>) {
			return backend_type{}.build(opts, graph, req);
		} else {
			return std::unexpected{
				"mgmake: backend '" +
				std::string{ cli::backend_name(Kind) } +
				"' does not support action 'build'"
			};
		}
	}

	[[nodiscard]] inline std::expected<void, std::string> build_selected_backend(
		const cli::options& opts,
		const dag::graph& graph,
		const build::request& req
	) {
		switch (opts.m_backend) {
			case cli::backend_kind::automatic:
				return backend::build<cli::backend_kind::automatic>(opts, graph, req);

			case cli::backend_kind::ninja:
				return backend::build<cli::backend_kind::ninja>(opts, graph, req);

			case cli::backend_kind::graphviz:
				return backend::build<cli::backend_kind::graphviz>(opts, graph, req);

			case cli::backend_kind::make:
				return backend::build<cli::backend_kind::make>(opts, graph, req);

			case cli::backend_kind::direct:
				return backend::build<cli::backend_kind::direct>(opts, graph, req);

			case cli::backend_kind::count:
				break;
		}

		return std::unexpected{ "mgmake: unknown backend" };
	}

	template <cli::backend_kind Kind>
	[[nodiscard]] inline std::expected<void, std::string> execute_project_action_for_backend(
		const cli::options& opts,
		const build::request& req,
		const dag::graph& graph
	) {
		switch (opts.m_action) {
			case cli::action_kind::generate:
				return backend::generate<Kind>(opts, graph, req);

			case cli::action_kind::build:
				return backend::build<Kind>(opts, graph, req);

			case cli::action_kind::run:
				return std::unexpected{
					"mgmake: internal error: run must be handled by the entry point"
				};

			case cli::action_kind::clean:
			case cli::action_kind::tools:
			case cli::action_kind::help:
			case cli::action_kind::version:
				return {};

			case cli::action_kind::count:
				break;
		}

		return std::unexpected{ "mgmake: unknown action" };
	}

	[[nodiscard]] inline std::expected<void, std::string> execute_project_action(
		const cli::options& opts,
		const build::request& req,
		const dag::graph& graph
	) {
		switch (opts.m_backend) {
			case cli::backend_kind::automatic:
				return execute_project_action_for_backend<
					cli::backend_kind::automatic
				>(opts, req, graph);

			case cli::backend_kind::ninja:
				return execute_project_action_for_backend<
					cli::backend_kind::ninja
				>(opts, req, graph);

			case cli::backend_kind::graphviz:
				return execute_project_action_for_backend<
					cli::backend_kind::graphviz
				>(opts, req, graph);

			case cli::backend_kind::make:
				return execute_project_action_for_backend<
					cli::backend_kind::make
				>(opts, req, graph);

			case cli::backend_kind::direct:
				return execute_project_action_for_backend<
					cli::backend_kind::direct
				>(opts, req, graph);

			case cli::backend_kind::count:
				break;
		}

		return std::unexpected{ "mgmake: unknown backend" };
	}
}

#endif
