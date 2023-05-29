#pragma once

#include <typeindex>

class Window;
class Monitor;
class Joystick;
enum class Key : int;
enum class MouseButton : int;
enum class JoystickButton : uint8_t;
enum class GamepadButton : uint8_t;
enum class GamepadAxis : uint8_t;

// WINDOW EVENTS
struct WindowEvent
{
    WindowEvent(Window& window) 
        : window(window) {}

    Window& window;
};

struct WindowResizeEvent : public WindowEvent
{
    WindowResizeEvent(Window& window, uint32_t width, uint32_t height)
        : WindowEvent(window), width(width), height(height) {}

    const uint32_t width, height;
};

struct FramebufferResizeEvent : public WindowEvent
{
    FramebufferResizeEvent(Window& window, uint32_t width, uint32_t height)
        : WindowEvent(window), width(width), height(height) {}

    const uint32_t width, height;
};

struct WindowCloseEvent : WindowEvent
{
    WindowCloseEvent(Window& window)
        : WindowEvent(window) {}
};

struct WindowMaximizeEvent : WindowEvent
{
    WindowMaximizeEvent(Window& window, bool maximized)
        : WindowEvent(window), maximized(maximized) {}

    const bool maximized;
};

struct WindowFocusEvent : WindowEvent
{
    WindowFocusEvent(Window& window, bool focused)
        : WindowEvent(window), focused(focused) {}

    const bool focused;
};

struct WindowMinimizeEvent : WindowEvent
{
    WindowMinimizeEvent(Window& window, bool minimized)
        : WindowEvent(window), minimized(minimized) {}

    const bool minimized;
};

struct WindowRefreshEvent : WindowEvent
{
    WindowRefreshEvent(Window& window)
        : WindowEvent(window) {}
};

struct WindowMoveEvent : WindowEvent
{
    WindowMoveEvent(Window& window, int32_t x, int32_t y)
        : WindowEvent(window), x(x), y(y) {}

    const int32_t x, y;
};

struct WindowContentScaleEvent : WindowEvent
{
    WindowContentScaleEvent(Window& window, float x, float y)
        : WindowEvent(window), x(x), y(y) {}

    const float x, y;
};

//MONITOR EVENTS
struct MonitorEvent
{
    MonitorEvent(Monitor& monitor, bool connected)
        : monitor(monitor), connected(connected) {}

    Monitor& monitor;
    const bool connected;
};

//KEY EVENTS
struct KeyPressEvent : WindowEvent
{
    KeyPressEvent(Window& window, Key key, int repeat_count)
        : WindowEvent(window), key(key), repeat_count(repeat_count) {}

    const Key key;
    const int repeat_count;
 };

struct KeyReleaseEvent : WindowEvent
{
    KeyReleaseEvent(Window& window, Key key)
        : WindowEvent(window), key(key) {}

    const Key key;
 };

//MOUSE EVENTS
struct MousePressEvent : WindowEvent
{
    MousePressEvent(Window& window, MouseButton button)
        : WindowEvent(window), button(button) {}

    const MouseButton button;
 };

struct MouseReleaseEvent : WindowEvent
{
    MouseReleaseEvent(Window& window, MouseButton button)
        : WindowEvent(window), button(button) {}

    const MouseButton button;
 };

struct MouseMoveEvent : WindowEvent
{
    MouseMoveEvent(Window& window, double x, double y)
        : WindowEvent(window), x(x), y(y) {}

    const double x, y;
 };

struct MouseDragEvent : WindowEvent
{
    MouseDragEvent(Window& window, MouseButton button, double x, double y)
        : WindowEvent(window), button(button), x(x), y(y) {}

    const MouseButton button;
    const double x, y;
 };

struct MouseScrollEvent : public WindowEvent
{
    MouseScrollEvent(Window& window, double x, double y)
        : WindowEvent(window), x(x), y(y) {}

    const double x, y;
 };

struct MouseEnterEvent : WindowEvent
{
    MouseEnterEvent(Window& window, bool entered)
        : WindowEvent(window), entered(entered) {}

    const bool entered;
};

struct DragAndDropEvent : WindowEvent
{
    DragAndDropEvent(Window& window, const std::vector<const char*>& paths)
        : WindowEvent(window), paths(paths) {}

    const std::vector<const char*> paths;
};

struct WindowFullscreenEvent : WindowEvent
{
    WindowFullscreenEvent(Window& window, bool fullscreen)
        : WindowEvent(window), fullscreen(fullscreen) {}

    const bool fullscreen;
};

struct CharacterWriteEvent : WindowEvent
{
    CharacterWriteEvent(Window& window, char character)
        : WindowEvent(window), character(character) {}

    const char character;
};

struct JoystickEvent
{
    JoystickEvent(Joystick& joystick, bool connected)
        : joystick(joystick), connected(connected) {}

    Joystick& joystick;
    const bool connected;
};

struct JoystickPressEvent
{
    JoystickPressEvent(Joystick& joystick, JoystickButton button)
        : joystick(joystick), button(button) {}

    Joystick& joystick;
    const JoystickButton button;
};

struct JoystickReleaseEvent
{
    JoystickReleaseEvent(Joystick& joystick, JoystickButton button)
        : joystick(joystick), button(button) {}

    Joystick& joystick;
    const JoystickButton button;
};

struct JoystickHatEvent
{
    JoystickHatEvent(Joystick& joystick, int hat, const ivec2& direction)
        : joystick(joystick), hat(hat), direction(direction) {}

    Joystick& joystick;
    const int hat;
    const ivec2 direction;
};

struct JoystickAxisEvent
{
    JoystickAxisEvent(Joystick& joystick, int axis, float value, float delta)
        : joystick(joystick), axis(axis), value(value), delta(delta) {}

    Joystick& joystick;
    const int axis;
    const float value;
    const float delta;
};

struct GamepadPressEvent
{
    GamepadPressEvent(Joystick& joystick, GamepadButton button)
        : joystick(joystick), button(button) {}

    Joystick& joystick;
    const GamepadButton button;
};

struct GamepadReleaseEvent
{
    GamepadReleaseEvent(Joystick& joystick, GamepadButton button)
        : joystick(joystick), button(button) {}

    Joystick& joystick;
    const GamepadButton button;
};

struct GamepadAxisEvent
{
    GamepadAxisEvent(Joystick& joystick, GamepadAxis axis, float value, float delta)
        : joystick(joystick), axis(axis), value(value), delta(delta) {}

    Joystick& joystick;
    const GamepadAxis axis;
    const float value;
    const float delta;
};

template<typename EventType>
class HandlerFunctionBase
{
public:
    virtual void operator()(const EventType& event) const = 0;
    virtual bool operator==(size_t func) const = 0;
};

template <typename T, typename EventType>
class MemberFunctionHandler : public HandlerFunctionBase<EventType>
{
    typedef void (T::* MemberFunction)(const EventType&);
public:
    MemberFunctionHandler(T& instance, MemberFunction member_function)
        : instance(instance), member_function(member_function) {}

    void operator()(const EventType& event) const
    {
        (instance.*member_function)(event);
    }

    bool operator==(size_t func) const override
    {
        union {
            MemberFunction member_function;
            size_t hash;
        } converter;
        converter.member_function = member_function;
        return func == (size_t(&instance) ^ converter.hash);
    }

private:
    T& instance;
    MemberFunction member_function;
};

template <typename EventType>
class Dispatcher
{
public:
    static void post(const EventType& e)
    {
        for (auto& func : subscribed_functions)
            func(e);
        for (auto& func : subscribed_member_functions)
            (*func)(e);
    }

    template <typename... Args>
    static void post(Args&&... args)
    {
        EventType e(std::forward<Args>(args)...);
        for (auto& func : subscribed_functions)
            func(e);
        for (auto& func : subscribed_member_functions)
            (*func)(e);
    }

    template <typename T>
    static void subscribe(T& instance, void (T::* member_function)(const EventType&))
    {
        subscribed_member_functions.emplace_back(new MemberFunctionHandler<T, EventType>(instance, member_function));
    }

    static void subscribe(void (*function)(const EventType&))
    {
        subscribed_functions.emplace_back(function);
    }

    static void unsubscribe(void (*function)(const EventType&))
    {
        for (auto& func : subscribed_functions)
        {
            if (func == function)
            {
                std::swap(func, subscribed_functions.back());
                subscribed_functions.pop_back();
                break;
            }
        }
    }

    template <typename T>
    static void unsubscribe(T& instance, void (T::* member_function)(const EventType&))
    {
        union {
            void (T::* member_function)(const EventType&);
            size_t hash;
        } converter;
        converter.member_function = member_function;
        size_t function_hash = size_t(&instance) ^ converter.hash;
        for (auto& member_func : subscribed_member_functions)
        {
            if (*member_func == function_hash)
            {
                std::swap(member_func, subscribed_member_functions.back());
                subscribed_member_functions.pop_back();
                break;
            }
        }
    }

private:
    static inline std::vector<void(*)(const EventType&)> subscribed_functions;
    static inline std::vector<unique<HandlerFunctionBase<EventType>>> subscribed_member_functions;
};