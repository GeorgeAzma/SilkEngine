#pragma once

#include <map>
#include <typeindex>
#include <vector>
#include <memory>

//EVENT
class Event
{
public:
    virtual ~Event() {}
};

//WINDOW EVENTS
class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(int width, int height)
        : width(width), height(height) {}

    const int width, height;
};

class WindowCloseEvent : public Event
{
public:
    WindowCloseEvent() {}
};

class WindowMoveEvent : public Event
{
public:
    WindowMoveEvent(int x, int y)
        : x(x), y(y) {}

    const int x, y;
};

//KEY EVENTS
class KeyPressEvent : public Event
{
public:
    KeyPressEvent(uint16_t key, unsigned int repeat_count)
        : key(key), repeat_count(repeat_count) {}

    const int key;
    const unsigned int repeat_count;
 };

class KeyReleaseEvent : public Event
{
public:
    KeyReleaseEvent(int key)
        : key(key) {}

    const int key;
 };

//MOUSE EVENTS
class MousePressEvent : public Event
{
public:
    MousePressEvent(int button)
        : button(button) {}

    const int button;
 };

class MouseReleaseEvent : public Event
{
public:
    MouseReleaseEvent(int button)
        : button(button) {}

    const int button;
 };

class MouseMoveEvent : public Event
{
public:
    MouseMoveEvent(double x, double y)
        : x(x), y(y) {}

    const double x, y;
 };

class MouseDragEvent : public Event
{
public:
    MouseDragEvent(int button, double x, double y)
        : button(button), x(x), y(y) {}

    const int button;
    const double x, y;
 };

class MouseScrollEvent : public Event
{
public:
    MouseScrollEvent(double x, double y)
        : x(x), y(y) {}

    const double x, y;
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
        : instance(instance), member_function(member_function) {}

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
        : function(function) {};

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