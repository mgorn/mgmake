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
	inline void append_target_args(
		sys::command_line& command,
		const toolchain& tc,
		const request&
	) {
		switch (tc.target_selection()) {
			case toolchain::target_mode::implicit:
			case toolchain::target_mode::custom:
				return;

			case toolchain::target_mode::clang_target: {
				const auto& triple = tc.target_triple();

				if (!triple.has_value()) {
					return;
				}

				mgmkassert(
					!triple->empty(),
					"mgmake build: target triple must not be empty"
				);

				command.m_args.emplace_back("-target");
				command.m_args.emplace_back(*triple);
				return;
			}
		}

		mgmkassert(false, "mgmake build: unknown toolchain target mode");
	}
}

#endif // MGMAKE_BUILD_TARGET_HXX
