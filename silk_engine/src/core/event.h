#pragma once

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

class DragAndDropEvent : public Event
{
public:
    DragAndDropEvent(const std::vector<const char*>& paths)
        : paths(paths) {}

private:
    std::vector<const char*> paths;
};

class WindowFullscreenEvent : public Event
{
public:
    WindowFullscreenEvent(bool fullscreen)
        : fullscreen(fullscreen) {}

    const bool fullscreen;
};

class CharacterWriteEvent : public Event
{
public:
    CharacterWriteEvent(char character)
        : character(character) {}

    const char character;
};

class JoystickEvent : public Event
{
public:
    JoystickEvent(int id, bool connected)
        : id(id), connected(connected) {}

    const int id;
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
        SK_ASSERT(instance && member_function, 
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
    static void post(const EventType& event)
    {
        auto it = subscribers.find(typeid(EventType));
        if (it != subscribers.cend())
        {
            const auto& handlers = it->second;
            for (const auto& handler : handlers)
                (*handler)(event);
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