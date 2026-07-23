#pragma once

#ifndef MGMAKE_META_BUILDER_MIXIN_HXX
#define MGMAKE_META_BUILDER_MIXIN_HXX

#include "../meta/static_string.hxx"

namespace mgmake::meta {
	struct named {
		template<meta::static_string value_v>
		consteval auto name(this auto const& self) {
			return self.template set_str<"name", value_v>();
		}
		consteval auto name(this auto const& self) {
			return self.template get_str<"name">();
		}
	};
}

#endif // MGMAKE_META_BUILDER_MIXIN_HXX