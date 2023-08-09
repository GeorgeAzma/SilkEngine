#pragma once

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

    template <typename CharT>
    struct formatter<Bytes, CharT> : formatter<double, CharT>
    {
        template <typename FormatContext>
        auto format(Bytes b, FormatContext& ctx) const
        {
            constexpr char8_t UNITS[] = { 0, 'K', 'M', 'G', 'T', 'P', 'E', 'Z' };
            constexpr size_t BASE = 1024;

            size_t unit = 0;
            size_t base = BASE;
            for (; b >= base && unit < sizeof(UNITS); ++unit, base *= BASE);
            base /= BASE;

            auto&& out = ctx.out();
            format_to(out, "{:.3g} ", double(b) / base);
            if (unit)
                *out++ = UNITS[unit];
            *out++ = 'B';
            return out;
        }
    };

    template <typename CharT>
    struct formatter<Seconds, CharT> : formatter<double, CharT>
    {
        template <typename FormatContext>
        auto format(Seconds t, FormatContext& ctx) const
        {
            if (t < Seconds(1.0) && t > Seconds(0.0))
            {
                Seconds t2{ Seconds(1.0) / t };
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


    template <length_t L, typename T, qualifier Q, typename CharT>
    struct formatter<vec<L, T, Q>, CharT> : formatter<T, CharT>
    {
        template <typename FormatContext>
        auto format(const vec<L, T, Q>& vec, FormatContext& ctx) const
        {
            auto&& out = ctx.out();
            format_to(out, "{{ {}", vec[0]);
            for (length_t l = 1; l < L; ++l)
                format_to(out, ", {}", vec[l]);
            format_to(out, " }}");
            return out;
        }
    };

    template <typename CharT>
    struct formatter<fs::path, CharT> : formatter<string, CharT>
    {
        template <typename FormatContext>
        auto format(const fs::path& p, FormatContext& ctx) const
        {
            return formatter<string>::format(p.string(), ctx);
        }
    };
}

#ifdef SK_ENABLE_DEBUG_OUTPUT
    #define SPDLOG_USE_STD_FORMAT
    #define SPDLOG_ACTIVE_LEVEL 0
    #include <spdlog/spdlog.h>
#endif

class Log
{
public:
    static void init();

#ifdef SK_ENABLE_DEBUG_OUTPUT
    static shared<spdlog::logger>& getCoreLogger() { return core_logger; }
    static shared<spdlog::logger>& getClientLogger() { return client_logger; }

private:
    static inline shared<spdlog::logger> core_logger = nullptr;
    static inline shared<spdlog::logger> client_logger = nullptr;
#endif
};

#ifdef SK_ENABLE_DEBUG_OUTPUT
    #ifdef SK_CORE
        #define SK_LOGGER Log::getCoreLogger()
    #else
        #define SK_LOGGER Log::getClientLogger()
    #endif

    template<typename... Args>
    static std::string _sk_assert(const char* assert_expr, std::string_view msg, Args&&... args)
    {
        return std::format("Assertion Failed: {} && {}", assert_expr, std::vformat(msg, std::make_format_args(args...)));
    }

    template<typename T>
    static std::string _sk_assert(const char* assert_expr, const T& t)
    {
        return _sk_assert(assert_expr, "{}", t);
    }

    static std::string _sk_assert(const char* assert_expr)
    {
        return _sk_assert(assert_expr, "");
    }

    #define SK_PROFILE_FUNCTION(...) DebugTimer __timer(__FUNCTION__)
    #define SK_TRACE(...) SPDLOG_LOGGER_TRACE(SK_LOGGER, __VA_ARGS__)
    #define SK_INFO(...) SPDLOG_LOGGER_INFO(SK_LOGGER, __VA_ARGS__)
    #define SK_WARN(...) SPDLOG_LOGGER_WARN(SK_LOGGER, __VA_ARGS__)
    #define SK_ERROR(...) SPDLOG_LOGGER_ERROR(SK_LOGGER, __VA_ARGS__)
    #define SK_CRITICAL(...) do { SPDLOG_LOGGER_CRITICAL(SK_LOGGER, __VA_ARGS__); std::abort(); } while(0)
    #define SK_VERIFY(x, ...) do { if (!(x)) { SPDLOG_LOGGER_ERROR(SK_LOGGER, _sk_assert(#x, __VA_ARGS__)); } } while(0)
    #define SK_VERIFY_WARN(x, ...) do { if (!(x)) { SPDLOG_LOGGER_WARN(SK_LOGGER, _sk_assert(#x, __VA_ARGS__)); } } while(0)
    #define SK_ASSERT(x, ...) do { if (!(x)) { SPDLOG_LOGGER_CRITICAL(SK_LOGGER, _sk_assert(#x, __VA_ARGS__)); std::abort(); } } while(0)
#else
    #define SK_PROFILE_FUNCTION(...)
    #define SK_TRACE(...)
    #define SK_INFO(...)
    #define SK_WARN(...)
    #define SK_ERROR(...)
    #define SK_CRITICAL(...)
    #define SK_VERIFY(x, ...)
    #define SK_VERIFY_WARN(x, ...)
    #define SK_ASSERT(x, ...)
#endif