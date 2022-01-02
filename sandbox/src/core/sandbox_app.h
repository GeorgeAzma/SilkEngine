#pragma once

#include "vulkan_engine.h"
#include "core/entry_point.h"

class SandboxApp : public Application
{
public:
    SandboxApp(ApplicationCommandLineArgs args);
    ~SandboxApp();
};