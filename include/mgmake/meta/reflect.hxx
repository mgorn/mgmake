#pragma once

#ifndef MGMAKE_META_REFLECT_HXX
#define MGMAKE_META_REFLECT_HXX

#if defined(__cpp_impl_reflection) and (__cpp_impl_reflection >= 202506L)
	#if defined(__has_include)
		#if __has_include(<meta>)
			#include <meta>

			#if defined(__cpp_lib_reflection) and (__cpp_lib_reflection >= 202506L)
				#define MGMK_META_HAS_STANDARD_REFLECTION 1
			#endif
		#endif
	#endif
#endif

#ifndef MGMK_META_HAS_STANDARD_REFLECTION
	#define MGMK_META_HAS_STANDARD_REFLECTION 0
#endif

#endif