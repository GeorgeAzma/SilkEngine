#include <vulkan_engine.h>

class Sandbox : public Application
{
public:
    Sandbox()
    {
    }

    ~Sandbox()
    {
    }
};

Application *createApp()
{
    return new Sandbox();
}