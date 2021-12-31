#pragma once
#include <spdlog/spdlog.h>

class Log
{
public:
    static void init();

    static std::shared_ptr<spdlog::logger> &getCoreLogger() { return core_logger; }
    static std::shared_ptr<spdlog::logger> &getClientLogger() { return client_logger; }

private:
    static std::shared_ptr<spdlog::logger> core_logger;
    static std::shared_ptr<spdlog::logger> client_logger;
};

#ifndef DIST
#define VE_CORE_TRACE(...) Log::getCoreLogger()->trace(__VA_ARGS__)
#define VE_CORE_INFO(...) Log::getCoreLogger()->info(__VA_ARGS__)
#define VE_CORE_WARN(...) Log::getCoreLogger()->warn(__VA_ARGS__)
#define VE_CORE_FATAL(...) Log::getCoreLogger()->fatal(__VA_ARGS__)

#define VE_TRACE(...) Log::getClientLogger()->trace(__VA_ARGS__)
#define VE_INFO(...) Log::getClientLogger()->info(__VA_ARGS__)
#define VE_WARN(...) Log::getClientLogger()->warn(__VA_ARGS__)
#define VE_FATAL(...) Log::getClientLogger()->fatal(__VA_ARGS__)
#else
#define VE_CORE_TRACE(...)
#define VE_CORE_INFO(...)
#define VE_CORE_WARN(...)
#define VE_CORE_FATAL(...)

#define VE_TRACE(...)
#define VE_INFO(...)
#define VE_WARN(...)
#define VE_FATAL(...)
#endif