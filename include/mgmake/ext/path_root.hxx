#pragma once

#ifndef MGMK_EXT_PATH_ROOT_HXX
#define MGMK_EXT_PATH_ROOT_HXX

// Path roots describe how provider-relative include and artifact paths are resolved.

namespace mgmake::ext {
	enum struct path_root {
		usage,
		source,
		build,
		install
	};
}

#endif // MGMK_EXT_PATH_ROOT_HXX
