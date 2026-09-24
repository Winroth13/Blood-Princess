#pragma once
#include <functional>
#include <vector>
#include <cstdint>
#include <algorithm>

using ConnectionId = uint64_t;

template <typename... Args>
class Signal
{
public:
    using Callback = std::function<void(Args...)>;

private:
    struct Slot
    {
        ConnectionId id;
        const void* receiver;
        Callback callback;
    };

    std::vector<Slot> slots;
    ConnectionId nextId = 1;

public:
    Signal() = default;
    ~Signal() = default;

    
    ConnectionId Connect(Callback callback)
    {
        const ConnectionId id = this->nextId++;
        this->slots.push_back({ id, nullptr, std::move(callback) });
        return id;
    }

    
    template <typename T>
    ConnectionId Connect(T* instance, void (T::* method)(Args...))
    {
        const ConnectionId id = this->nextId++;
        this->slots.push_back({
            id,
            static_cast<const void*>(instance),
            [instance, method](Args... args)
            {
                (instance->*method)(args...);
            }
            });
        return id;
    }

   
    template <typename T>
    ConnectionId Connect(const T* instance, void (T::* method)(Args...) const)
    {
        const ConnectionId id = this->nextId++;
        this->slots.push_back({
            id,
            static_cast<const void*>(instance),
            [instance, method](Args... args)
            {
                (instance->*method)(args...);
            }
            });
        return id;
    }

   
    void Disconnect(ConnectionId id)
    {
        this->slots.erase(
            std::remove_if(this->slots.begin(), this->slots.end(),
                [id](const Slot& s)
                {
                    return s.id == id;
                }),
            this->slots.end()
        );
    }

    
    void DisconnectObject(const void* instance)
    {
        if (instance == nullptr)
        {
            return;
        }

        this->slots.erase(
            std::remove_if(this->slots.begin(), this->slots.end(),
                [instance](const Slot& s)
                {
                    return s.receiver == instance;
                }),
            this->slots.end()
        );
    }

   
    void Emit(Args... args)
    {
        auto snapshot = this->slots;
        for (auto& slot : snapshot)
        {
            if (slot.callback)
            {
                slot.callback(args...);
            }
        }
    }

    void Clear()
    {
        this->slots.clear();
    }

    bool IsEmpty() const
    {
        return this->slots.empty();
    }

    size_t Size() const
    {
        return this->slots.size();
    }
};