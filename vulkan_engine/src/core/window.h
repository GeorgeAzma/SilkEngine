#pragma once

#include <GLM/glm.hpp>
#include <GLFW/glfw3.h>
#include <string>

struct WindowProps
{
    int width = 1280, height = 720;
    const char *title = "Window";
    bool transparent = false;
};

enum class WindowAlignment
{
    CENTER,
    LEFT,
    RIGHT,
    BOTTOM,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    TOP,
    TOP_LEFT,
    TOP_RIGHT,
};

class Window
{
public:
    Window(const WindowProps &props = {});
    ~Window();
    void update();
    unsigned int getWidth() const { return data.width; }
    unsigned int getHeight() const { return data.height; }
    unsigned int getX() const { return data.x; }
    unsigned int getY() const { return data.y; }
    GLFWwindow *getGLFWWindow() { return window; }
    bool isFullscreen() const { return data.fullscreen; }
    bool isVsync() const { return data.vsync; }
    bool isTransparent() const { return transparent; }
    bool isMinimized() const { return data.width == 0 || data.height == 0; }
    const glm::mat4 &getProjection() const { return data.projection; }
    void setVsync(bool vsync);
    void setFullscreen(bool fullscreen);
    void setX(int x);
    void setY(int y);
    void setPosition(const glm::ivec2 &position);
    void setWidth(unsigned int width);
    void setHeight(unsigned int height);
    void setSize(const glm::uvec2 &size);
    void setTitle(const char* title);
    void align(WindowAlignment a = WindowAlignment::CENTER);
    // void setIcon(std::shared_ptr<Texture> icon);
private:
    void updateProjection();

private:
    GLFWwindow *window;

    struct Data
    {
        int width, height;
        const char *title;
        bool fullscreen;
        bool vsync;
        int x, y;
        glm::mat4 projection;

        struct Input
        {
            bool anyButtonDown = false;
            int latestButtonDown = false;

            bool anyKeyDown = false;
            int latestKeyDown = false;

            std::vector<bool> keysDown = std::vector<bool>(GLFW_KEY_LAST + 1, false);
            std::vector<bool> buttonsDown = std::vector<bool>(GLFW_KEY_LAST + 1, false);
        } input;

    } data;

    bool transparent;
};