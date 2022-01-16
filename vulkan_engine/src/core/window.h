#pragma once

#include <GLM/glm.hpp>
#include <GLFW/glfw3.h>
#include <string>

enum class WindowAlignment
{
    NONE,
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
    static void init();
    static void cleanup();
    static unsigned int getWidth() { return data.width; }
    static unsigned int getHeight() { return data.height; }
    static unsigned int getX() { return data.x; }
    static unsigned int getY() { return data.y; }
    static float getAspectRatio() { return (float)data.width / data.height; }
    static GLFWwindow *getGLFWWindow() { return window; }
    static bool isFullscreen() { return data.fullscreen; }
    static bool isVsync() { return vsync; }
    static bool isTransparent() { return transparent; }
    static bool isMinimized() { return data.width == 0 || data.height == 0; }
    static void setVsync(bool vsync);
    static void setFullscreen(bool fullscreen);
    static void setX(int x);
    static void setY(int y);
    static void setPosition(const glm::ivec2 &position);
    static void setWidth(unsigned int width);
    static void setHeight(unsigned int height);
    static void setSize(const glm::uvec2 &size);
    static void setTitle(const char* title);
    static void align(WindowAlignment a = WindowAlignment::CENTER);
    //static void setIcon(std::shared_ptr<Texture> icon);

private:
    static inline GLFWwindow *window = nullptr;

    static inline struct Data
    {
        int width = 1280;
        int height = 720;
        const char *title = "Window";
        int x = 0;
        int y = 0;
        bool fullscreen = false;
    } data;

    static inline bool vsync = false;
    static inline bool transparent = false;
};