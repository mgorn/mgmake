#pragma once

#ifndef MGMAKE_SPEC_FETCH_HXX
#define MGMAKE_SPEC_FETCH_HXX

#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	enum struct fetch_type {
		git,
		archive,
		local
	};

	enum struct archive_format {
		auto_detect,
		zip,
		tar,
		tar_gz,
		tar_xz
	};

	template<typename storage_t = meta::type_map<>>
	struct git_fetch_impl {
		using storage_type = storage_t;

		
	};
	template<typename storage_t = meta::type_map<>>
	struct archive_fetch_impl {};
	template<typename storage_t = meta::type_map<>>
	struct local_fetch_impl {};

	template<typename builder_t = meta::type_builder<>>
	struct git_fetch_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(git_fetch_builder, name, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(git_fetch_builder, url, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(git_fetch_builder, tag, meta::static_string);

		using build = typename builder_type::template build<git_fetch_impl>;
	};

	template<typename builder_t = meta::type_builder<>>
	struct archive_fetch_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(archive_fetch_builder, name, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(archive_fetch_builder, url, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(archive_fetch_builder, checksum, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(archive_fetch_builder, format, meta::static_string);

		using build = typename builder_type::template build<archive_fetch_impl>;
	};

	template<typename builder_t = meta::type_builder<>>
	struct local_fetch_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(local_fetch_builder, name, meta::static_string);
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(local_fetch_builder, path, meta::static_string);

		using build = typename builder_type::template build<local_fetch_impl>;
	};

	template<typename builder_t = meta::type_builder<>>
	struct fetch_builder {
		using builder_type = builder_t;

		MGMAKE_TYPE_BUILDER_VALUE_FIELD(fetch_builder, name, meta::static_string);

		using git = git_fetch_builder<builder_type>;
		using archive = archive_fetch_builder<builder_type>;
		using local = local_fetch_builder<builder_type>;
	};
	using fetch = fetch_builder<>;
}

#endif // MGMAKE_SPEC_FETCH_HXX