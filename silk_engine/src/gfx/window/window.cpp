#include "window.h"
#include "core/input/keys.h"
#include "core/event.h"
#include "gfx/images/raw_image.h"
#include <GLFW/glfw3.h>

Window::Window()
{
    data.window = this;
    if(!active_window)
        active_window = this;

    // Initializing input related stuff
    data.keys.resize(GLFW_KEY_LAST + 1);
    data.last_keys.resize(GLFW_KEY_LAST + 1);
    data.mouse_buttons.resize(GLFW_MOUSE_BUTTON_LAST + 1);
    data.last_mouse_buttons.resize(GLFW_MOUSE_BUTTON_LAST + 1);
  
    // Creating window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(data.width, data.height, data.title.c_str(), data.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    glfwDefaultWindowHints();

    glfwSetWindowUserPointer(window, &data);
    setVsync(vsync);
    align(WindowAlignment::CENTER);

    Dispatcher::post(WindowResizeEvent(*this, data.width, data.height));
    Dispatcher::post(WindowMoveEvent(*this, data.x, data.y));

    // Event callbacks
    glfwSetWindowSizeCallback(window,
        [](GLFWwindow *window, int width, int height)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;
            data.window->recreate();
            Dispatcher::post(WindowResizeEvent(*data.window, width, height));
        });

    glfwSetWindowPosCallback(window,
        [](GLFWwindow* window, int x, int y)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            if (!data.fullscreen)
            {
                data.x = x;
                data.y = y;
                Dispatcher::post(WindowMoveEvent(*data.window, x, y));
            }
        });

    glfwSetWindowMaximizeCallback(window,
        [](GLFWwindow* window, int maximized)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.maximized = maximized;
            Dispatcher::post(WindowMaximizeEvent(*data.window, maximized));
        });

    glfwSetWindowFocusCallback(window, 
        [](GLFWwindow* window, int focused) 
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.focused = focused;
            Dispatcher::post(WindowFocusEvent(*data.window, focused));
        });

    glfwSetWindowIconifyCallback(window, 
        [](GLFWwindow* window, int minimized) 
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.minimized = minimized;
            Dispatcher::post(WindowMinimizeEvent(*data.window, minimized));
        });

    glfwSetWindowRefreshCallback(window, 
        [](GLFWwindow* window) 
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher::post(WindowRefreshEvent(*data.window));
        });

    glfwSetWindowContentScaleCallback(window, 
        [](GLFWwindow* window, float x, float y)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher::post(WindowContentScaleEvent(*data.window, x, y));
            data.dpi = { x, y };
        });

    glfwSetWindowCloseCallback(window,
        [](GLFWwindow *window)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher::post(WindowCloseEvent(*data.window));
        });

    glfwSetKeyCallback(window,
        [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            switch (action)
            {
            case GLFW_PRESS:
            {
                Dispatcher::post(KeyPressEvent(*data.window, key, 0));
                data.keys[key] = true;
                break;
            }
            case GLFW_REPEAT:
            {
                Dispatcher::post(KeyPressEvent(*data.window, key, 1));
                break;
            }
            case GLFW_RELEASE:
            {
                Dispatcher::post(KeyReleaseEvent(*data.window, key));
                data.keys[key] = false;
                break;
            }
            }
        });

    glfwSetMouseButtonCallback(window,
        [](GLFWwindow *window, int button, int action, int mods)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.mouse_pressed = -1;
            switch (action)
            {
            case GLFW_PRESS:
            {
                Dispatcher::post(MousePressEvent(*data.window, button));
                data.mouse_pressed = button;
                data.mouse_buttons[button] = true;
                break;
            }
            case GLFW_RELEASE:
            {
                Dispatcher::post(MouseReleaseEvent(*data.window, button));
                data.mouse_buttons[button] = false;
                break;
            }
            }
        });

    glfwSetCursorPosCallback(window,
        [](GLFWwindow *window, double mx, double my)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            my = data.height - my;
            Dispatcher::post(MouseMoveEvent(*data.window, mx, my));
            data.mouse = vec2(mx, my);
            if (data.mouse_pressed != -1)
                Dispatcher::post(MouseDragEvent(*data.window, data.mouse_pressed, mx, my));
        });

    glfwSetCursorEnterCallback(window, 
        [](GLFWwindow* window, int entered)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.is_mouse_on = entered;
            Dispatcher::post(MouseEnterEvent(*data.window, entered));
        });

    glfwSetScrollCallback(window,
        [](GLFWwindow *window, double sx, double sy)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher::post(MouseScrollEvent(*data.window, sx, sy));
        });

    glfwSetDropCallback(window,
        [](GLFWwindow* window, int path_count, const char* paths[])
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            std::vector<const char*> paths_vec(path_count);
            for (size_t i = 0; i < paths_vec.size(); ++i)
                paths_vec[i] = paths[i];
            Dispatcher::post(DragAndDropEvent(*data.window, paths_vec));
        });

    glfwSetCharModsCallback(window,
        [](GLFWwindow* window, uint codepoint, int mods)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher::post(CharacterWriteEvent(*data.window, codepoint));
        });

    surface = new Surface(*this);
    swap_chain = new SwapChain(*surface);

    SK_TRACE("Window created: \"{}\" {}x{}", data.title.c_str(), data.width, data.height);
}

Window::~Window()
{
    delete swap_chain;
    delete surface;
    glfwDestroyWindow(window);
}

void Window::update()
{
    memcpy(data.last_mouse_buttons.data(), data.mouse_buttons.data(), data.mouse_buttons.size() * sizeof(bool));
    memcpy(data.last_keys.data(), data.keys.data(), data.keys.size() * sizeof(bool));
}

void Window::recreate()
{
    if (isMinimized())
        return;

    surface->update(getWidth(), getHeight());
    swap_chain->recreate();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Window::setVsync(bool vsync)
{
    if (this->vsync == vsync)
        return;
    this->vsync = vsync;
    glfwSwapInterval((int)vsync);
}

void Window::setFullscreen(bool fullscreen)
{
    if (data.fullscreen == fullscreen)
        return;
    data.fullscreen = fullscreen;
    if (fullscreen)
    {
        windowed.x = data.x;
        windowed.y = data.y;
        windowed.width = data.width;
        windowed.height = data.height;
        const GLFWvidmode* video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        setSize({ video_mode->width, video_mode->height });
        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, video_mode->width, video_mode->height, GLFW_DONT_CARE);
        data.x = 0;
        data.y = 0;
        data.width = video_mode->width;
        data.height = video_mode->height;
        Dispatcher::post(WindowMoveEvent(*this, data.x, data.y));
        Dispatcher::post(WindowResizeEvent(*this, data.width, data.height));
        
    }
    else
    {
        data.x = windowed.x;
        data.y = windowed.y;
        data.width = windowed.width;
        data.height = windowed.height;
        glfwSetWindowMonitor(window, nullptr, data.x, data.y, data.width, data.height, GLFW_DONT_CARE);
        Dispatcher::post(WindowMoveEvent(*this, data.x, data.y));
        Dispatcher::post(WindowResizeEvent(*this, data.width, data.height));
    }
    Dispatcher::post(WindowFullscreenEvent(*this, fullscreen));
}

void Window::setX(int32_t x)
{
    if (x == data.x)
        return;
    data.x = x;
    glfwSetWindowPos(window, data.x, data.y);
}

void Window::setY(int32_t y)
{
    if (y == data.y)
        return;
    data.y = y;
    glfwSetWindowPos(window, data.x, data.y);
}

void Window::setPosition(const ivec2 &position)
{
    if (position.x == data.x && position.y == data.y)
        return;
    data.x = position.x;
    data.y = position.y;
    glfwSetWindowPos(window, data.x, data.y);
}

void Window::setWidth(uint32_t width)
{
    if (width == data.width)
        return;
    data.width = width;
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setHeight(uint32_t height)
{
    if (height == data.height)
        return;
    data.height = height;
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setSize(const uvec2 &size)
{
    if (size.x == data.width && size.y == data.height)
        return;
    data.width = size.x;
    data.height = size.y;
    glfwSetWindowSize(window, data.width, data.height);
}

void Window::setTitle(std::string_view title)
{
    if (title == data.title)
        return;
    data.title = title;
    glfwSetWindowTitle(window, title.data());
}

void Window::setIcon(std::string_view file)
{
    std::string path = std::string("icons/") + file.data();
    RawImage<> data(path);
    std::vector<GLFWimage> icons(1);
    icons[0].height = data.height;
    icons[0].width = data.width;
    icons[0].pixels = data.pixels.data();
    glfwSetWindowIcon(window, icons.size(), icons.data());
}

void Window::setMinSize(uint32_t min_width, uint32_t min_height)
{
    glfwSetWindowSizeLimits(window, min_width, min_height, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void Window::setMaxSize(uint32_t max_width, uint32_t max_height)
{
    glfwSetWindowSizeLimits(window, GLFW_DONT_CARE, GLFW_DONT_CARE, max_width, max_height);
}

void Window::setAspectRatioLimit(uint32_t numerator, uint32_t denominator)
{
    glfwSetWindowAspectRatio(window, numerator, denominator);
}

void Window::setOpacity(float opacity)
{
    if (this->opacity == opacity)
        return;
    this->opacity = opacity;
    glfwSetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER, opacity < 1.0f); // TODO: This might invalidate some vulkan object and mess things up, test it
    glfwSetWindowOpacity(window, opacity);
}

void Window::setResizable(bool resizable)
{
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable);
}

void Window::setDecorated(bool decorated)
{
    glfwSetWindowAttrib(window, GLFW_DECORATED, decorated);
}

void Window::setAlwaysOnTop(bool always_on_top)
{
    glfwSetWindowAttrib(window, GLFW_FLOATING, always_on_top);
}

void Window::setCursor(std::string_view file, CursorHotSpot hot_spot)
{
    std::string path = std::string("cursors/") + file.data();
    RawImage<> raw(path);
    GLFWimage cursor_image[1];
    cursor_image[0].height = raw.height;
    cursor_image[0].width = raw.width;
    cursor_image[0].pixels = raw.pixels.data();

    if (data.cursor)
    {
        glfwDestroyCursor(data.cursor);
        data.cursor = nullptr;
    }

    int x, y;
    switch (hot_spot)
    {
    case CursorHotSpot::TOP:
        x = raw.width / 2;
        y = raw.height - 1;
        break;
    case CursorHotSpot::BOTTOM:
        x = raw.width / 2;
        y = 0;
        break;
    case CursorHotSpot::LEFT:
        x = 0;
        y = raw.height / 2;
        break;
    case CursorHotSpot::RIGHT:
        x = raw.width - 1;
        y = raw.height / 2;
        break;
    case CursorHotSpot::TOP_LEFT:
        x = 0;
        y = raw.height - 1;
        break;
    case CursorHotSpot::TOP_RIGHT:
        x = raw.width - 1;
        y = raw.height - 1;
        break;
    case CursorHotSpot::BOTTOM_LEFT:
        x = 0;
        y = 0;
        break;
    case CursorHotSpot::BOTTOM_RIGHT:
        x = raw.width - 1;
        y = 0;
        break;
    case CursorHotSpot::CENTER:
        x = raw.width / 2;
        y = raw.height / 2;
        break;
    default:
        x = raw.width - 1;
        y = raw.height - 1;
        break;
    }

    data.cursor = glfwCreateCursor(cursor_image, x, y);
    glfwSetCursor(Window::getGLFWWindow(), data.cursor);
}

void Window::setAutoMinify(bool auto_minify)
{
    if (this->auto_minify == auto_minify)
        return;
    this->auto_minify = auto_minify;
    glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, auto_minify);
}

void Window::setCursorMode(CursorMode mode)
{
    if (cursor_mode == mode)
        return;
    cursor_mode = mode;

    switch (mode)
    {
    case CursorMode::NORMAL:
        glfwSetInputMode(Window::getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case CursorMode::HIDDEN:
        glfwSetInputMode(Window::getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        break;
    case CursorMode::LOCKED:
        glfwSetInputMode(Window::getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    case CursorMode::CAPTURED:
        glfwSetInputMode(Window::getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        break;
    }
}

void Window::align(WindowAlignment a)
{
    if (a == WindowAlignment::NONE)
        return;

    auto& x = data.x;
    auto& y = data.y;
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

void Window::focus()
{
    glfwFocusWindow(window);
}

void Window::show()
{
    glfwShowWindow(window);
}

void Window::hide()
{
    glfwHideWindow(window);
}

void Window::minimize()
{
    glfwIconifyWindow(window);
}

void Window::maximize()
{
    glfwMaximizeWindow(window);
}

void Window::restore()
{
    glfwRestoreWindow(window);
}

void Window::requestAttention()
{
    glfwRequestWindowAttention(window);
}