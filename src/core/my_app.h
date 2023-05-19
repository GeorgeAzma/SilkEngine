#pragma once

#include "silk_engine/core/application.h"

struct KeyPressEvent;
struct WindowCloseEvent;
struct FramebufferResizeEvent;
class Window;
class Scene;
class Material;
class RenderGraph;

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
    unique<Window> window = nullptr;
    shared<Scene> scene = nullptr;
    shared<Material> material = nullptr;
    unique<RenderGraph> render_graph = nullptr;
};