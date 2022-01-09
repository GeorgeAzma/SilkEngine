#include "window.h"
#include "event.h"

static bool GLFW_initialized = false;

static void GLFWErrorCallback(int error, const char *description)
{
    VE_CORE_ERROR("GLFW({0}): {1}", error, description);
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

    // RenderCommand::setViewport(0, 0, data.width, data.height);

    glfwDefaultWindowHints();
    glfwShowWindow(window);

    glfwSetWindowUserPointer(window, &data);
    setVsync(data.vsync);
    align(WindowAlignment::CENTER);

    // Event callbacks
    glfwSetWindowSizeCallback(window,
        [](GLFWwindow *window, int width, int height)
        {
            Data &data = *(Data *)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;
            data.projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
            Dispatcher::post(WindowResizeEvent(data.width, data.height, window));
        });

    glfwSetWindowCloseCallback(window,
        [](GLFWwindow *window)
        {
            Data &data = *(Data *)glfwGetWindowUserPointer(window);
            Dispatcher::post(WindowCloseEvent(window));
        });

    glfwSetKeyCallback(window,
        [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            Data &data = *(Data *)glfwGetWindowUserPointer(window);
            switch (action)
            {
            case GLFW_PRESS:
            {
                data.input.latestKeyDown = key;
                data.input.keysDown[key] = true;
                data.input.anyKeyDown = true;
                Dispatcher::post(KeyPressEvent(key, 0, window));
                break;
            }
            case GLFW_REPEAT:
            {
                data.input.latestKeyDown = key;
                data.input.keysDown[key] = true;
                data.input.anyKeyDown = true;
                Dispatcher::post(KeyPressEvent(key, 1, window));
                break;
            }
            case GLFW_RELEASE:
            {
                data.input.keysDown[key] = false;
                data.input.anyKeyDown = std::accumulate(data.input.keysDown.begin(), data.input.keysDown.end(), 0);
                Dispatcher::post(KeyReleaseEvent(key, window));
                break;
            }
            }
        });

    glfwSetMouseButtonCallback(window,
        [](GLFWwindow *window, int button, int action, int mods)
        {
            Data &data = *(Data *)glfwGetWindowUserPointer(window);
            switch (action)
            {
            case GLFW_PRESS:
            {
                data.input.latestButtonDown = button;
                data.input.buttonsDown[button] = true;
                data.input.anyButtonDown = true;
                Dispatcher::post(MousePressEvent(button, window));
                break;
            }
            case GLFW_RELEASE:
            {
                data.input.buttonsDown[button] = false;
                data.input.anyButtonDown = std::accumulate(data.input.buttonsDown.begin(), data.input.buttonsDown.end(), 0);
                Dispatcher::post(MouseReleaseEvent(button, window));
                break;
            }
            }
        });

    glfwSetCursorPosCallback(window,
        [](GLFWwindow *window, double mx, double my)
        {
            Data &data = *(Data *)glfwGetWindowUserPointer(window);
            Dispatcher::post(MouseMoveEvent(mx, (double)data.height - my, window));
            if (data.input.anyButtonDown)
            {
                Dispatcher::post(MouseDragEvent(data.input.latestButtonDown, mx, (double)data.height - my, window));
            }
        });

    glfwSetScrollCallback(window,
        [](GLFWwindow *window, double sx, double sy)
        {
            Data &data = *(Data *)glfwGetWindowUserPointer(window);
            Dispatcher::post(MouseScrollEvent(sx, sy, window));
        });

    glfwSetWindowRefreshCallback(window, 
        [](GLFWwindow* window)
        {
        });
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
    align();
    glfwSetWindowSize(window, data.width, data.height);
    updateProjection();
}

void Window::setHeight(unsigned int height)
{
    if (height == data.height)
        return;
    data.height = height;
    align();
    glfwSetWindowSize(window, data.width, data.height);
    updateProjection();
}

void Window::setSize(const glm::uvec2 &size)
{
    if (size.x == data.width && size.y == data.height)
        return;
    data.width = size.x;
    data.height = size.y;
    align();
    glfwSetWindowSize(window, data.width, data.height);
    updateProjection();
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