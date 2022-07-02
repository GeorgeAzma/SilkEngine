#pragma once

#include "silk_engine.h"


class SandboxApp : public Application
{
public:
    SandboxApp(ApplicationCommandLineArgs args);
    void onUpdate() override;
    ~SandboxApp();
    
    virtual std::string getName() const override { return "Sandbox"; }

private:
    shared<Scene> scene;
	shared<Entity> camera;
    std::vector<shared<Entity>> entities;
};