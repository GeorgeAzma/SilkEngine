#pragma once

#include "silk_engine.h"
#include "core/entry_point.h"

class SandboxApp : public Application
{
public:
    SandboxApp(ApplicationCommandLineArgs args);
    void onUpdate();
    ~SandboxApp();

private:
	shared<Scene> scene;
	shared<Entity> camera;

    std::vector<shared<Entity>> circles;
    std::vector<shared<Entity>> squares;
};