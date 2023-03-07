#pragma once

#include <memory>
#include <cmath>
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

#include <vulkan/vulkan.h>

constexpr const char* ENGINE_NAME = "SilkEngine";

#ifndef SK_DIST
#define SK_ENABLE_DEBUG_OUTPUT
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

using path = std::filesystem::path;

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

#include "utils/math.h"

namespace std
{
    struct Bytes
    {
        size_t value;
        operator const size_t& () const { return value; }
    };

    struct Seconds
    {
        double value;
        operator double& () { return value; }
        operator const double& () const { return value; }
    };

    template <>
    struct formatter<Bytes> : formatter<double> 
    {
        template <typename FormatContext>
        auto format(Bytes b, FormatContext& ctx)
        {
            constexpr char8_t UNITS[] = { 0, 'K', 'M', 'G', 'T', 'P', 'E', 'Z' };
            constexpr size_t BASE = 1024;

            size_t unit = 0;
            size_t base = BASE;
            for (; b >= base; ++unit, base *= BASE);
            base /= BASE;

            auto out = formatter<double>::format(b / base, ctx);
            if (unit)
                *out++ = UNITS[unit];
            *out++ = 'B';
            return out;
        }
    };

    template <>
    struct formatter<Seconds> : formatter<double>
    {
        template <typename FormatContext>
        auto format(Seconds t, FormatContext& ctx)
        {
            if (t < Seconds(1.0) && t > Seconds(0.0))
            {
                Seconds t2 { Seconds(1.0) / t };
                constexpr size_t BASE = 1000;
                constexpr char8_t UNITS[] = { 'm', 'u', 'n' };

                size_t unit = 0;
                size_t base = BASE;
                for (; t2 >= base && unit < sizeof(UNITS); ++unit, base *= BASE);

                auto out = formatter<double>::format(t * base, ctx);

                *out++ = UNITS[unit];
                *out++ = 's';
                return out;
            }
            else if (t >= Seconds(1.0))
            {
                auto out = formatter<double>::format(t, ctx);
                *out++ = 's';
                return out;
            }
            else
            {
                auto out = formatter<double>::format(0.0, ctx);
                *out++ = 's';
                return out;
            }
        }
    };


    template <length_t L, typename T, qualifier Q>
    struct formatter<vec<L, T, Q>> : formatter<T>
    {
        template <typename FormatContext>
        auto format(const vec<L, T, Q>& vec, FormatContext& ctx)
        {
            auto&& out = ctx.out();
            format_to(out, "{{ {}", vec[0]);
            for (length_t l = 1; l < L; ++l)
                format_to(out, ", {}", vec[l]);
            format_to(out, " }}");
            return out;
        }
    };
    
    template <>
    struct formatter<path> : formatter<double>
    {
        template <typename FormatContext>
        auto format(const path& p, FormatContext& ctx)
        {
            auto&& out = ctx.out();
            format_to(out, "{}", p.string());
            return out;
        }
    };
}

#include "platform.h"
#include "utils/time.h"
#include "utils/RNG.h"
#include "core/log.h"
#include "utils/debug_timer.h"