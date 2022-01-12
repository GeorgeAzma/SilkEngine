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
        : width{width}, height{height}, window{window} {}

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
    KeyPressEvent(uint16_t key, unsigned int repeat_count, GLFWwindow* window)
        : key{key}, repeat_count{repeat_count}, window{window} {}

    const int key;
    const unsigned int repeat_count;
    const GLFWwindow* window;
};

class KeyReleaseEvent : public Event
{
public:
    KeyReleaseEvent(int key, GLFWwindow* window)
        : key{key}, window{window} {}

    const int key;
    const GLFWwindow* window;
};

class MousePressEvent : public Event
{
public:
    MousePressEvent(int button, GLFWwindow* window)
        : button{button}, window{window} {}

    const int button;
    const GLFWwindow* window;
};

class MouseReleaseEvent : public Event
{
public:
    MouseReleaseEvent(int button, GLFWwindow* window)
        : button{button}, window{window} {}

    const int button;
    const GLFWwindow* window;
};

class MouseMoveEvent : public Event
{
public:
    MouseMoveEvent(double x, double y, GLFWwindow* window)
        : x{x}, y{y}, window{window} {}

    const double x, y;
    const GLFWwindow* window;
};

class MouseDragEvent : public Event
{
public:
    MouseDragEvent(int button, double x, double y, GLFWwindow* window)
        : button{button}, x{x}, y{y}, window{window} {}

    const int button;
    const double x, y;
    const GLFWwindow* window;
};

class MouseScrollEvent : public Event
{
public:
    MouseScrollEvent(double x, double y, GLFWwindow* window)
        : x{ x }, y{ y }, window{window} {}

    const double x, y;
    const GLFWwindow* window;
};









// Dispatching and such code
class HandlerFunctionBase
{
public:
    void exec(const Event& event) const
    {
        call(event);
    }

    virtual bool operator==(const HandlerFunctionBase& other) const = 0;
    virtual bool operator!=(const HandlerFunctionBase& other) const = 0;

private:
    virtual void call(const Event& event) const = 0;
};

template <typename T>
class HandlerFunctionComparator : public HandlerFunctionBase
{
public:
    virtual bool operator==(const HandlerFunctionBase& other) const override
    {
        if (const T* self = dynamic_cast<const T*>(&other))
        {
            return ((T*)this)->operator==(*self);
        }
        return false;
    }

    virtual bool operator!=(const HandlerFunctionBase& other) const override
    {
        if (const T* self = dynamic_cast<const T*>(&other))
        {
            return ((T*)this)->operator!=(*self);
        }
        return true;
    }
};

template <class T, class EventType>
class MemberFunctionHandler : public HandlerFunctionComparator<MemberFunctionHandler<T, EventType>>
{
public:
    typedef void (T::* MemberFunction)(const EventType&);

    MemberFunctionHandler(T* instance, MemberFunction member_function)
        : instance{ instance }, member_function{ member_function } {}

    void call(const Event& event) const
    {
        if (instance && member_function)
        {
            (instance->*member_function)(static_cast<const EventType&>(event));
        }
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
        : function{ function } {};

    void call(const Event& event) const
    {
        if (function)
        {
            (*function)(static_cast<const EventType&>(event));
        }
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
    typedef std::vector<std::shared_ptr<HandlerFunctionBase>> HandlerList;

public:
    template <typename EventType>
    static void post(const EventType& event)
    {
        const HandlerList& handlers = subscribers[typeid(EventType)];

        for (const auto& handler : handlers)
        {
            handler->exec(event);
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
        HandlerList& handlers = subscribers.at(typeid(EventType));
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
        HandlerList& handlers = subscribers.at(typeid(EventType));
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

private:
    static inline std::map<std::type_index, HandlerList> subscribers;
};