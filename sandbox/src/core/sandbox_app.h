#pragma once

#include "silk_engine.h"

class SandboxApp : public Application
{
public:
    SandboxApp(ApplicationCommandLineArgs args);
    void onUpdate();
    ~SandboxApp();

private:
	unique<Scene> scene;
	shared<Entity> camera;
    std::vector<shared<Entity>> entities;
};