#pragma once

#ifndef MGMAKE_DETAIL_ASSERT_HXX
#define MGMAKE_DETAIL_ASSERT_HXX

#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string_view>

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

namespace mgmake::detail {

inline void debug_break() {
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__clang__) || defined(__GNUC__)
    __builtin_trap();
#else
    std::abort();
#endif
}

[[noreturn]] inline void assertion_failed(
    std::string_view condition,
    std::string_view message,
    std::source_location location = std::source_location::current()
) {
    std::fprintf(
        stderr,
        "\nmgmake assertion failed\n"
        "  condition: %.*s\n"
        "  message:   %.*s\n"
        "  location:  %s:%u:%u\n"
        "  function:  %s\n\n",
        static_cast<int>(condition.size()),
        condition.data(),
        static_cast<int>(message.size()),
        message.data(),
        location.file_name(),
        location.line(),
        location.column(),
        location.function_name()
    );

    debug_break();

    // In case the platform/debugger allows execution to continue.
    std::abort();
}

inline void mgmk_assert_impl(
    bool condition,
    std::string_view condition_text,
    std::string_view message,
    std::source_location location = std::source_location::current()
) {
    if (!condition) {
        assertion_failed(condition_text, message, location);
    }
}

} // namespace mgmake::detail

#ifndef MGMK_ENABLE_ASSERTS
    #ifndef NDEBUG
        #define MGMK_ENABLE_ASSERTS 1
    #else
        #define MGMK_ENABLE_ASSERTS 0
    #endif
#endif

#if MGMK_ENABLE_ASSERTS
    #define mgmkassert(condition, message)                                      \
        do {                                                                    \
            ::mgmake::detail::mgmk_assert_impl(                                 \
                static_cast<bool>(condition),                                   \
                #condition,                                                     \
                message,                                                        \
                std::source_location::current()                                 \
            );                                                                  \
        } while (false)
#else
    #define mgmkassert(condition, message)                                      \
        do {                                                                    \
            (void)sizeof(condition);                                            \
        } while (false)
#endif

#endif