#include "sandbox_app.h"
#include "sandbox.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    pushLayer(new Sandbox());
}

SandboxApp::~SandboxApp()
{
}



//CREATE APP
Application *createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
