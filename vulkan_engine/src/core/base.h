#pragma once
#include "platform.h"

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

#define VE_MAKE_VERSION(major, minor, patch)