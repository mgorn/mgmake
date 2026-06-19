#pragma once

#ifndef MGMK_EXT_PROVIDER_REF_HXX
#define MGMK_EXT_PROVIDER_REF_HXX

#include <filesystem>
#include <string>

namespace mgmake::ext {
	enum struct provider_kind {
		cmake
	};

	enum struct output_root {
		source_dir,
		build_dir,
		install_dir
	};

	struct provider_ref {
		provider_kind m_kind = provider_kind::cmake;
		std::string m_project;
		std::string m_target;
		output_root m_usage_root = output_root::install_dir;
	};

	struct rooted_path {
		output_root m_root = output_root::install_dir;
		std::filesystem::path m_path;
	};
}

#endif
