#pragma once

#ifndef MGMAKE_EXT_FETCH_HXX
#define MGMAKE_EXT_FETCH_HXX

#include "../detail/assert.hxx"

#include <filesystem>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// Fetch specs describe how external source trees are acquired before lowering provider-backed targets.

namespace mgmake::ext {
	enum struct archive_format {
		auto_detect,
		zip,
		tar,
		tar_gz,
		tar_xz
	};

	struct git_fetch {
		std::string m_url;
		std::string m_ref;
		bool m_shallow = true;
		bool m_submodules = false;
	};

	struct archive_fetch {
		std::string m_url;
		archive_format m_format = archive_format::auto_detect;
		std::string m_sha256;
		std::filesystem::path m_strip_prefix;
	};

	struct local_fetch {
		std::filesystem::path m_path;
	};

	struct fetch {
		using id = std::vector<fetch>::size_type;

		std::string m_name;
		std::variant<git_fetch, archive_fetch, local_fetch> m_data;

		explicit fetch(std::string_view name)
			: m_name{name}
			, m_data{local_fetch{}} {
			mgmkassert(!m_name.empty(), "mgmake ext: fetch has no name");
		}

		fetch& git(std::string_view url) {
			mgmkassert(!url.empty(), "mgmake ext: git fetch has no URL");
			m_data = git_fetch{.m_url = std::string{url}};
			return *this;
		}

		fetch& archive(std::string_view url) {
			mgmkassert(!url.empty(), "mgmake ext: archive fetch has no URL");
			m_data = archive_fetch{.m_url = std::string{url}};
			return *this;
		}

		fetch& zip(std::string_view url) {
			archive(url);
			std::get<archive_fetch>(m_data).m_format = archive_format::zip;
			return *this;
		}

		fetch& tar(std::string_view url) {
			archive(url);
			std::get<archive_fetch>(m_data).m_format = archive_format::tar;
			return *this;
		}

		fetch& tar_gz(std::string_view url) {
			archive(url);
			std::get<archive_fetch>(m_data).m_format = archive_format::tar_gz;
			return *this;
		}

		fetch& tar_xz(std::string_view url) {
			archive(url);
			std::get<archive_fetch>(m_data).m_format = archive_format::tar_xz;
			return *this;
		}

		fetch& local(const std::filesystem::path& path) {
			mgmkassert(!path.empty(), "mgmake ext: local fetch has no path");
			m_data = local_fetch{.m_path = path};
			return *this;
		}

		fetch& ref(std::string_view value) {
			mgmkassert(std::holds_alternative<git_fetch>(m_data), "mgmake ext: ref() requires a git fetch");
			std::get<git_fetch>(m_data).m_ref = std::string{value};
			return *this;
		}

		fetch& branch(std::string_view value) {
			return ref(value);
		}

		fetch& tag(std::string_view value) {
			return ref(value);
		}

		fetch& commit(std::string_view value) {
			return ref(value);
		}

		fetch& sha256(std::string_view value) {
			mgmkassert(std::holds_alternative<archive_fetch>(m_data), "mgmake ext: sha256() requires an archive fetch");
			std::get<archive_fetch>(m_data).m_sha256 = std::string{value};
			return *this;
		}

		fetch& strip_prefix(const std::filesystem::path& value) {
			mgmkassert(std::holds_alternative<archive_fetch>(m_data), "mgmake ext: strip_prefix() requires an archive fetch");
			std::get<archive_fetch>(m_data).m_strip_prefix = value;
			return *this;
		}

		fetch& submodules(bool enabled = true) {
			mgmkassert(std::holds_alternative<git_fetch>(m_data), "mgmake ext: submodules() requires a git fetch");
			std::get<git_fetch>(m_data).m_submodules = enabled;
			return *this;
		}

		fetch& shallow(bool enabled = true) {
			mgmkassert(std::holds_alternative<git_fetch>(m_data), "mgmake ext: shallow() requires a git fetch");
			std::get<git_fetch>(m_data).m_shallow = enabled;
			return *this;
		}
	};
}

#endif // MGMAKE_EXT_FETCH_HXX
