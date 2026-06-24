#pragma once

#ifndef MGMAKE_BACKEND_REGISTRY_HXX
#define MGMAKE_BACKEND_REGISTRY_HXX

#include "ninja.hxx"
#include "../cli/backend.hxx"

#include <type_traits>

// Maps a parsed backend enum to its concrete backend type at compile time.
// A void mapping means the backend name is accepted by the CLI but not implemented yet.

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


	template <cli::backend_kind Kind>
	using for_kind_t = typename for_kind<Kind>::type;

	template <cli::backend_kind Kind>
	inline constexpr bool implemented_v =
		!std::is_void_v<for_kind_t<Kind>>;
}

#endif // MGMAKE_BACKEND_REGISTRY_HXX
