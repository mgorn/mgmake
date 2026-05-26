#pragma once

#ifndef MGMAKE_DETAIL_POOF_HXX
#define MGMAKE_DETAIL_POOF_HXX

namespace mgmake::detail {
	// Poof! You have a T
	template<typename T>
	constexpr decltype(auto) poof() {
		constexpr T value;
		return value;
	}
}

#endif