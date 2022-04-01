#pragma once

#include "platform.h"

constexpr const char* ENGINE_NAME = "SilkEngine";

#ifndef SK_DIST
#define SK_ENABLE_DEBUG_OUTPUT
#endif

#ifdef SK_ENABLE_DEBUG_OUTPUT
#if defined(SK_PLATFORM_WINDOWS)
#define SK_DEBUG_BREAK() __debugbreak()
#elif defined(SK_PLATFORM_LINUX)
#include <signal.h>
#define SK_DEBUG_BREAK() raise(SIGTRAP)
#else
#define SK_DEBUG_BREAK()
//#error "Platform doesn't support debugbreak yet!"
#endif
#else
#define SK_DEBUG_BREAK()
#endif

#define SK_MAKE_VERSION(major, minor, patch) \
    (((uint32_t(major)) << 22)               \
    | ((uint32_t(minor)) << 12)              \
    | (uint32_t(patch)))

class NonMovable
{
public:
    NonMovable() {}
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

class NonCopyable
{
public:
    NonCopyable() {}
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

struct Descriptor
{
    void* ptr;
    size_t size;
    operator void* () { return ptr; }
    operator size_t () { return size; }
    bool operator==(const Descriptor& other) const { return std::memcmp(ptr, other.ptr, size) == 0; }
    void copy(void* dst) const { std::memcpy(dst, ptr, size); }
    void move(void* dst) const { std::memmove(dst, ptr, size); }
    void set(int val) const { std::memset(ptr, val, size); }
};

template<typename T>
using shared = std::shared_ptr<T>;
template<typename T>
using unique = std::unique_ptr<T>;

template<typename T, typename... Args>
static constexpr auto makeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
static constexpr auto makeUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}