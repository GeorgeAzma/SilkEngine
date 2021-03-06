#pragma once

//EVENT
struct Event
{
    virtual ~Event() = default;
};

//WINDOW EVENTS
struct WindowResizeEvent : public Event
{
    WindowResizeEvent(uint32_t width, uint32_t height)
        : width(width), height(height) {}

    const uint32_t width, height;
};

struct WindowCloseEvent : public Event
{
    WindowCloseEvent() {}
};

struct WindowMoveEvent : public Event
{
    WindowMoveEvent(int32_t x, int32_t y)
        : x(x), y(y) {}

    const int32_t x, y;
};

struct SwapchainRecreate : public Event
{
    SwapchainRecreate() {}
};

//KEY EVENTS
struct KeyPressEvent : public Event
{
    KeyPressEvent(uint16_t key, uint32_t repeat_count)
        : key(key), repeat_count(repeat_count) {}

    const int key;
    const unsigned int repeat_count;
 };

struct KeyReleaseEvent : public Event
{
    KeyReleaseEvent(uint16_t key)
        : key(key) {}

    const uint16_t key;
 };

//MOUSE EVENTS
struct MousePressEvent : public Event
{
    MousePressEvent(uint32_t button)
        : button(button) {}

    const uint32_t button;
 };

struct MouseReleaseEvent : public Event
{
    MouseReleaseEvent(uint32_t button)
        : button(button) {}

    const uint32_t button;
 };

struct MouseMoveEvent : public Event
{
    MouseMoveEvent(double x, double y)
        : x(x), y(y) {}

    const double x, y;
 };

struct MouseDragEvent : public Event
{
    MouseDragEvent(uint32_t button, double x, double y)
        : button(button), x(x), y(y) {}

    const uint32_t button;
    const double x, y;
 };

struct MouseScrollEvent : public Event
{
    MouseScrollEvent(double x, double y)
        : x(x), y(y) {}

    const double x, y;
 };

struct DragAndDropEvent : public Event
{
    DragAndDropEvent(const std::vector<const char*>& paths)
        : paths(paths) {}

    const std::vector<const char*> paths;
};

struct WindowFullscreenEvent : public Event
{
    WindowFullscreenEvent(bool fullscreen)
        : fullscreen(fullscreen) {}

    const bool fullscreen;
};

struct CharacterWriteEvent : public Event
{
    CharacterWriteEvent(char character)
        : character(character) {}

    const char character;
};

struct JoystickEvent : public Event
{
    JoystickEvent(uint32_t id, bool connected)
        : id(id), connected(connected) {}

    const uint32_t id;
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