#include <vulkan_engine.h>

class SandboxApp : public Application
{
public:
    SandboxApp()
    {
    }

    ~SandboxApp()
    {
    }
};

Application *createApp()
{
    return new SandboxApp();
}