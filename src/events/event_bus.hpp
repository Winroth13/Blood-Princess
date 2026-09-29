#pragma once
#include "events/signal.hpp"
#include <functional>

class EventBus
{
public:
    // Lambda / free function
    template<typename Event>
    static ConnectionId Subscribe(std::function<void(const Event&)> callback)
    {
        return GetSignal<Event>().Connect(std::move(callback));
    }

    // Member function
    template<typename Event, typename T>
    static ConnectionId Subscribe(T* instance, void (T::* method)(const Event&))
    {
        return GetSignal<Event>().Connect(instance, method);
    }

    template<typename Event>
    static void Unsubscribe(ConnectionId id)
    {
        GetSignal<Event>().Disconnect(id);
    }

    template<typename Event>
    static void Publish(const Event& event)
    {
        GetSignal<Event>().Emit(event);
    }

    // Disconnect every subscriber of a given event (useful on scene teardown)
    template<typename Event>
    static void Clear()
    {
        GetSignal<Event>().Clear();
    }

private:
    template<typename Event>
    static Signal<const Event&>& GetSignal()
    {
        static Signal<const Event&> signal;
        return signal;
    }
};