#include "window.h"
#include "core/input/keys.h"
#include "core/event.h"
#include "gfx/images/raw_image.h"
#include <GLFW/glfw3.h>

static void GLFWErrorCallback(int error, const char *description)
{
    SK_ERROR("GLFW({}): {}", error, description);
}

void Window::init()
{
    // Initializing input related stuff
    input.keys.resize(GLFW_KEY_LAST + 1);
    input.last_keys.resize(GLFW_KEY_LAST + 1);
    input.mouse_buttons.resize(GLFW_MOUSE_BUTTON_LAST + 1);
    input.last_mouse_buttons.resize(GLFW_MOUSE_BUTTON_LAST + 1);
    Dispatcher::subscribe(onMousePress);
    Dispatcher::subscribe(onMouseRelease);
    Dispatcher::subscribe(onMouseMove);
    Dispatcher::subscribe(onKeyPress);
    Dispatcher::subscribe(onKeyRelease);

    // Initializing GLFW
    int success = glfwInit();
    SK_ASSERT(success, "GLFW: Couldn't initialize glfw");
    glfwSetErrorCallback(GLFWErrorCallback);
  
    // Creating window
    if (transparent)
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, transparent);
    glfwWindowHint(GLFW_DECORATED, decorated);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(data.width, data.height, data.title.c_str(), data.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

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
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;
            Dispatcher::post(WindowResizeEvent(width, height));
        });

    glfwSetWindowPosCallback(window,
        [](GLFWwindow* window, int x, int y)
        {
            UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            if (!data.fullscreen)
            {
                data.x = x;
                data.y = y;
                Dispatcher::post(WindowMoveEvent(x, y));
            }
        });

    glfwSetWindowMaximizeCallback(window,
        [](GLFWwindow* window, int maximized)
        {
            Dispatcher::post(WindowMaximizeEvent(maximized));
        });

    glfwSetWindowFocusCallback(window, 
        [](GLFWwindow* window, int focused) 
        {
            Dispatcher::post(WindowFocusEvent(focused));
        });

    glfwSetWindowIconifyCallback(window, 
        [](GLFWwindow* window, int iconified) 
        {
            Dispatcher::post(WindowIconifyEvent(iconified));
        });

    glfwSetWindowRefreshCallback(window, 
        [](GLFWwindow* window) 
        {
            Dispatcher::post(WindowRefreshEvent());
        });

    glfwSetWindowContentScaleCallback(window, 
        [](GLFWwindow* window, float x, float y) 
        {
            Dispatcher::post(WindowContentScaleEvent(x, y));
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
                Dispatcher::post(MousePressEvent(button));
                data.mouse_pressed = button;
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
            const UserPointer& data = *(UserPointer*)glfwGetWindowUserPointer(window);
            Dispatcher::post(MouseMoveEvent(mx, data.height - my));
            if (data.mouse_pressed != -1)
                Dispatcher::post(MouseDragEvent(data.mouse_pressed, mx, data.height - my));
        });

    glfwSetCursorEnterCallback(window, 
        [](GLFWwindow* window, int entered)
        {
            if (entered == GLFW_TRUE)
            {
                input.is_mouse_on = true;
                Dispatcher::post(MouseEnterEvent());
            }
            else
            {
                input.is_mouse_on = false;
                Dispatcher::post(MouseLeaveEvent());
            }
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
        [](GLFWwindow* window, uint codepoint, int mods)
        {
            Dispatcher::post(CharacterWriteEvent(codepoint));
        });

    glfwSetJoystickCallback(
        [](int id, int event)
        {
            Dispatcher::post(JoystickEvent(id, event == GLFW_CONNECTED));
        });

    SK_TRACE("Window created: \"{}\" {}x{}", data.title.c_str(), data.width, data.height);
}

void Window::update()
{
    memcpy(input.last_mouse_buttons.data(), input.mouse_buttons.data(), input.mouse_buttons.size() * sizeof(bool));
    memcpy(input.last_keys.data(), input.keys.data(), input.keys.size() * sizeof(bool));
}

void Window::destroy()
{
    glfwDestroyWindow(window);
}

void Window::onMousePress(const MousePressEvent& e)
{
    input.mouse_buttons[e.button] = true;
}

void Window::onMouseRelease(const MouseReleaseEvent& e)
{
    input.mouse_buttons[e.button] = false;
}

void Window::onMouseMove(const MouseMoveEvent& e)
{
    input.mouse = vec2(e.x, e.y);
}

void Window::onKeyPress(const KeyPressEvent& e)
{
    input.keys[e.key] = true;
}

void Window::onKeyRelease(const KeyReleaseEvent& e)
{
    input.keys[e.key] = false;
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
        Dispatcher::post(WindowMoveEvent(data.x, data.y));
        Dispatcher::post(WindowResizeEvent(data.width, data.height));
        
    }
    else
    {
        data.x = windowed.x;
        data.y = windowed.y;
        data.width = windowed.width;
        data.height = windowed.height;
        glfwSetWindowMonitor(window, nullptr, data.x, data.y, data.width, data.height, GLFW_DONT_CARE);
        Dispatcher::post(WindowMoveEvent(data.x, data.y));
        Dispatcher::post(WindowResizeEvent(data.width, data.height));
    }
    Dispatcher::post(WindowFullscreenEvent(fullscreen));
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

void Window::setSizeLimits(uint32_t min_width, uint32_t max_width, uint32_t min_height, uint32_t max_height)
{
    glfwSetWindowSizeLimits(window, min_width, min_height, max_width, max_height);
}

void Window::setAspectRatio(uint32_t numerator, uint32_t denominator)
{
    glfwSetWindowAspectRatio(window, numerator, denominator);
}

void Window::setCursor(std::string_view file, CursorHotSpot hot_spot)
{
    std::string path = std::string("cursors/") + file.data();
    RawImage<> raw(path);
    GLFWimage cursor_image[1];
    cursor_image[0].height = raw.height;
    cursor_image[0].width = raw.width;
    cursor_image[0].pixels = raw.pixels.data();

    if (input.cursor)
    {
        glfwDestroyCursor(input.cursor);
        input.cursor = nullptr;
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

    input.cursor = glfwCreateCursor(cursor_image, x, y);
    glfwSetCursor(Window::getGLFWWindow(), input.cursor);
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

void Window::lockMouse()
{
    glfwSetInputMode(Window::getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::unlockMouse()
{
    glfwSetInputMode(Window::getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}