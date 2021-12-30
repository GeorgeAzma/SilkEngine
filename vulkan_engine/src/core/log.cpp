#include "log.h"

std::shared_ptr<spdlog::logger> Log::core_logger;
std::shared_ptr<spdlog::logger> Log::client_logger;

Log::Log()
{
}

Log::~Log()
{
}

void Log::init()
{
    spdlog::set_pattern("%^[%T] %n: %v%$");

    core_logger = spdlog::stdout_color_mt("VulkanEngine");
    core_logger->set_level(spdlog::level::trace);

    client_logger = spdlog::stdout_color_mt("App");
    client_logger->set_level(spdlog::level::trace);
}
