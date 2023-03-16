#include "log.h"
#ifdef SK_ENABLE_DEBUG_OUTPUT
    #include <spdlog/sinks/ansicolor_sink.h>
#endif

using namespace spdlog;
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
void Log::init()
{
#ifdef SK_ENABLE_DEBUG_OUTPUT
    using sink_type = sinks::ansicolor_stdout_sink_mt;
    auto core_sink = makeShared<sink_type>();
    core_sink->set_pattern("%^[%n][%s:%#][%!] %v%$");
    core_sink->set_color(level::trace, "\033[37m\033[2m");
    core_sink->set_color(level::debug, "\033[37m\033[2m");
    core_sink->set_color(level::info, "\033[36m");
    core_sink->set_color(level::warn, "\033[33m");
    core_sink->set_color(level::err, "\033[31m");
    core_sink->set_color(level::critical, "\033[1m\033[41m");

    core_logger = makeShared<logger>(ENGINE_NAME, core_sink);
    register_logger(core_logger);
    client_logger = makeShared<logger>("App", core_sink);
    register_logger(client_logger);
#endif
}