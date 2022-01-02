#pragma once
#include <map>
#include <typeindex>
#include <vector>
#include <memory>

// Different core events
class Event
{
public:
    virtual ~Event() {}
};

class WindowResizeEvent : Event
{
public:
    WindowResizeEvent(int width, int height)
        : width{width}, height{height} {}

    const int width, height;
};

class WindowCloseEvent : Event
{
public:
    WindowCloseEvent() {}
};

class KeyPressEvent : Event
{
public:
    KeyPressEvent(uint16_t keycode, unsigned int repeat_count = 0)
        : keycode{keycode}, repeat_count{repeat_count} {}

private:
    uint16_t keycode;
    unsigned int repeat_count;
};

class KeyReleaseEvent : Event
{
public:
    KeyReleaseEvent(uint16_t keycode)
        : keycode{keycode} {}

private:
    uint16_t keycode;
};

class MousePressEvent : Event
{
public:
    MousePressEvent(uint16_t button)
        : button{button} {}

private:
    uint16_t button;
};

class MouseReleaseEvent : Event
{
public:
    MouseReleaseEvent(uint16_t button)
        : button{button} {}

private:
    uint16_t button;
};

class MouseMoveEvent : Event
{
public:
    MouseMoveEvent(double x, double y)
        : x{x}, y{y} {}

private:
    double x, y;
};

class MouseDragEvent : Event
{
public:
    MouseDragEvent(uint16_t button, double x, double y)
        : button{button}, x{x}, y{y} {}

private:
    uint16_t button;
    double x, y;
};

class MouseScrollEvent : Event
{
public:
    MouseScrollEvent(double x, double y)
        : x{x}, y{y} {}

private:
    double x, y;
};

// Dispatching and such code
class HandlerFunctionBase
{
public:
    void exec(const Event &event) const
    {
        call(event);
    }

private:
    virtual void call(const Event &event) const = 0;
};

template <class T, class EventType>
class MemberFunctionHandler : public HandlerFunctionBase
{
public:
    typedef void (T::*MemberFunction)(const EventType &);

    MemberFunctionHandler(T *instance, MemberFunction member_function) : instance{instance}, member_function{member_function} {};

    void call(const Event &event) const
    {
        if (instance && member_function)
        {
            (instance->*member_function)(static_cast<const EventType &>(event));
        }
    }

private:
    T *instance;
    MemberFunction member_function;
};

template <class EventType>
class FunctionHandler : public HandlerFunctionBase
{
public:
    typedef void (*Function)(const EventType &);

    FunctionHandler(Function function) : function{function} {};

    void call(const Event &event) const
    {
        if (function)
        {
            (*function)(static_cast<const EventType &>(event));
        }
    }

private:
    Function function;
};

class Dispatcher
{
    typedef std::vector<std::shared_ptr<HandlerFunctionBase>> HandlerList;

public:
    template <typename EventType>
    static void post(const EventType &event)
    {
        const HandlerList &handlers = subscribers[typeid(EventType)];

        for (const auto &handler : handlers)
        {
            handler->exec(event);
        }
    }

    template <class T, class EventType>
    static void subscribe(T *instance, void (T::*member_function)(const EventType &))
    {
        subscribers[typeid(EventType)].emplace_back(new MemberFunctionHandler<T, EventType>(instance, member_function));
    }

    template <class EventType>
    static void subscribe(void (*function)(const EventType &))
    {
        subscribers[typeid(EventType)].emplace_back(new FunctionHandler<EventType>(function));
    }

private:
    static std::map<std::type_index, HandlerList> subscribers;
};
inline std::map<std::type_index, Dispatcher::HandlerList> Dispatcher::subscribers;