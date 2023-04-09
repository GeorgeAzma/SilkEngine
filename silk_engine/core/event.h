#pragma once

#include <typeindex>

class Window;
class Monitor;
class Joystick;

//EVENT
struct Event
{
    virtual ~Event() = default;
};

//WINDOW EVENTS
struct WindowEvent : Event
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
struct MonitorEvent : Event
{
    MonitorEvent(Monitor& monitor, bool connected)
        : monitor(monitor), connected(connected) {}

    Monitor& monitor;
    const bool connected;
};

//KEY EVENTS
struct KeyPressEvent : WindowEvent
{
    KeyPressEvent(Window& window, uint16_t key, uint32_t repeat_count)
        : WindowEvent(window), key(key), repeat_count(repeat_count) {}

    const int key;
    const uint repeat_count;
 };

struct KeyReleaseEvent : WindowEvent
{
    KeyReleaseEvent(Window& window, uint16_t key)
        : WindowEvent(window), key(key) {}

    const uint16_t key;
 };

//MOUSE EVENTS
struct MousePressEvent : WindowEvent
{
    MousePressEvent(Window& window, uint32_t button)
        : WindowEvent(window), button(button) {}

    const uint32_t button;
 };

struct MouseReleaseEvent : WindowEvent
{
    MouseReleaseEvent(Window& window, uint32_t button)
        : WindowEvent(window), button(button) {}

    const uint32_t button;
 };

struct MouseMoveEvent : WindowEvent
{
    MouseMoveEvent(Window& window, double x, double y)
        : WindowEvent(window), x(x), y(y) {}

    const double x, y;
 };

struct MouseDragEvent : WindowEvent
{
    MouseDragEvent(Window& window, uint32_t button, double x, double y)
        : WindowEvent(window), button(button), x(x), y(y) {}

    const uint32_t button;
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

struct JoystickEvent : Event
{
    JoystickEvent(Joystick& joystick, bool connected)
        : joystick(joystick), connected(connected) {}

    Joystick& joystick;
    const bool connected;
};

class HandlerFunctionBase
{
public:
    virtual void operator()(const Event& event) const = 0;
    virtual bool operator==(size_t func) const = 0;
};

template <class T, class EventType>
class MemberFunctionHandler : public HandlerFunctionBase
{
    typedef void (T::* MemberFunction)(const EventType&);
public:
    MemberFunctionHandler(T& instance, MemberFunction member_function)
        : instance(instance), member_function(member_function) {}

    void operator()(const Event& event) const override
    {
        (instance.*member_function)(static_cast<const EventType&>(event));
    }

    bool operator==(size_t func) const override
    {
        union {
            void (T::* member_function)(const EventType&);
            size_t hash;
        } converter;
        converter.member_function = member_function;
        return func == (size_t(&instance) ^ converter.hash);
    }

private:
    T& instance;
    MemberFunction member_function;
};

class Dispatcher
{
public:
    template <typename EventType>
    static void post(const EventType& e)
    {
        auto it = subscribers.find(typeid(EventType));
        if (it == subscribers.end())
            return;
        for (auto&& func : it->second.functions)
            func(e);
        for (auto&& func : it->second.member_functions)
            (*func)(e);
    }

    template <class T, class EventType>
    static void subscribe(T& instance, void (T::* member_function)(const EventType&))
    {
        subscribers[typeid(EventType)].member_functions.emplace_back(new MemberFunctionHandler<T, EventType>(instance, member_function));
    }

    template <class EventType>
    static void subscribe(void (*function)(const EventType&))
    {
        subscribers[typeid(EventType)].functions.emplace_back((void(*)(const Event&))function);
    }

    template <class EventType>
    static void unsubscribe(void (*function)(const EventType&))
    {
        auto it = subscribers.find(typeid(EventType));
        if (it == subscribers.end())
            return;
        auto& functions = it->second.functions;
        for (auto& func : functions)
        {
            if (size_t(func) == size_t(function))
            {
                std::swap(func, functions.back());
                functions.pop_back();
                break;
            }
        }
    }

    template <class T, class EventType>
    static void unsubscribe(T& instance, void (T::* member_function)(const EventType&))
    {
        auto it = subscribers.find(typeid(EventType));
        if (it == subscribers.end())
            return;
        union {
            void (T::* member_function)(const EventType&);
            size_t hash;
        } converter;
        converter.member_function = member_function;
        size_t function_hash = size_t(&instance) ^ converter.hash;
        auto& member_functions = it->second.member_functions;
        for (auto& member_func : member_functions)
        {
            if (*member_func == function_hash)
            {
                std::swap(member_func, member_functions.back());
                member_functions.pop_back();
                break;
            }
        }
    }

private:
    struct Subscribers
    {
        std::vector<void(*)(const Event&)> functions;
        std::vector<unique<HandlerFunctionBase>> member_functions;
    };
    static inline std::unordered_map<std::type_index, Subscribers> subscribers;
};