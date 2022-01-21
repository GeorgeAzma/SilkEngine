#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

class Log
{
public:
    static void init();

    static shared<spdlog::logger> &getCoreLogger() { return core_logger; }
    static shared<spdlog::logger> &getClientLogger() { return client_logger; }

private:
    static shared<spdlog::logger> core_logger;
    static shared<spdlog::logger> client_logger;
};

template <typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream &operator<<(OStream &os, const glm::vec<L, T, Q> &vector)
{
    return os << glm::to_string(vector);
}

template <typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream &operator<<(OStream &os, const glm::mat<C, R, T, Q> &matrix)
{
    return os << glm::to_string(matrix);
}

template <typename OStream, typename T, glm::qualifier Q>
inline OStream &operator<<(OStream &os, glm::qua<T, Q> quaternion)
{
    return os << glm::to_string(quaternion);
}

#ifdef SK_ENABLE_DEBUG_OUTPUT
    #ifdef SK_CORE
        #define SK_TRACE(...) Log::getCoreLogger()->trace(__VA_ARGS__)
        #define SK_INFO(...) Log::getCoreLogger()->info(__VA_ARGS__)
        #define SK_WARN(...) Log::getCoreLogger()->warn(__VA_ARGS__)
        #define SK_ERROR(...) do { Log::getCoreLogger()->error(__VA_ARGS__); SK_DEBUG_BREAK(); } while(0)
        #define SK_CRITICAL(...) do{ Log::getCoreLogger()->critical(__VA_ARGS__); SK_DEBUG_BREAK() } while(0)
        #define SK_ASSERT(x, ...)          \
            do                                  \
            {                                   \
                if (!(x))                       \
                {                               \
                    SK_ERROR(__VA_ARGS__); \
                    SK_DEBUG_BREAK();           \
                }                               \
            } while (0)
    #else
        #define SK_TRACE(...) Log::getClientLogger()->trace(__VA_ARGS__)
        #define SK_INFO(...) Log::getClientLogger()->info(__VA_ARGS__)
        #define SK_WARN(...) Log::getClientLogger()->warn(__VA_ARGS__)
        #define SK_ERROR(...) do { Log::getClientLogger()->error(__VA_ARGS__); SK_DEBUG_BREAK(); } while(0)
        #define SK_CRITICAL(...) do { Log::getClientLogger()->critical(__VA_ARGS__); SK_DEBUG_BREAK(); } while(0)
        #define SK_ASSERT(x, ...)          \
            do                             \
            {                              \
                if (!(x))                  \
                {                          \
                    SK_ERROR(__VA_ARGS__); \
                    SK_DEBUG_BREAK();      \
                }                          \
            } while (0)
    #endif
#else
    #ifdef SK_CORE
        #define SK_TRACE(...)
        #define SK_INFO(...)
        #define SK_WARN(...)
        #define SK_ERROR(...)
        #define SK_CRITICAL(...)
        #define SK_ASSERT(x, ...)
    #else
        #define SK_TRACE(...)
        #define SK_INFO(...)
        #define SK_WARN(...)
        #define SK_ERROR(...)
        #define SK_CRITICAL(...)
        #define SK_ASSERT(x, ...)
    #endif
#endif