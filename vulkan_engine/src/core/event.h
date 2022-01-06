#pragma once

#include <map>
#include <typeindex>
#include <vector>
#include <memory>
#include <GLFW/glfw3.h>

// Different core events
class Event
{
public:
    virtual ~Event() {}
};

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(int width, int height, GLFWwindow* window)
        : width{ width }, height{ height }, window{ window } {}

    const int width, height;
    const GLFWwindow* window;
};

class WindowCloseEvent : public Event
{
public:
    WindowCloseEvent(GLFWwindow* window) {}

    const GLFWwindow* window;
};

class KeyPressEvent : public Event
{
public:
    KeyPressEvent(uint16_t keycode, unsigned int repeat_count, GLFWwindow* window)
        : keycode{keycode}, repeat_count{repeat_count}, window{ window } {}

    const int keycode;
    const unsigned int repeat_count;
    const GLFWwindow* window;
};

class KeyReleaseEvent : public Event
{
public:
    KeyReleaseEvent(int keycode, GLFWwindow* window)
        : keycode{keycode}, window{ window } {}

    const int keycode;
    const GLFWwindow* window;
};

class MousePressEvent : public Event
{
public:
    MousePressEvent(int button, GLFWwindow* window)
        : button{button}, window{ window } {}

    const int button;
    const GLFWwindow* window;
};

class MouseReleaseEvent : public Event
{
public:
    MouseReleaseEvent(int button, GLFWwindow* window)
        : button{button}, window{ window } {}

    const int button;
    const GLFWwindow* window;
};

class MouseMoveEvent : public Event
{
public:
    MouseMoveEvent(double x, double y, GLFWwindow* window)
        : x{x}, y{y}, window{ window } {}

    const double x, y;
    const GLFWwindow* window;
};

class MouseDragEvent : public Event
{
public:
    MouseDragEvent(int button, double x, double y, GLFWwindow* window)
        : button{button}, x{x}, y{y}, window{ window } {}

    const int button;
    const double x, y;
    const GLFWwindow* window;
};

class MouseScrollEvent : public Event
{
public:
    MouseScrollEvent(double x, double y, GLFWwindow* window)
        : x{ x }, y{ y }, window{ window } {}

    const double x, y;
    const GLFWwindow* window;
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