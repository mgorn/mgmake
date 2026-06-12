#pragma once

#ifndef MGMAKE_BACKEND_REGISTRY_HXX
#define MGMAKE_BACKEND_REGISTRY_HXX

#include "graphviz.hxx"
#include "ninja.hxx"
#include "../cli/backend.hxx"

#include <type_traits>

namespace mgmake::backend {
	template <cli::backend_kind Kind>
	struct for_kind {
		using type = void;
	};

	template <>
	struct for_kind<cli::backend_kind::automatic> {
		using type = backend::ninja;
	};

	template <>
	struct for_kind<cli::backend_kind::ninja> {
		using type = backend::ninja;
	};

	template <>
	struct for_kind<cli::backend_kind::graphviz> {
		using type = backend::graphviz<>;
	};

	template <cli::backend_kind Kind>
	using for_kind_t = typename for_kind<Kind>::type;

	template <cli::backend_kind Kind>
	inline constexpr bool implemented_v =
		!std::is_void_v<for_kind_t<Kind>>;
}

#endif
