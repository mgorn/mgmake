#pragma once

#ifndef MGMAKE_DETAIL_PROJECT_FACTORY_HXX
#define MGMAKE_DETAIL_PROJECT_FACTORY_HXX

#include "../cli/options.hxx"
#include "../spec/project.hxx"

#include <functional>
#include <type_traits>
#include <utility>

namespace mgmake::detail {
	template <typename>
	inline constexpr bool always_false_v = false;

	template <typename ProjectFactory>
	[[nodiscard]] inline constexpr spec::project make_project(
		ProjectFactory&& factory,
		const cli::options& opts
	) {
		if constexpr (std::is_invocable_r_v<
			spec::project,
			ProjectFactory&&,
			const cli::options&
		>) {
			return std::invoke(
				std::forward<ProjectFactory>(factory),
				opts
			);
		} else if constexpr (std::is_invocable_r_v<
			spec::project,
			ProjectFactory&&
		>) {
			return std::invoke(std::forward<ProjectFactory>(factory));
		} else {
			static_assert(
				always_false_v<ProjectFactory>,
				"mgmake entry: project factory must return mgmake::spec::project"
			);
		}
	}
}

#endif // MGMAKE_DETAIL_PROJECT_FACTORY_HXX
