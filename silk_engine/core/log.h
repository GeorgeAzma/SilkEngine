#pragma once

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
    #define SK_ASSERT(x, ...) do { if (!(x)) { SPDLOG_LOGGER_CRITICAL(SK_LOGGER, _sk_assert(#x, __VA_ARGS__)); std::abort(); } } while(0)
#else
    #define SK_PROFILE_FUNCTION(...)
    #define SK_TRACE(...)
    #define SK_INFO(...)
    #define SK_WARN(...)
    #define SK_ERROR(...)
    #define SK_CRITICAL(...)
    #define SK_VERIFY(x, ...)
    #define SK_ASSERT(x, ...)
#endif