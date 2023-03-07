#pragma once

#include "silk_engine/core/application.h"
#include "silk_engine/scene/scene.h"

class MyApp : public Application
{
public:
    MyApp(const ApplicationCommandLineArgs& args);
    void onUpdate() override;
    ~MyApp();

    std::string getName() const override { return "MyApp"; }

private:
    shared<Scene> scene;
};