#pragma once

#include <memory>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <thread>
#include <functional>
#include <algorithm>
#include <map>
#include <filesystem>
#include <fstream>
#include <optional>
#include <unordered_set>
#include <set>
#include <queue>
#include <sstream>
#include <concepts>
#include <type_traits>
#include <chrono>
#include <format>
#include <mutex>
#include <span>
#include <numeric>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

constexpr const char* ENGINE_NAME = "Silk";

#ifndef SK_DIST
#define SK_ENABLE_DEBUG_OUTPUT
#endif

#define SK_ENABLE_DEBUG_MESSENGER (VK_EXT_debug_utils && defined(SK_ENABLE_DEBUG_OUTPUT))

#define SK_MAKE_VERSION(major, minor, patch) \
    (((uint32_t(major)) << 22)               \
    | ((uint32_t(minor)) << 12)              \
    | (uint32_t(patch)))

template <typename Enum> requires std::is_enum_v<Enum>
static constexpr std::underlying_type_t<Enum> ecast(Enum e)
{
    return static_cast<std::underlying_type_t<Enum>>(e);
}

#define ADD_ENUM_CLASS_OPERATORS(Enum)\
static constexpr Enum operator|(Enum lhs, Enum rhs) { return Enum(ecast(lhs) | ecast(rhs)); }\
static constexpr Enum& operator|=(Enum& lhs, Enum rhs) { return (lhs = lhs | rhs); }\
static constexpr Enum operator&(Enum lhs, Enum rhs) { return Enum(ecast(lhs) & ecast(rhs)); }\
static constexpr Enum& operator&=(Enum& lhs, Enum rhs) { return (lhs = lhs & rhs); }\
static constexpr Enum operator*(std::underlying_type_t<Enum> lhs, Enum rhs) { return Enum(lhs * ecast(rhs)); }

struct NoMove
{
    NoMove() {}
    NoMove(NoMove&&) = delete;
    NoMove& operator=(NoMove&&) = delete;
};

struct NoCopy
{
    NoCopy() {}
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

struct NoCopyNoMove : NoCopy, NoMove {};

using namespace std::chrono_literals;

template<typename T>
using shared = std::shared_ptr<T>;
template<typename T>
using unique = std::unique_ptr<T>;
using ulonglong = unsigned long long;
using ulong = unsigned long;
using uint = unsigned int;
using ushort = unsigned short;
using uchar = unsigned char;

#define scast static_cast
#define ccast const_cast
#define dcast dynamic_cast
#define rcast reinterpret_cast

namespace fs
{
    using namespace std::filesystem;
}

template <typename T>
concept IsContainer = requires (T t, size_t n)
{
    typename T::value_type;
    { t.data() } -> std::same_as<typename T::value_type*>;
    { t.resize(n) };
};

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

template <typename... T>
static constexpr auto makeArray(T&&... values) -> std::array<typename std::decay<typename std::common_type<T...>::type>::type, sizeof...(T)>
{
    return std::array<typename std::decay<typename std::common_type<T...>::type>::type, sizeof...(T)>{std::forward<T>(values)...};
}

#include "silk_engine/utils/math.h"
#include "silk_engine/core/platform.h"
#include "silk_engine/core/log.h"
#include "silk_engine/utils/time.h"
#include "silk_engine/utils/random.h"
#include "silk_engine/gfx/enums.h"