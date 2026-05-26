#pragma once

#ifndef MGMAKE_SYS_PLATFORM_HXX
#define MGMAKE_SYS_PLATFORM_HXX

// Cursed windows shit here
#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>

    #define MGMK_PLATFORM_WINDOWS 1
	#pragma message("Windows is included here. This is probably the source of your pain.")
#elif defined(__unix__) || defined(__APPLE__)
    #define MGMK_PLATFORM_POSIX 1
#else
    #define MGMK_PLATFORM_UNSUPPORTED 1
#endif

namespace mgmake::sys {
	enum struct platform {
		windows,
		posix,
		unsupported
	};
}

#endif