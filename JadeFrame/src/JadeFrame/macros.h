#pragma once
/*
    In this file various useful macros are defined.
*/

#if defined(__clang__)
    #define JF_PRAGMA_PUSH       _Pragma("clang diagnostic push")
    #define JF_PRAGMA_POP        _Pragma("clang diagnostic pop")
    #define JF_PRAGMA_DISABLE(x) _Pragma("clang diagnostic ignored")
#elif defined(__GNUC__)
    #define JF_PRAGMA_PUSH       _Pragma("GCC diagnostic push")
    #define JF_PRAGMA_POP        _Pragma("GCC diagnostic pop")
    #define JF_PRAGMA_DISABLE(x) _Pragma("GCC diagnostic ignored")
#elif defined(_MSC_VER)
    #define JF_PRAGMA_PUSH       _Pragma("warning(push)")
    #define JF_PRAGMA_POP        _Pragma("warning(pop)")
    #define JF_PRAGMA_DISABLE(x) _Pragma("warning(disable : x)")
#else
    #define JF_PRAGMA_PUSH
    #define JF_PRAGMA_POP
    #define JF_PRAGMA_DISABLE(x)
    #error("Unknown compiler")
#endif

// Define compiler macros
#if defined(__clang__)
    #define JF_COMPILER_CLANG
#elif defined(__GNUC__)
    #define JF_COMPILER_GCC
#elif defined(_MSC_VER)
    #define JF_COMPILER_MSVC
#else
    #error("Unknown compiler")
#endif

// Define platform/os macros
#if defined(_WIN32)
    #define JF_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define JF_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define JF_PLATFORM_MACOS
#else
    #error("Unknown platform")
#endif

// Define architecture macros
#if defined(__x86_64__) || defined(_M_X64)
    #define JF_ARCH_X86_64
#elif defined(__i386) || defined(_M_IX86)
    #define JF_ARCH_X86
#else
    #error("Unknown architecture")
#endif

// Define debug/release macros
#if defined(NDEBUG)
    #define JF_RELEASE
#else
    #define JF_DEBUG
#endif

// Define static/dynamic library macros
#if defined(JF_STATIC)
    #define JF_API
#else
    #if defined(JF_BUILD)
        #define JF_API __declspec(dllexport)
    #else
        #define JF_API __declspec(dllimport)
    #endif
#endif