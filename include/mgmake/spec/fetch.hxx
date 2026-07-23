#pragma once

#ifndef MGMAKE_SPEC_FETCH_HXX
#define MGMAKE_SPEC_FETCH_HXX

#include "../meta/builder_mixin.hxx"
#include "../meta/type_builder.hxx"

namespace mgmake::spec {
	enum struct archive_format {
		auto_detect,
		zip,
		tar,
		tar_gz,
		tar_xz
	};

	template<typename storage_t = meta::type_map<>>
	struct git_fetch_impl : public meta::type_builder<git_fetch_impl, storage_t>, public meta::named<git_fetch_impl<storage_t>> {
		using builder_type = meta::type_builder<git_fetch_impl, storage_t>;

		template<meta::static_string url_v>
		[[nodiscard]] static consteval auto url() {
			return builder_type::template set_str<"url", url_v>();
		}
		static consteval auto url() {
			return builder_type::template get_str<"url">();
		}

		template<meta::static_string tag_v>
		[[nodiscard]] static consteval auto tag() {
			return builder_type::template set_str<"tag", tag_v>();
		}
		static consteval auto tag() {
			return builder_type::template get_str<"tag">();
		}
	};

	template<typename storage_t = meta::type_map<>>
	struct archive_fetch_impl : public meta::type_builder<archive_fetch_impl, storage_t>, public meta::named<archive_fetch_impl<storage_t>> {
		using builder_type = meta::type_builder<archive_fetch_impl, storage_t>;

		template<meta::static_string url_v>
		[[nodiscard]] static consteval auto url() {
			return builder_type::template set_str<"url", url_v>();
		}
		static consteval auto url() {
			return builder_type::template get_str<"url">();
		}

		template<meta::static_string checksum_v>
		[[nodiscard]] static consteval auto checksum() {
			return builder_type::template set_str<"checksum", checksum_v>();
		}
		static consteval auto checksum() {
			return builder_type::template get_str<"checksum">();
		}

		template<archive_format format_v>
		[[nodiscard]] static consteval auto format() {
			return builder_type::template set_value<"format", format_v>();
		}
		static consteval archive_format format() {
			if constexpr (builder_type::template has<"format">()) {
				return builder_type::template get_value<"format">();
			} else {
				return archive_format::auto_detect;
			}
		}
	};

	template<typename storage_t = meta::type_map<>>
	struct local_fetch_impl : public meta::type_builder<local_fetch_impl, storage_t>, public meta::named<local_fetch_impl<storage_t>> {
		using builder_type = meta::type_builder<local_fetch_impl, storage_t>;

		template<meta::static_string path_v>
		[[nodiscard]] static consteval auto path() {
			return builder_type::template set_str<"path", path_v>();
		}
		static consteval auto path() {
			return builder_type::template get_str<"path">();
		}
	};

	template<typename storage_t = meta::type_map<>>
	struct fetch_impl : public meta::type_builder<fetch_impl, storage_t>, public meta::named<fetch_impl<storage_t>> {
		[[nodiscard]] static consteval auto git() {
			return git_fetch_impl<>{}.name<fetch_impl{}.name()>();
		}
		[[nodiscard]] static consteval auto archive() {
			return archive_fetch_impl<>{}.name<fetch_impl{}.name()>();
		}
		[[nodiscard]] static consteval auto local() {
			return local_fetch_impl<>{}.name<fetch_impl{}.name()>();
		}
	};
	static constexpr auto fetch = fetch_impl<>{};
}

#endif // MGMAKE_SPEC_FETCH_HXX