#include "log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

shared<spdlog::logger> Log::core_logger;
shared<spdlog::logger> Log::client_logger;

void Log::init()
{
#ifdef VE_ENABLE_DEBUG_OUTPUT
    std::vector<spdlog::sink_ptr> log_sinks;
    log_sinks.emplace_back(makeShared<spdlog::sinks::stdout_color_sink_mt>());
    log_sinks.emplace_back(makeShared<spdlog::sinks::basic_file_sink_mt>("VulkanEngine.log", true));

    log_sinks[0]->set_pattern("%^[%T] %n: %v%$");
    log_sinks[1]->set_pattern("[%T] [%l] %n: %v");

    core_logger = makeShared<spdlog::logger>("VulkanEngine", begin(log_sinks), end(log_sinks));
    spdlog::register_logger(core_logger);
    core_logger->set_level(spdlog::level::trace);
    core_logger->flush_on(spdlog::level::trace);

    client_logger = makeShared<spdlog::logger>("App", begin(log_sinks), end(log_sinks));
    spdlog::register_logger(client_logger);
    client_logger->set_level(spdlog::level::trace);
    client_logger->flush_on(spdlog::level::trace);
#endif
}
