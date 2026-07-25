#pragma once

#ifndef MGMAKE_META_TYPE_NAME_HXX
#define MGMAKE_META_TYPE_NAME_HXX

#include "reflect.hxx"

#include <cstddef>
#include <string_view>

namespace mgmake::meta {
	template<typename type_t>
	consteval std::string_view type_name() {
#if MGMK_META_HAS_STANDARD_REFLECTION
		return std::meta::display_string_of(^^type_t);
#elif defined(__clang__)
		constexpr std::string_view signature = __PRETTY_FUNCTION__;
		constexpr std::string_view prefix = "type_t = ";
		constexpr std::size_t prefix_pos = signature.find(prefix);

		static_assert(prefix_pos != std::string_view::npos, "type_name could not parse Clang's function signature");

		constexpr std::size_t begin = prefix_pos + prefix.size();
		constexpr std::size_t end = signature.rfind(']');

		static_assert(end != std::string_view::npos and begin <= end, "type_name could not parse Clang's function signature");

		return signature.substr(begin, end - begin);
#elif defined(__GNUC__)
		constexpr std::string_view signature = __PRETTY_FUNCTION__;
		constexpr std::string_view prefix = "type_t = ";
		constexpr std::size_t prefix_pos = signature.find(prefix);

		static_assert(prefix_pos != std::string_view::npos, "type_name could not parse GCC's function signature");

		constexpr std::size_t begin = prefix_pos + prefix.size();
		constexpr std::size_t separator = signature.find(';', begin);
		constexpr std::size_t end = separator == std::string_view::npos ? signature.rfind(']') : separator;

		static_assert(end != std::string_view::npos and begin <= end, "type_name could not parse GCC's function signature");

		return signature.substr(begin, end - begin);
#elif defined(_MSC_VER)
		constexpr std::string_view signature = __FUNCSIG__;
		constexpr std::string_view prefix = "type_name<";
		constexpr std::string_view suffix = ">(void)";
		constexpr std::size_t prefix_pos = signature.find(prefix);

		static_assert(prefix_pos != std::string_view::npos, "type_name could not parse MSVC's function signature");

		constexpr std::size_t begin = prefix_pos + prefix.size();
		constexpr std::size_t end = signature.rfind(suffix);

		static_assert(end != std::string_view::npos and begin <= end, "type_name could not parse MSVC's function signature");

		return signature.substr(begin, end - begin);
#else
		static_assert(false, "type_name is unsupported by this compiler");
#endif
	}

	template<typename type_t>
	inline constexpr std::string_view type_name_v = type_name<type_t>();
}

#endif