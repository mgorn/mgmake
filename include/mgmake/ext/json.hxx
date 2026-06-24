#pragma once

#ifndef MGMK_EXT_JSON_HXX
#define MGMK_EXT_JSON_HXX

// JSON support is optional; this header selects a backend when one is available and exposes a stable json alias.

namespace mgmake::ext {
	struct no_json_backend_t;
	struct nlohmann_json_backend_t;

	template <typename backend_t>
	struct json_value;
}

#include "json/common.hxx"

#if defined(MGMK_JSON_BACKEND_HEADER)
	#include MGMK_JSON_BACKEND_HEADER
#elif defined(__cxxmg_urlinclude)
	#urlinclude <https://raw.githubusercontent.com/nlohmann/json/refs/tags/v3.12.0/single_include/nlohmann/json.hpp>
#endif // defined(MGMK_JSON_BACKEND_HEADER)

#if defined(INCLUDE_NLOHMANN_JSON_HPP_) || defined(NLOHMANN_JSON_VERSION_MAJOR)
#include "json/nlohmann.hxx"

namespace mgmake::ext {
	using json_backend_t = nlohmann_json_backend_t;
	using json = json_value<json_backend_t>;

	inline constexpr bool has_json_backend = true;
}
#else
namespace mgmake::ext {
	using json_backend_t = no_json_backend_t;
	using json = json_value<json_backend_t>;

	inline constexpr bool has_json_backend = false;
}
#endif // defined(INCLUDE_NLOHMANN_JSON_HPP_) || defined(NLOHMANN_JSON_VERSION_MAJOR)

#endif // MGMK_EXT_JSON_HXX
