#pragma once

#ifndef MGMK_SPEC_LIBRARY_HXX
#define MGMK_SPEC_LIBRARY_HXX

#include "../dag/graph.hxx"
#include "../dag/target.hxx"
#include "target.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct library : public target<library> {
		using id = std::vector<library>::size_type;

		enum struct kind {
			kstatic, // k prefix bc static is a keyword
			dynamic,
			interface
		} m_kind;

		library(std::string_view name, kind k) : target<library>{ std::string{ name } }, m_kind{k} {}
	};
}

#endif