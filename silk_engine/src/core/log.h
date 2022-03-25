#pragma once

#ifdef SK_ENABLE_DEBUG_OUTPUT
    #include <spdlog/spdlog.h>
    #include <spdlog/fmt/ostr.h>

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