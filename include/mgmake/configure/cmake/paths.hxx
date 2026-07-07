#pragma once

#ifndef MGMK_CONFIGURE_CMAKE_PATHS_HXX
#define MGMK_CONFIGURE_CMAKE_PATHS_HXX

#include "../../build/request.hxx"

#include <filesystem>
#include <string>
#include <string_view>

namespace mgmake::configure::cmake {
	[[nodiscard]] inline std::filesystem::path root(const build::request& req) {
		return req.build_dir() / "ext";
	}

	[[nodiscard]] inline std::filesystem::path build_dir(
		const build::request& req,
		std::string_view name
	) {
		return root(req) / "build" / std::string{name};
	}

	[[nodiscard]] inline std::filesystem::path install_dir(
		const build::request& req,
		std::string_view name
	) {
		return root(req) / "install" / std::string{name};
	}

	[[nodiscard]] inline std::filesystem::path configure_output(
		const std::filesystem::path& build_dir
	) {
		return build_dir / "CMakeCache.txt";
	}
}

#endif // MGMK_CONFIGURE_CMAKE_PATHS_HXX
