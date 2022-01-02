#include "window.h"

static bool GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char *description)
{
    VE_CORE_ERROR("GLFW Error (%i): {%s}", error, description);
}

Window::Window(const WindowProps &props)
    : transparent(props.transparent)
{
    data.width = props.width;
    data.height = props.height;
    data.title = props.title;
    data.x = 0;
    data.y = 0;
    data.fullscreen = false;
    data.vsync = false;
    updateProjection();

    if (!GLFWInitialized)
    {
        int success = glfwInit();
        VE_CORE_ASSERT(success, "Couldn't initialize glfw");
        glfwSetErrorCallback(GLFWErrorCallback);
        GLFWInitialized = true;
    }

    if (transparent)
    {
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(data.width, data.height, data.title.c_str(), data.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    glfwDefaultWindowHints();
    glfwShowWindow(window);

    // Set pointer to window data somewhere in memory
    glfwSetWindowUserPointer(window, &data);
    setVsync(data.vsync);
    align(WindowAlignment::CENTER);

    // Event callbacks
}

Window::~Window()
{
    glfwDestroyWindow(window);
}

void Window::update()
{
    glfwPollEvents();
}

void Window::setVsync(bool vsync)
{
    if (data.vsync == vsync)
        return;
    data.vsync = vsync;
    glfwSwapInterval((int)vsync);
}

void Window::setFullscreen(bool fullscreen)
{
    if (data.fullscreen == fullscreen)
        return;
    data.fullscreen = fullscreen;
    if (fullscreen)
        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, data.width, data.height, GLFW_DONT_CARE);
    else
        glfwSetWindowMonitor(window, nullptr, data.x, data.y, data.width, data.height, GLFW_DONT_CARE);
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
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setHeight(unsigned int height)
{
    if (height == data.height)
        return;
    data.height = height;
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setSize(const glm::uvec2 &size)
{
    if (size.x == data.width && size.y == data.height)
        return;
    data.width = size.x;
    data.height = size.y;
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setTitle(const std::string &title)
{
    if (title == data.title)
        return;
    data.title = title;
    glfwSetWindowTitle(window, data.title.c_str());
}

void Window::align(WindowAlignment a)
{
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

// void Window::setIcon(std::shared_ptr<Texture> icon)
// {
//     GLFWimage images[1];
//     images[0].width = icon->getWidth();
//     images[0].height = icon->getHeight();
//     images[0].pixels = new unsigned char[images[0].width * images[0].height * 4];
//     icon->getData(images[0].pixels);
//     glfwSetWindowIcon(m_window, 1, images);
//     delete[] images[0].pixels;
// }

void Window::updateProjection()
{
    data.projection = glm::ortho(0.0f, (float)data.width, 0.0f, (float)data.height);
}