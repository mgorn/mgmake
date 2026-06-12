#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX

#include "target.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct executable : public target<executable> {
		using id = std::vector<executable>::size_type;

		inline constexpr executable(std::string_view name)
			: target<executable>{ std::string{ name } } {
			mgmkassert(not m_name.empty(), "mgmake spec: executable target has no name");
		}
	};
}

#endif
