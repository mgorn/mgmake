#pragma once

#ifndef MGMK_EXT_CMAKE_CODEMODEL_HXX
#define MGMK_EXT_CMAKE_CODEMODEL_HXX

#include "target.hxx"

#include <map>
#include <string>
#include <string_view>

// A CMake codemodel stores the target metadata mgmake needs after CMake configuration.

namespace mgmake::ext::cmake {
	struct codemodel {
		std::map<std::string, target> m_targets;

		[[nodiscard]] const target* find_target(std::string_view name) const {
			const auto found = m_targets.find(std::string{name});
			return found == m_targets.end() ? nullptr : &found->second;
		}
	};
}

#endif // MGMK_EXT_CMAKE_CODEMODEL_HXX
