#pragma once

#ifndef MGMAKE_SPEC_TOOLCHAIN_HXX
#define MGMAKE_SPEC_TOOLCHAIN_HXX

namespace mgmake::spec {
	template<typename storage_t = meta::type_map<>>
	struct toolchain_impl {

	};

	template<typename builder_t = meta::type_builder<>>
	struct toolchain_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(toolchain_builder, name, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(toolchain_builder, cc, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(toolchain_builder, cxx, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(toolchain_builder, ar, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(toolchain_builder, linker, meta::static_string);

		using build = typename builder_type::template build<toolchain_impl>;
	};
	using toolchain = toolchain_builder<>;
}

#endif // MGMAKE_SPEC_TOOLCHAIN_HXX