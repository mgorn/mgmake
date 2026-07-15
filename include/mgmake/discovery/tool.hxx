#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_HXX
#define MGMAKE_DISCOVERY_TOOL_HXX

#include "../meta/static_string.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/type_map.hxx"

namespace mgmake::discovery {
	template<typename storage_t = meta::type_map<>>
	struct tool_impl {

	};

	template<typename builder_t = meta::type_builder<>>
	struct tool_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(tool_builder, logical, meta::static_string);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(tool_builder, role, meta::static_string);

		using build = typename builder_type::template build<tool_impl>;
	};
	using tool = tool_builder<>;

	
}

#endif // MGMAKE_DISCOVERY_TOOL_HXX