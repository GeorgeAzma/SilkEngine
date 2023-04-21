#include "window.h"
#include "core/input/keys.h"
#include "core/event.h"
#include "gfx/images/raw_image.h"
#include "monitor.h"
#include "surface.h"
#include "swap_chain.h"

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

    window = glfwCreateWindow(data.width, data.height, data.title.c_str(), data.fullscreen ? Monitor::getPrimary() : nullptr, nullptr);

    glfwDefaultWindowHints();

    glfwSetWindowUserPointer(window, &data);
    align(WindowAlignment::CENTER);

    Dispatcher<WindowResizeEvent>::post(*this, data.width, data.height);
    Dispatcher<WindowMoveEvent>::post(*this, data.x, data.y);

    glfwSetWindowPosCallback(window,
        [](GLFWwindow* window, int x, int y)
        {
            SK_INFO("Window moved: ({}, {})", x, y);
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            if (!data.fullscreen)
            {
                data.x = x;
                data.y = y;
                Dispatcher<WindowMoveEvent>::post(*data.window, x, y);
            }
        });

    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow* window, int width, int height)
        {
            SK_INFO("Framebuffer resized: {}x{}", width, height);
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;
            Dispatcher<FramebufferResizeEvent>::post(*data.window, width, height);
        });

    // Event callbacks
    glfwSetWindowSizeCallback(window,
        [](GLFWwindow *window, int width, int height)
        {
            SK_INFO("Window resized: {}x{}", width, height);
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;
            Dispatcher<WindowResizeEvent>::post(*data.window, width, height);
        });

    glfwSetWindowMaximizeCallback(window,
        [](GLFWwindow* window, int maximized)
        {
            SK_INFO("Window {}", maximized == GLFW_TRUE ? "maximized" : "restored");
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.maximized = maximized;
            Dispatcher<WindowMaximizeEvent>::post(*data.window, maximized);
        });

    glfwSetWindowFocusCallback(window, 
        [](GLFWwindow* window, int focused) 
        {
            SK_INFO("Window {}focused", focused == GLFW_TRUE ? "" : "un");
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.focused = focused;
            Dispatcher<WindowFocusEvent>::post(*data.window, focused);
        });

    glfwSetWindowIconifyCallback(window, 
        [](GLFWwindow* window, int minimized) 
        {
            SK_INFO("Window {}", minimized == GLFW_TRUE ? "minimized" : "restored");
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.minimized = minimized;
            Dispatcher<WindowMinimizeEvent>::post(*data.window, minimized);
        });

    glfwSetWindowRefreshCallback(window, 
        [](GLFWwindow* window) 
        {
            SK_INFO("Window refreshed");
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher<WindowRefreshEvent>::post(*data.window);
        });

    glfwSetWindowContentScaleCallback(window, 
        [](GLFWwindow* window, float x, float y)
        {
            SK_INFO("Window content scaled: ({}, {})", x, y);
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher<WindowContentScaleEvent>::post(*data.window, x, y);
            data.dpi = { x, y };
        });

    glfwSetWindowCloseCallback(window,
        [](GLFWwindow *window)
        {
            SK_INFO("Window closed");
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher<WindowCloseEvent>::post(*data.window);
        });

    glfwSetKeyCallback(window,
        [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            switch (action)
            {
            case GLFW_PRESS:
            {
                Dispatcher<KeyPressEvent>::post(*data.window, key, 0);
                data.keys[key] = true;
                break;
            }
            case GLFW_REPEAT:
            {
                Dispatcher<KeyPressEvent>::post(*data.window, key, 1);
                break;
            }
            case GLFW_RELEASE:
            {
                Dispatcher<KeyReleaseEvent>::post(*data.window, key);
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
                Dispatcher<MousePressEvent>::post(*data.window, button);
                data.mouse_pressed = button;
                data.mouse_buttons[button] = true;
                break;
            }
            case GLFW_RELEASE:
            {
                Dispatcher<MouseReleaseEvent>::post(*data.window, button);
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
            Dispatcher<MouseMoveEvent>::post(*data.window, mx, my);
            data.mouse = vec2(mx, my);
            if (data.mouse_pressed != -1)
                Dispatcher<MouseDragEvent>::post(*data.window, data.mouse_pressed, mx, my);
        });

    glfwSetCursorEnterCallback(window, 
        [](GLFWwindow* window, int entered)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.is_mouse_on = entered;
            Dispatcher<MouseEnterEvent>::post(*data.window, entered);
        });

    glfwSetScrollCallback(window,
        [](GLFWwindow *window, double sx, double sy)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher<MouseScrollEvent>::post(*data.window, sx, sy);
        });

    glfwSetDropCallback(window,
        [](GLFWwindow* window, int path_count, const char* paths[])
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            std::vector<const char*> paths_vec(path_count);
            for (size_t i = 0; i < paths_vec.size(); ++i)
                paths_vec[i] = paths[i];
            Dispatcher<DragAndDropEvent>::post(*data.window, paths_vec);
        });

    glfwSetCharModsCallback(window,
        [](GLFWwindow* window, uint codepoint, int mods)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher<CharacterWriteEvent>::post(*data.window, codepoint);
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

    if (surface)
        surface->updateCapabilities();
    if (swap_chain)
        swap_chain->recreate();
}

GLFWmonitor* Window::getMonitor() const
{
    return glfwGetWindowMonitor(window);
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
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
        Dispatcher<WindowMoveEvent>::post(*this, data.x, data.y);
        Dispatcher<WindowResizeEvent>::post(*this, data.width, data.height);
        
    }
    else
    {
        data.x = windowed.x;
        data.y = windowed.y;
        data.width = windowed.width;
        data.height = windowed.height;
        glfwSetWindowMonitor(window, nullptr, data.x, data.y, data.width, data.height, GLFW_DONT_CARE);
        Dispatcher<WindowMoveEvent>::post(*this, data.x, data.y);
        Dispatcher<WindowResizeEvent>::post(*this, data.width, data.height);
    }
    Dispatcher<WindowFullscreenEvent>::post(*this, fullscreen);
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

void Window::setIcon(const path& file)
{
    path file_path = path("icons") / file;
    RawImage<> data(file_path);
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
    glfwSetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER, opacity < 1.0f); // TODO: This may invalidate some vulkan object and mess things up, test it
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

void Window::setCursor(const path& file, CursorHotSpot hot_spot)
{
    path file_path = path("cursors") / file;
    RawImage<> raw(file_path);
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
    glfwSetCursor(window, data.cursor);
}

void Window::setAutoMinify(bool auto_minify)
{
    if (this->auto_minify == auto_minify)
        return;
    this->auto_minify = auto_minify;
    glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, auto_minify);
}

void Window::setFocusOnShow(bool focus_on_show)
{
    if (this->focus_on_show == focus_on_show)
        return;
    this->focus_on_show = focus_on_show;
    glfwSetWindowAttrib(window, GLFW_FOCUS_ON_SHOW, focus_on_show);
}

void Window::setCursorMode(CursorMode mode)
{
    if (cursor_mode == mode)
        return;
    cursor_mode = mode;

    switch (mode)
    {
    case CursorMode::NORMAL:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case CursorMode::HIDDEN:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        break;
    case CursorMode::LOCKED:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    case CursorMode::CAPTURED:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        break;
    }
}

void Window::setVsync(bool vsync)
{
    if (this->vsync == vsync)
        return;
    this->vsync = vsync;
    if (swap_chain)
        swap_chain->recreate(vsync);
}

void Window::align(WindowAlignment a)
{
    if (a == WindowAlignment::NONE)
        return;

    auto& x = data.x;
    auto& y = data.y;
    switch (a)
    {
    case WindowAlignment::CENTER:Monitor::getPrimary().getPhysicalSize();
        x = (Monitor::getPrimary().getWorkArea().z - data.width) / 2;
        y = (Monitor::getPrimary().getWorkArea().w - data.height) / 2;
        break;
    case WindowAlignment::TOP:
        x = (Monitor::getPrimary().getWorkArea().z - data.width) / 2;
        y = 0;
        break;
    case WindowAlignment::TOP_LEFT:
        x = 0;
        y = 0;
        break;
    case WindowAlignment::TOP_RIGHT:
        x = (Monitor::getPrimary().getWorkArea().z - data.width);
        y = 0;
        break;
    case WindowAlignment::LEFT:
        x = 0;
        y = (Monitor::getPrimary().getWorkArea().w - data.height) / 2;
        break;
    case WindowAlignment::RIGHT:
        x = (Monitor::getPrimary().getWorkArea().z - data.width);
        y = (Monitor::getPrimary().getWorkArea().w - data.height) / 2;
        break;
    case WindowAlignment::BOTTOM:
        x = (Monitor::getPrimary().getWorkArea().z - data.width) / 2;
        y = (Monitor::getPrimary().getWorkArea().w - data.height);
        break;
    case WindowAlignment::BOTTOM_LEFT:
        x = 0;
        y = (Monitor::getPrimary().getWorkArea().w - data.height);
        break;
    case WindowAlignment::BOTTOM_RIGHT:
        x = (Monitor::getPrimary().getWorkArea().z - data.width);
        y = (Monitor::getPrimary().getWorkArea().w - data.height);
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