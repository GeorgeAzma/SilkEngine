#pragma once

class Application
{
public:
    Application() {}
    virtual ~Application() {}
    virtual void run() {}

private:
};

// To be defined in client
Application *createApp();