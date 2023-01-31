#include "log.h"
#ifdef SK_ENABLE_DEBUG_OUTPUT
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
shared<spdlog::logger> Log::core_logger;
shared<spdlog::logger> Log::client_logger;
#endif

void Log::init()
{
#ifdef SK_ENABLE_DEBUG_OUTPUT
    std::vector<spdlog::sink_ptr> log_sinks;
    log_sinks.emplace_back(makeShared<spdlog::sinks::stdout_color_sink_mt>());

    log_sinks[0]->set_pattern("%^[%T] %n: %v%$");

    core_logger = makeShared<spdlog::logger>("SilkEngine", begin(log_sinks), end(log_sinks));
    spdlog::register_logger(core_logger);
    core_logger->set_level(spdlog::level::trace);
    core_logger->flush_on(spdlog::level::trace);

    client_logger = makeShared<spdlog::logger>("App", begin(log_sinks), end(log_sinks));
    spdlog::register_logger(client_logger);
    client_logger->set_level(spdlog::level::trace);
    client_logger->flush_on(spdlog::level::trace);
#endif
}
