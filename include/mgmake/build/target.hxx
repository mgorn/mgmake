#pragma once

#ifndef MGMAKE_BUILD_TARGET_HXX
#define MGMAKE_BUILD_TARGET_HXX

#include "request.hxx"
#include "toolchain.hxx"
#include "../detail/assert.hxx"
#include "../sys/command_line.hxx"
#include "../sys/platform.hxx"

#include <string>

namespace mgmake::build {
	[[nodiscard]] inline std::string effective_target_triple(
		const toolchain& tc,
		const request& req
	) {
		if (tc.target_triple().has_value()) {
			return *tc.target_triple();
		}

		return sys::triple(req.target());
	}

	inline void append_target_args(
		sys::command_line& command,
		const toolchain& tc,
		const request& req
	) {
		switch (tc.target_selection()) {
			case toolchain::target_mode::implicit:
			case toolchain::target_mode::custom:
				return;

			case toolchain::target_mode::clang_target: {
				const auto triple = effective_target_triple(tc, req);

				mgmkassert(
					!triple.empty(),
					"mgmake build: clang target mode requires a non-empty target triple"
				);

				command.m_args.emplace_back("-target");
				command.m_args.emplace_back(triple);
				return;
			}
		}

		mgmkassert(false, "mgmake build: unknown toolchain target mode");
	}
}

#endif
