#pragma once

#ifndef MGMK_EXT_PROVIDER_KIND_HXX
#define MGMK_EXT_PROVIDER_KIND_HXX

// Provider kinds identify external systems that can supply mgmake targets.

namespace mgmake::ext {
	enum struct provider_kind {
		cmake
	};
}

#endif // MGMK_EXT_PROVIDER_KIND_HXX
