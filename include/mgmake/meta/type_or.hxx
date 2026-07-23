#pragma once

#ifndef MGMAKE_META_TYPE_OR_HXX
#define MGMAKE_META_TYPE_OR_HXX

#include <type_traits>

// If the type is void, return the default type instead

namespace mgmake::meta {
	template<typename type_t, typename default_t, typename none_t = void>
	struct type_or {
		using type = std::conditional_t<std::is_same_v<type_t, none_t>, default_t, type_t>;

		static_assert(not std::is_same_v<default_t, none_t>, "If your default_t is none_t, you don't need `meta::type_or`...");
	};

	template<typename type_t, typename default_t, typename none_t = void>
	using type_or_t = type_or<type_t, default_t, none_t>::type;
}

#endif // MGMAKE_META_TYPE_OR_HXX
