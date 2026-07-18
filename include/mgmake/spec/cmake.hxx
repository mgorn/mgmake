#pragma once

#ifndef MGMAKE_SPEC_CMAKE_HXX
#define MGMAKE_SPEC_CMAKE_HXX

#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	template<typename storage_t>
	struct cmake_target_impl {};

	template<typename storage_t = meta::type_map<>>
	struct cmake_impl {
		template<typename builder_t = meta::type_builder<>>
		struct cmake_target_builder {
			using builder_type = builder_t;

			MGMAKE_TYPE_BUILDER_VALUE_FIELD(cmake_target_builder, name, meta::static_string);

			using build = typename builder_type::template build<cmake_target_impl>;
		};

		using target = cmake_target_builder<>;
		using library = target;
	};

	template<typename builder_t = meta::type_builder<>>
	struct cmake_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(cmake_builder, name, meta::static_string);
		MGMAKE_TYPE_BUILDER_TYPE_FIELD(cmake_builder, fetch);
		// Sets the map of CMake variables to define
		MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(cmake_builder, set_cmake_vars, "cmake_vars");
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(cmake_builder, set_install, bool);

		template<meta::static_string var_v, meta::static_string val_v>
		using define = set_cmake_vars<typename meta::type_or_t<
			typename builder_t::template get<"cmake_vars", false>,
			meta::type_map<>
		>::template emplace_unique<meta::type_value<var_v>, meta::type_value<val_v>>>;

		using build = typename builder_type::template build<cmake_impl>;
		using install = set_install<true>::build;
	};
	using cmake = cmake_builder<>;
}

#endif // MGMAKE_SPEC_CMAKE_HXX