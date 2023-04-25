#pragma once

int Main(int argc, char** argv);

struct CommandLineArgs
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
    Application();

    static CommandLineArgs getCommandLineArgs() { return command_line_args; }

protected:

    void update();
    void stop() { running = false; }

    virtual void onUpdate() = 0;

private:
    friend int Main(int, char**);
    void run();
    static inline CommandLineArgs command_line_args{};

protected:
    bool running = true;
};

Application* createApplication(int argc, char** argv);