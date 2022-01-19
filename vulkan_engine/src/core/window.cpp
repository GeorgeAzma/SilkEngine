#include "window.h"
#include "keys.h"
#include "event.h"

static bool GLFW_initialized = false;

static void GLFWErrorCallback(int error, const char *description)
{
    VE_CORE_ERROR("GLFW({0}): {1}", error, description);
}

void Window::init()
{
    if (!GLFW_initialized)
    {
        int success = glfwInit();
        VE_CORE_ASSERT(success, "GLFW: Couldn't initialize glfw");
        glfwSetErrorCallback(GLFWErrorCallback);
        GLFW_initialized = true;
    }

    if (transparent)
    {
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(data.width, data.height, data.title, data.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    glfwDefaultWindowHints();
    glfwShowWindow(window);

    glfwSetWindowUserPointer(window, &data);
    setVsync(vsync);
    align(WindowAlignment::CENTER);

    Dispatcher::post(WindowResizeEvent(data.width, data.height));
    Dispatcher::post(WindowMoveEvent(data.x, data.y));

    // Event callbacks
    glfwSetWindowSizeCallback(window,
        [](GLFWwindow *window, int width, int height)
        {
            Data &data = *(Data *)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;
            if(width != 0 && height != 0)
                Dispatcher::post(WindowResizeEvent(width, height));
        });
    glfwSetWindowPosCallback(window,
        [](GLFWwindow* window, int x, int y)
        {
            Data& data = *(Data*)glfwGetWindowUserPointer(window);
            if (!data.fullscreen)
            {
                data.x = x;
                data.y = y;
            }
            Dispatcher::post(WindowMoveEvent(x, y));
        });

    glfwSetWindowCloseCallback(window,
        [](GLFWwindow *window)
        {
            Dispatcher::post(WindowCloseEvent());
        });

    glfwSetKeyCallback(window,
        [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            switch (action)
            {
            case GLFW_PRESS:
            {
                Dispatcher::post(KeyPressEvent(key, 0));
                break;
            }
            case GLFW_REPEAT:
            {
                Dispatcher::post(KeyPressEvent(key, 1));
                break;
            }
            case GLFW_RELEASE:
            {
                Dispatcher::post(KeyReleaseEvent(key));
            }
            }
        });

    glfwSetMouseButtonCallback(window,
        [](GLFWwindow *window, int button, int action, int mods)
        {
            switch (action)
            {
            case GLFW_PRESS:
            {
                Dispatcher::post(MousePressEvent(button));
                break;
            }
            case GLFW_RELEASE:
            {
                Dispatcher::post(MouseReleaseEvent(button));
                break;
            }
            }
        });

    glfwSetCursorPosCallback(window,
        [](GLFWwindow *window, double mx, double my)
        {
            Dispatcher::post(MouseMoveEvent(mx, my));
        });

    glfwSetScrollCallback(window,
        [](GLFWwindow *window, double sx, double sy)
        {
            Dispatcher::post(MouseScrollEvent(sx, sy));
        });

    glfwSetDropCallback(window,
        [](GLFWwindow* window, int path_count, const char* paths[])
        {
            std::vector<const char*> paths_vec(path_count);
            for (size_t i = 0; i < paths_vec.size(); ++i)
                paths_vec[i] = paths[i];
            Dispatcher::post(DragAndDropEvent(paths_vec));
        });

    glfwSetCharModsCallback(window,
        [](GLFWwindow* window, unsigned int codepoint, int mods)
        {
            Dispatcher::post(CharacterWriteEvent(codepoint));
        });
}

void Window::cleanup()
{
    glfwDestroyWindow(window);
}

void Window::setVsync(bool vsync)
{
    if (Window::vsync == vsync)
        return;
    Window::vsync = vsync;
    glfwSwapInterval((int)vsync);
}

void Window::setFullscreen(bool fullscreen)
{
    if (data.fullscreen == fullscreen)
        return;
    data.fullscreen = fullscreen;
    glfwSetWindowMonitor(window, fullscreen ? glfwGetPrimaryMonitor() : nullptr, data.x, data.y, data.width, data.height, GLFW_DONT_CARE);
    Dispatcher::post(WindowFullscreenEvent(fullscreen));
}

void Window::setX(int x)
{
    if (x == data.x)
        return;
    data.x = x;
    glfwSetWindowPos(window, data.x, data.y);
}

void Window::setY(int y)
{
    if (y == data.y)
        return;
    data.y = y;
    glfwSetWindowPos(window, data.x, data.y);
}

void Window::setPosition(const glm::ivec2 &position)
{
    if (position.x == data.x && position.y == data.y)
        return;
    data.x = position.x;
    data.y = position.y;
    glfwSetWindowPos(window, data.x, data.y);
}

void Window::setWidth(unsigned int width)
{
    if (width == data.width)
        return;
    data.width = width;
    align();
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setHeight(unsigned int height)
{
    if (height == data.height)
        return;
    data.height = height;
    align();
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setSize(const glm::uvec2 &size)
{
    if (size.x == data.width && size.y == data.height)
        return;
    data.width = size.x;
    data.height = size.y;
    align();
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setTitle(const const char *title)
{
    if (title == data.title)
        return;
    data.title = title;
    glfwSetWindowTitle(window, data.title);
}

void Window::align(WindowAlignment a)
{
    if (a == WindowAlignment::NONE) 
        return;

    auto &x = data.x;
    auto &y = data.y;
    switch (a)
    {
    case WindowAlignment::CENTER:
        x = (glfwGetVideoMode(glfwGetPrimaryMonitor())->width - data.width) / 2;
        y = (glfwGetVideoMode(glfwGetPrimaryMonitor())->height - data.height) / 2;
        break;
    case WindowAlignment::TOP:
        x = (glfwGetVideoMode(glfwGetPrimaryMonitor())->width - data.width) / 2;
        y = 0;
        break;
    case WindowAlignment::TOP_LEFT:
        x = 0;
        y = 0;
        break;
    case WindowAlignment::TOP_RIGHT:
        x = (glfwGetVideoMode(glfwGetPrimaryMonitor())->width - data.width);
        y = 0;
        break;
    case WindowAlignment::LEFT:
        x = 0;
        y = (glfwGetVideoMode(glfwGetPrimaryMonitor())->height - data.height) / 2;
        break;
    case WindowAlignment::RIGHT:
        x = (glfwGetVideoMode(glfwGetPrimaryMonitor())->width - data.width);
        y = (glfwGetVideoMode(glfwGetPrimaryMonitor())->height - data.height) / 2;
        break;
    case WindowAlignment::BOTTOM:
        x = (glfwGetVideoMode(glfwGetPrimaryMonitor())->width - data.width) / 2;
        y = (glfwGetVideoMode(glfwGetPrimaryMonitor())->height - data.height);
        break;
    case WindowAlignment::BOTTOM_LEFT:
        x = 0;
        y = (glfwGetVideoMode(glfwGetPrimaryMonitor())->height - data.height);
        break;
    case WindowAlignment::BOTTOM_RIGHT:
        x = (glfwGetVideoMode(glfwGetPrimaryMonitor())->width - data.width);
        y = (glfwGetVideoMode(glfwGetPrimaryMonitor())->height - data.height);
        break;
    };
    glfwSetWindowPos(window, x, y);
}

// void Window::setIcon(shared<Texture> icon)
// {
//     GLFWimage images[1];
//     images[0].width = icon->getWidth();
//     images[0].height = icon->getHeight();
//     images[0].pixels = new unsigned char[images[0].width * images[0].height * 4];
//     icon->getData(images[0].pixels);
//     glfwSetWindowIcon(m_window, 1, images);
//     delete[] images[0].pixels;
// }