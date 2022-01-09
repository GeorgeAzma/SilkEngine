#pragma once

#include "log.h"
#include "window.h"
#include "layer_stack.h"
#include "utils/fixed_update.h"
#include "event.h"

int main(int argc, char **argv);

struct ApplicationCommandLineArgs
{
    int count = 0;
    char **args = nullptr;

    const char *operator[](int index) const
    {
        return args[index];
    }
};

class Application
{
public:
    Application(const char *name = "App", ApplicationCommandLineArgs args = {});
    virtual ~Application();

    void pushLayer(Layer *layer);
    void pushOverlay(Layer *layer);

    ApplicationCommandLineArgs getCommandLineArgs() const { return command_line_args; }

private:
    void run();
    void onWindowClose(const WindowCloseEvent &e);

protected:
    std::shared_ptr<Window> window;
    FixedUpdate app_update;

private:
    ApplicationCommandLineArgs command_line_args;
    bool running = true;
    LayerStack layer_stack;
    double runtime = 0.0;

private:
    friend int ::main(int argc, char **argv);
};

// To be defined in client
Application *createApp();