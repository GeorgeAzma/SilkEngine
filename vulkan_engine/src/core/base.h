#pragma once
#include "platform.h"

//Small frequently used utility functions/classes/defines etc..

#ifndef VE_DIST
#define VE_ENABLE_DEBUG_OUTPUT
#endif

#ifdef VE_ENABLE_DEBUG_OUTPUT
#if defined(VE_PLATFORM_WINDOWS)
#define VE_DEBUG_BREAK() __debugbreak()
#elif defined(VE_PLATFORM_LINUX)
#include <signal.h>
#define VE_DEBUG_BREAK() raise(SIGTRAP)
#else
#define VE_DEBUG_BREAK()
//#error "Platform doesn't support debugbreak yet!"
#endif
#else
#define VE_DEBUG_BREAK()
#endif

#define VE_MAKE_VERSION(major, minor, patch) \
    (((uint32_t(major)) << 22)               \
    | ((uint32_t(minor)) << 12)              \
    | (uint32_t(patch)))

class NonMovable
{
public:
    NonMovable() {}
private:
    NonMovable(NonMovable&&);
    NonMovable& operator=(NonMovable&&);
};

class NonCopyable
{
public:
    NonCopyable() {}
private:
    NonCopyable(const NonCopyable&);
    NonCopyable& operator=(const NonCopyable&);
};