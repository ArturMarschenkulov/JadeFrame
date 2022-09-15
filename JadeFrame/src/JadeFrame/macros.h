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