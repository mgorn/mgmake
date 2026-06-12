#pragma once

#ifndef MGMK_SPEC_LIBRARY_HXX
#define MGMK_SPEC_LIBRARY_HXX

#include "target.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project;

	struct library : public target<library> {
		using id = std::vector<library>::size_type;

		enum struct kind {
			static_lib, // k prefix bc static is a keyword
			shared_lib,
			interface
		} m_kind;

		inline constexpr library(std::string_view name, kind k)
			: target<library>{ std::string{ name } }, m_kind{k} {
			mgmkassert(not m_name.empty(), "mgmake spec: library target has no name");
			mgmkassert(
				m_kind == kind::static_lib ||
					m_kind == kind::shared_lib ||
					m_kind == kind::interface,
				"mgmake spec: invalid library kind"
			);
		}

		inline constexpr auto& add_source(const std::filesystem::path& file) {
			mgmkassert(
				m_kind != kind::interface,
				"mgmake spec: interface library '" + m_name + "' cannot have sources"
			);

			target<library>::add_source(file);
			return *this;
		}
	};
}

#endif
