#pragma once

#ifndef MGMAKE_ENTRY_MACRO_HXX
#define MGMAKE_ENTRY_MACRO_HXX

#include "entry.hxx"

// Entry macros generate main or wmain and forward argv into the typed mgmake entry overloads.

namespace mgmk = mgmake;

#if defined(MGMK_PLATFORM_WINDOWS) && defined(MGMK_INCLUDED_WINDOWS)

#define MGMK_DETAIL_ENTRY_0()                                                   \
int wmain(int argc, wchar_t** argv) {                                            \
    auto args = ::mgmk::sys::args_from_wide(argc, argv);                         \
    return ::mgmk::entry<>(args);                                                  \
}

#define MGMK_DETAIL_ENTRY_1(FACTORY_FN)                                      \
int wmain(int argc, wchar_t** argv) {                                            \
    auto args = ::mgmk::sys::args_from_wide(argc, argv);                         \
    return ::mgmk::entry<FACTORY_FN>(args);                                  \
}

#define MGMK_DETAIL_ENTRY_2(FACTORY_FN, TOOLCHAINS_V)                          \
int wmain(int argc, wchar_t** argv) {                                            \
    auto args = ::mgmk::sys::args_from_wide(argc, argv);                         \
    return ::mgmk::entry<FACTORY_FN, TOOLCHAINS_V>(args);                      \
}

#else

#define MGMK_DETAIL_ENTRY_0()                                                   \
int main(int argc, char** argv) {                                                \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv);                         \
    return ::mgmk::entry(args);                                                  \
}

#define MGMK_DETAIL_ENTRY_1(FACTORY_FN)                                      \
int main(int argc, char** argv) {                                                \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv);                         \
    return ::mgmk::entry<FACTORY_FN>(args);                                  \
}

#define MGMK_DETAIL_ENTRY_2(FACTORY_FN, TOOLCHAINS_V)                          \
int main(int argc, char** argv) {                                                \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv);                         \
    return ::mgmk::entry<FACTORY_FN, TOOLCHAINS_V>(args);                      \
}

#endif // defined(MGMK_PLATFORM_WINDOWS) && defined(MGMK_INCLUDED_WINDOWS)

#define MGMK_DETAIL_ENTRY_SELECT(_0, _1, _2, NAME, ...) NAME

#define MGMK_ENTRY(...)                                                         \
    MGMK_DETAIL_ENTRY_SELECT(                                                   \
        _0 __VA_OPT__(,) __VA_ARGS__,                                           \
        MGMK_DETAIL_ENTRY_2,                                                    \
        MGMK_DETAIL_ENTRY_1,                                                    \
        MGMK_DETAIL_ENTRY_0                                                     \
    )(__VA_ARGS__)

#define MGMAKE_BUILD_ENTRY(...) MGMK_ENTRY(__VA_ARGS__)
#define MGMK_BUILD_ENTRY(...) MGMK_ENTRY(__VA_ARGS__)

#endif // MGMAKE_ENTRY_MACRO_HXX
