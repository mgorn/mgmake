#pragma once

#ifndef MGMAKE_BUILD_CLEAN_HXX
#define MGMAKE_BUILD_CLEAN_HXX

#include "request.hxx"

#include <expected>
#include <filesystem>
#include <string>

namespace mgmake::build {
	[[nodiscard]] inline std::expected<void, std::string> clean(
		const build::request& req
	) {
		std::error_code ec;
		std::filesystem::remove_all(req.build_dir(), ec);

		if (ec) {
			return std::unexpected{
				"mgmake: failed to clean build directory '" +
				req.build_dir().string() +
				"': " +
				ec.message()
			};
		}

		return {};
	}
}

#endif
