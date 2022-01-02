#pragma once

class Event
{
public:
    virtual ~Event() {}
};

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
std::map<std::type_index, Dispatcher::HandlerList> Dispatcher::subscribers;