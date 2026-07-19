#pragma once

#ifndef MGMAKE_SPEC_TRAITS_HXX
#define MGMAKE_SPEC_TRAITS_HXX

namespace mgmake::spec {
	template<typename spec_t>
	concept has_links = requires {
		typename spec_t::links;
	};

	template<typename spec_t>
	concept collects_targets = requires {
		typename spec_t::collect_targets;
	};
}

#endif // MGMAKE_SPEC_TRAITS_HXX