#pragma once

#include "vulkan_engine.h"
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

    std::vector<Entity> circles;
    std::vector<Entity> squares;
    shared<MaterialData> material_data;
    shared<Image> image; 
};