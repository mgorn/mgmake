#pragma once

#ifndef MGMAKE_DETAIL_CONVERT_HXX
#define MGMAKE_DETAIL_CONVERT_HXX

#include <mgmake/sys/platform.hxx>

namespace mgmake::detail {
	#ifdef MGMK_PLATFORM_WINDOWS
	inline constexpr std::string wide_to_utf8(std::wstring_view text) {
		if (text.empty()) {
			return {};
		}

		if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
			throw std::runtime_error("Wide string is too large to convert to UTF-8");
		}

		int wide_size = static_cast<int>(text.size());

		int utf8_size = WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			text.data(),
			wide_size,
			nullptr,
			0,
			nullptr,
			nullptr
		);

		if (utf8_size <= 0) {
			throw std::runtime_error("Failed to calculate UTF-8 argument size");
		}

		std::string result;
		result.resize(static_cast<std::size_t>(utf8_size));

		int written = WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			text.data(),
			wide_size,
			result.data(),
			utf8_size,
			nullptr,
			nullptr
		);

		if (written <= 0) {
			throw std::runtime_error("Failed to convert command line argument to UTF-8");
		}

		return result;
	}
#endif
}

#endif