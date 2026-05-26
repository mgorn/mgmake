#pragma once

#ifndef MGMAKE_CLI_BACKEND_HXX
#define MGMAKE_CLI_BACKEND_HXX

#include <string_view>

namespace mgmake::cli {
	enum struct backend_kind {
		automatic,
		ninja,
		make,
		direct
	};

	[[nodiscard]] inline constexpr std::string_view backend_name(backend_kind backend) {
		switch (backend) {
			case backend_kind::automatic:
				return "automatic";
			case backend_kind::ninja:
				return "ninja";
			case backend_kind::make:
				return "make";
			case backend_kind::direct:
				return "direct";
		}

		return "unknown";
	}

	[[nodiscard]] inline constexpr bool parse_backend(std::string_view text, backend_kind& out) {
		if (text == "auto" || text == "automatic") {
			out = backend_kind::automatic;
			return true;
		}

		if (text == "ninja") {
			out = backend_kind::ninja;
			return true;
		}

		if (text == "make" || text == "makefile" || text == "makefiles") {
			out = backend_kind::make;
			return true;
		}

		if (text == "direct" || text == "compiler") {
			out = backend_kind::direct;
			return true;
		}

		return false;
	}
}

#endif