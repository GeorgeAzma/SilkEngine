#pragma once

#include "core/entry_point.h"
#include "core/application.h"
#include "scene/scene.h"

class MyApp : public Application
{
public:
    MyApp(ApplicationCommandLineArgs args);
    void onUpdate() override;
    ~MyApp();
    
    std::string getName() const override { return "MyApp"; }

private:
    shared<Scene> scene;
};