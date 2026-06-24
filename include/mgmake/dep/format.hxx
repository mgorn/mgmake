#pragma once

#ifndef MGMAKE_DEP_FORMAT_HXX
#define MGMAKE_DEP_FORMAT_HXX

#include "../detail/enum_flag.hxx"

#include <cstdint>

// Dependency formats are flags because a compiler action may expose more than one dependency mechanism.

namespace mgmake::dep {
	enum struct format_bits : std::uint8_t {
		gcc,
		msvc_source_dependencies,
		msvc_show_includes,
		count
	};

	using format_flags = detail::enum_flag<format_bits>;

	namespace format {
		inline constexpr format_flags none{};
		inline constexpr format_flags gcc{format_bits::gcc};

		inline constexpr format_flags msvc_source_dependencies{
			format_bits::msvc_source_dependencies
		};

		inline constexpr format_flags msvc_show_includes{
			format_bits::msvc_show_includes
		};

		inline constexpr format_flags msvc_all{
			format_bits::msvc_source_dependencies,
			format_bits::msvc_show_includes
		};
	}
}

#endif // MGMAKE_DEP_FORMAT_HXX
