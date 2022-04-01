#pragma once

#include "core/application.h"
#include "core/entry_point.h"
#include "scene/scene.h"
#include "scene/entity.h"


class SandboxApp : public Application
{
public:
    SandboxApp(ApplicationCommandLineArgs args);
    void onUpdate();
    ~SandboxApp();
    
    virtual std::string getName() const override { return "Sandbox"; }

private:
    shared<Scene> scene;
	shared<Entity> camera;
    std::vector<shared<Entity>> entities;
};