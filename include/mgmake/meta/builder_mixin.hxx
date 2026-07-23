#pragma once

#ifndef MGMAKE_META_BUILDER_MIXIN_HXX
#define MGMAKE_META_BUILDER_MIXIN_HXX

#include "../meta/static_string.hxx"

namespace mgmake::meta {
	template<typename builder_t>
	struct named {
		template<meta::static_string value_v>
		[[nodiscard]] static consteval auto name() {
			return builder_t::template set_str<"name", value_v>();
		}
		static consteval auto name() {
			return builder_t::template get_str<"name">();
		}
	};
}

#endif // MGMAKE_META_BUILDER_MIXIN_HXX
