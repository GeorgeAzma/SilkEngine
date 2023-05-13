#pragma once

#include "silk_engine/core/application.h"

struct KeyPressEvent;
struct WindowCloseEvent;
struct FramebufferResizeEvent;
class Window;
class Scene;
class Fence;
class Semaphore;
class RenderPass;
class Material;

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
    Fence* previous_frame_finished = nullptr;
    Semaphore* swap_chain_image_available = nullptr;
    Semaphore* render_finished = nullptr;
    std::vector<shared<RenderPass>> render_passes{};
    shared<Material> material = nullptr;
};