#include <vulkan_engine.h>

class Sandbox : public Application
{
public:
    Sandbox()
    {
        Log::init();
    }

    ~Sandbox()
    {
    }

    void run()
    {
        VE_WARN("RAN SUCCESFULLY TROLOLO");
    }
};

Application *createApp()
{
    return new Sandbox();
}