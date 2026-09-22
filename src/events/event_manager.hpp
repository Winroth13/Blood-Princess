#pragma once
#include "event.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <cstdint>
#include <algorithm>

using SubscriptionId = uint64_t;

class EventManager
{
public:

    template <typename TEvent>
    SubscriptionId Subscribe(std::function<void(const TEvent&)> callback)
    {
        static_assert(std::is_base_of_v<Event, TEvent>,
            "TEvent must derive from Event");

        const SubscriptionId id = this->nextId++;

        auto& bucket = this->subscribers[std::type_index(typeid(TEvent))];

        bucket.push_back({
            id,
            [cb = std::move(callback)](const Event& e) {
                cb(static_cast<const TEvent&>(e));
            }
            });

        return id;
    }

    void Unsubscribe(SubscriptionId id)
    {
        for (auto& [type, bucket] : this->subscribers)
        {
            bucket.erase(
                std::remove_if(bucket.begin(), bucket.end(),
                    [id](const Subscriber& s) { return s.id == id; }),
                bucket.end()
            );
        }
    }

    template <typename TEvent>
    void Publish(const TEvent& event)
    {
        static_assert(std::is_base_of_v<Event, TEvent>,
            "TEvent must derive from Event");

        auto it = this->subscribers.find(std::type_index(typeid(TEvent)));
        if (it == this->subscribers.end())
            return;

        auto snapshot = it->second; 
        for (auto& sub : snapshot)
            sub.invoke(event);
    }

    void Clear() { this->subscribers.clear(); }

private:
    struct Subscriber
    {
        SubscriptionId id;
        std::function<void(const Event&)> invoke;
    };

    std::unordered_map<std::type_index, std::vector<Subscriber>> subscribers;
    SubscriptionId nextId = 1;
};