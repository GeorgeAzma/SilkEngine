#pragma once

#include "log.h"
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

    ApplicationCommandLineArgs getCommandLineArgs() const { return command_line_args; }

protected:
    virtual void onUpdate() = 0;

private:
    void run();
    void update();
    void onWindowClose(const WindowCloseEvent &e);
    void onWindowResize(const WindowResizeEvent &e);
    void onKeyPress(const KeyPressEvent &e);

protected:
    FixedUpdate app_update;

private:
    ApplicationCommandLineArgs command_line_args;
    bool running = true;
    double runtime = 0.0;

private:
    friend int ::main(int argc, char **argv);
};