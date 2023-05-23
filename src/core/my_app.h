#pragma once

#include "silk_engine/core/application.h"

struct KeyPressEvent;
struct WindowCloseEvent;
struct FramebufferResizeEvent;
class Window;
class Scene;

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
    shared<Window> window = nullptr;
    shared<Scene> scene = nullptr;
};