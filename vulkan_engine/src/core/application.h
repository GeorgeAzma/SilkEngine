#pragma once
#include "log.h"
#include "window.h"

struct ApplicationCommandLineArgs
{
    int count = 0;
    char **args = nullptr;

    const char *operator[](int index) const
    {
        VE_CORE_ASSERT(index < Count);
        return args[index];
    }
};

class Application
{
public:
    Application(const std::string &name = "App", ApplicationCommandLineArgs args = {});
    virtual ~Application();

    void run();

private:
    bool running = true;
    bool minimized = false;
    std::shared_ptr<Window> window;

private:
    friend int ::main(int argc, char **argv);
};

// To be defined in client
Application *createApp();