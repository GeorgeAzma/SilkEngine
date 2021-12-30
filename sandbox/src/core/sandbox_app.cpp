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

    void run()
    {
        std::cout << "ran lol" << std::endl;
    }
};

Application *createApp()
{
    return new Sandbox();
}