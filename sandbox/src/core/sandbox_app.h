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
	std::shared_ptr<Scene> scene;
	std::shared_ptr<Entity> camera;

    std::vector<std::shared_ptr<Entity>> circles;
    std::shared_ptr<UniformBuffer> uniform_buffer;
    std::shared_ptr<Image> image;
};