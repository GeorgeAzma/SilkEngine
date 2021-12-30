#pragma once
#include <spdlog/spdlog.h>

class Log
{
public:
    Log();
    ~Log();

    static void init();

    static std::shared_ptr<spdlog::logger> &getCoreLogger() { return core_logger; }
    static std::shared_ptr<spdlog::logger> &getClientLogger() { return client_logger; }

private:
    static std::shared_ptr<spdlog::logger> core_logger;
    static std::shared_ptr<spdlog::logger> client_logger;
};

#define VE_CORE_TRACE(...) Log::getCoreLogger()->trace(__VA_ARGS__)
#define VE_CORE_INFO(...) Log::getCoreLogger()->info(__VA_ARGS__)
#define VE_CORE_WARN(...) Log::getCoreLogger()->warning(__VA_ARGS__)
#define VE_CORE_FATAL(...) Log::getCoreLogger()->fatal(__VA_ARGS__)