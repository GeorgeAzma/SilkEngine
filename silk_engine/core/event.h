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

// Dispatching and such code
class HandlerFunctionBase
{
public:
    virtual void operator()(const Event& event) const = 0;

    virtual bool operator==(const HandlerFunctionBase& other) const = 0;
};

template <typename T>
class HandlerFunctionComparator : public HandlerFunctionBase
{
public:
    virtual bool operator==(const HandlerFunctionBase& other) const override
    {
        if (const T* self = dynamic_cast<const T*>(&other))
            return ((T*)this)->operator==(*self);
        return false;
    }
};

template <class T, class EventType>
class MemberFunctionHandler : public HandlerFunctionComparator<MemberFunctionHandler<T, EventType>>
{
public:
    typedef void (T::* MemberFunction)(const EventType&);

    MemberFunctionHandler(T* instance, MemberFunction member_function)
        : instance(instance), member_function(member_function) {}

    void operator()(const Event& event) const
    {
        SK_VERIFY(instance && member_function, 
            "Attempted to call \"onEvent(const EventType&)\" when it was nullptr, did you forget to unsubscribe event when class got deleted?");
        (instance->*member_function)((const EventType&)event);
    }

    bool operator==(const MemberFunctionHandler& other) const
    {
        return other.instance == instance && other.member_function == member_function;
    }

private:
    T* instance;
    MemberFunction member_function;
};

template <class EventType>
class FunctionHandler : public HandlerFunctionComparator<FunctionHandler<EventType>>
{
public:
    typedef void (*Function)(const EventType&);

    FunctionHandler(Function function)
        : function(function) {}

    void operator()(const Event& event) const
    {
        SK_ASSERT(function,
            "Attempted to call \"onEvent(const EventType&)\" when it was nullptr, did you forget to unsubscribe event before lambda function got deleted?");
        (*function)(static_cast<const EventType&>(event));
    }

    bool operator==(const FunctionHandler& other) const
    {
        return other.function == function;
    }

private:
    Function function;
};

class Dispatcher
{
public:
    template <typename EventType>
    static void post(const EventType& e)
    {
        auto it = subscribers.find(typeid(EventType));
        if (it != subscribers.cend())
        {
            const auto& handlers = it->second;
            for (const auto& handler : handlers)
                (*handler)(e);
        }
    }

    template <class T, class EventType>
    static void subscribe(T* instance, void (T::* member_function)(const EventType&))
    {
        subscribers[typeid(EventType)].emplace_back(new MemberFunctionHandler<T, EventType>(instance, member_function));
    }

    template <class EventType>
    static void subscribe(void (*function)(const EventType&))
    {
        subscribers[typeid(EventType)].emplace_back(new FunctionHandler<EventType>(function));
    }

    template <class EventType>
    static void unsubscribe(void (*function)(const EventType&))
    {
        const FunctionHandler<EventType> comp(function); // tmp for operator==
        auto& handlers = subscribers.at(typeid(EventType));
        for (size_t i = 0; i < handlers.size(); ++i)
        {
            if (*handlers[i] == comp)
            {
                std::swap(handlers[i], handlers.back());
                handlers.pop_back();
                break;
            }
        }
    }

    template <class T, class EventType>
    static void unsubscribe(T* instance, void (T::* member_function)(const EventType&))
    {
        const MemberFunctionHandler<T, EventType> comp(instance, member_function); // tmp for operator==
        auto& handlers = subscribers.at(typeid(EventType));
        for (size_t i = 0; i < handlers.size(); ++i)
        {
            if (comp == *handlers[i])
            {
                std::swap(handlers[i], handlers.back());
                handlers.pop_back();
                break;
            }
        }
    }

private:
    static inline std::map<std::type_index, std::vector<shared<HandlerFunctionBase>>> subscribers;
};