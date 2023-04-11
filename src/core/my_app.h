#pragma once

#include "silk_engine/core/application.h"
#include "silk_engine/scene/scene.h"

struct KeyPressEvent;
struct WindowCloseEvent;
struct FramebufferResizeEvent;

class MyApp : public Application
{
public:
    MyApp();
    ~MyApp();

    void onUpdate() override;

    void onKeyPress(const KeyPressEvent& e);
    void onWindowClose(const WindowCloseEvent& e);
    void onFramebufferResize(const FramebufferResizeEvent& e);

private:
    Window* window = nullptr;
    shared<Scene> scene = nullptr;
};