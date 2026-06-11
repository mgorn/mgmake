#pragma once

#ifndef MGMAKE_SYS_PLATFORM_HXX
#define MGMAKE_SYS_PLATFORM_HXX

// Cursed windows shit here
#if defined(_WIN32) 
    // You can still use MGMake on Windows without windows.h <3
    // with love - Michael
    #ifndef MGMK_NO_WINDOWS
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif

        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif

        #include <windows.h>
	    #pragma message("Windows is included here. This is probably the source of your pain.")
        #define MGMK_INCLUDED_WINDOWS
    #endif

    #define MGMK_PLATFORM_WINDOWS 1
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

	static constexpr platform g_host_platform = [] constexpr {
#ifdef MGMK_PLATFORM_WINDOWS
		return platform::windows;
#elifdef MGMK_PLATFORM_POSIX
		return platform::posix;
#else
		return platform::unsupported;
#endif
	}();
}

#endif