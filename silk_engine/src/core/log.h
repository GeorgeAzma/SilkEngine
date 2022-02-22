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
        #define SK_LOGGER Log::getCoreLogger()
    #else
        #define SK_LOGGER Log::getClientLogger()
    #endif

    #define SK_TRACE(...) SK_LOGGER->trace(__VA_ARGS__)
    #define SK_INFO(...) SK_LOGGER->info(__VA_ARGS__)
    #define SK_WARN(...) SK_LOGGER->warn(__VA_ARGS__)
    #define SK_ERROR(...) do { SK_LOGGER->error(__VA_ARGS__); SK_DEBUG_BREAK(); } while(0)
    #define SK_CRITICAL(...) do { SK_LOGGER->critical(__VA_ARGS__); SK_DEBUG_BREAK(); } while(0)
    #define SK_ASSERT(x, ...) do { if (!(x)) { SK_ERROR(__VA_ARGS__); SK_DEBUG_BREAK(); } } while (0)
    #define SK_TODO(...) SK_ERROR(std::string("This part of code wasn't written yet, TODO: ") + ## __VA_ARGS__);
#else
    #define SK_TRACE(...)
    #define SK_INFO(...)
    #define SK_WARN(...)
    #define SK_ERROR(...)
    #define SK_CRITICAL(...)
    #define SK_ASSERT(x, ...)
    #define SK_TODO(...)
#endif