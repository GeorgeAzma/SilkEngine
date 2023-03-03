#pragma once

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
    Application(ApplicationCommandLineArgs args = {});
    virtual ~Application();

    virtual std::string getName() const { return "App"; }

    ApplicationCommandLineArgs getCommandLineArgs() const { return command_line_args; }

protected:
    virtual void onUpdate() = 0;

private:
    void run();
    void update();
    void onWindowClose(const WindowCloseEvent &e);
    void onWindowResize(const WindowResizeEvent &e);
    void onKeyPress(const KeyPressEvent &e);

private:
    Window* window;
    ApplicationCommandLineArgs command_line_args;
    bool running = true;
    bool fully_initialized = false;

private:
    friend int ::main(int argc, char **argv);
};