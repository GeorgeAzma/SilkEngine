#pragma once
#include "log.h"
#include "window.h"
#include "layer_stack.h"
#include "../utils/fixed_update.h"

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

    void pushLayer(Layer *layer);
    void pushOverlay(Layer *layer);

    ApplicationCommandLineArgs getCommandLineArgs() const { return commandLineArgs; }

private:
    void run();

private:
    ApplicationCommandLineArgs commandLineArgs;
    bool running = true;
    bool minimized = false;
    std::shared_ptr<Window> window;
    LayerStack layerStack;
    double runtime = 0.0;
    FixedUpdate appUpdate;

private:
    friend int ::main(int argc, char **argv);
};

// To be defined in client
Application *createApp();