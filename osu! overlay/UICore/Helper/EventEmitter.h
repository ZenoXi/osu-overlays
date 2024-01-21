#pragma once

#include "EventSubscription.h"

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

// SELF NOTES:
//
// EventEmitter<...> onSmth;
// EventSubscription sub = onSmth.Subscribe([]() { do; });
// sub.ExecutePending();
// 
// When a subscription is destroyed, it automatically unsubscribes from the EventEmitter
// When an EventEmitter is destroyed, the subscription does not automatically call unsubscribe
//
// EventEmitter has two types of subscriptions: a regular subscription and an async one
// When invoking the event handlers, regular subscriptions are processed sychronously and their return values can be used for stop conditions
// Async subscriptions copy the event arguments and store them in the subscription object until the subscription owner handles them using HandlePending()
// Since Async subscriptions are likely created from different threads, synchronization on Subscribe and Unsubscribe methods is required
// A default behavior of no synchronization could be used with the additional MULTITHREADED flag enabling it
// Multithreaded emitter needs to have a check for thread id when subscribing or unsubscribing to allow handlers themselves to perform those actions while handling an event
// 



// A thread safe event class with subscription lifecycle tied to object lifecycles
// A subscription to an event is held for as long as the object is alive. Once the subscription object is destroyed, the subscription is released
// If an EventEmitter is destroyed, its subscriptions will not unsubscribe on destruction to prevent invalid memory access
// 
// An EventEmitter can be safely subscribed/unsubscribed from the invocation code itself
// Subscriptions that are added while the event is being invoked are not themselves invoked in the cycle
// Unsubscribing mid invocation takes effect immediatelly and the subscription will not be invoked
// 
// While invoking and handling events is thread safe in multithreaded mode, destroying the EventEmitter
// object is not, so access to it should be protected in synchronized blocks
// Note that unsubscribing is always thread safe in multithreaded mode
//



enum class EventEmitterThreadMode
{
    SINGLETHREADED,
    MULTITHREADED
};

template<class _Ret, class... _Types>
class EventSubscription;

template<class _Ret, class... _Types>
class AsyncEventSubscription;

// This class should not be used directly, instead use EventEmitter
template<class _Ret, class... _Types>
class _EventEmitter : public std::enable_shared_from_this<_EventEmitter<_Ret, _Types...>>
{
    template<class _Ret, class... _Types>
    friend class EventSubscription;

    template<class _Ret, class... _Types>
    friend class AsyncEventSubscription;

    typedef _EventEmitter<_Ret, _Types...> _EmitterType;
    typedef EventSubscription<_Ret, _Types...> _SubscriptionType;
    typedef AsyncEventSubscription<_Ret, _Types...> _AsyncSubscriptionType;

public:
    _EventEmitter(EventEmitterThreadMode mode = EventEmitterThreadMode::SINGLETHREADED)
    {
        _threadMode = mode;
    }

    _SubscriptionType Subscribe(const std::function<_Ret(_Types...)>& handler);

    // Async subscription needs to be a pointer because the event object holds a reference to it

    std::unique_ptr<_AsyncSubscriptionType> SubscribeAsync(const std::function<void(_Types...)>& syncHandler = nullptr);

    void InvokeAll(_Types... args)
    {
        std::unique_lock<std::mutex> lock(_m_handlers, std::defer_lock);
        if (_threadMode == EventEmitterThreadMode::MULTITHREADED)
            lock.lock();

        _invoking.store(true);
        _invokingThreadId.store(std::this_thread::get_id());

        for (auto& handler : _handlers)
        {
            if (handler.handlerFunc && !handler.removed)
                handler.handlerFunc(args...);
        }
        for (auto handler : _asyncHandlers)
        {
            if (handler)
                handler->_Invoke(args...);
        }
        _ProcessPendingHandlerChanges();

        _invoking.store(false);
        _invokingThreadId.store(std::thread::id());
    }

    // Invokes all handlers and stops when stopCondition returns true for a handler return value
    // Function won't compile if event return type is void
    // Returns true if stop condition was triggered
    bool InvokeAll(std::function<bool(_Ret)> stopCondition, _Types... args)
    {
        std::unique_lock<std::mutex> lock(_m_handlers, std::defer_lock);
        if (_threadMode == EventEmitterThreadMode::MULTITHREADED)
            lock.lock();

        _invoking.store(true);
        _invokingThreadId.store(std::this_thread::get_id());

        bool stopConditionTriggered = false;
        for (auto& handler : _handlers)
        {
            if (!handler.handlerFunc || handler.removed)
                continue;

            // Compilation error on this line means the function is called on an event with void return type
            _Ret result = handler.handlerFunc(args...);
            if (stopCondition(result))
            {
                stopConditionTriggered = true;
                break;
            }
        }
        if (!stopConditionTriggered)
        {
            for (auto handler : _asyncHandlers)
            {
                if (handler)
                    handler->_Invoke(args...);
            }
        }
        _ProcessPendingHandlerChanges();

        _invoking.store(false);
        _invokingThreadId.store(std::thread::id());

        return stopConditionTriggered;
    }

private:
    struct _Handler
    {
        std::function<_Ret(_Types...)> handlerFunc;
        uint64_t handlerId;
        bool removed = false;
    };
    std::vector<_Handler> _handlers;
    std::vector<_AsyncSubscriptionType*> _asyncHandlers;

    EventEmitterThreadMode _threadMode;
    std::mutex _m_handlers;

    std::atomic<bool> _invoking = false;
    std::atomic<std::thread::id> _invokingThreadId;
    std::vector<_Handler> _addedHandlers;
    std::vector<_AsyncSubscriptionType*> _addedAsyncHandlers;

    void _Unsubscribe(uint64_t handlerId);

    void _UnsubscribeAsync(uint64_t handlerId);

    void _ProcessPendingHandlerChanges()
    {
        // Clear removed handlers
        _handlers.erase(
            std::remove_if(_handlers.begin(), _handlers.end(), [](const _Handler& data) { return data.removed; }),
            _handlers.end()
        );
        _asyncHandlers.erase(
            std::remove(_asyncHandlers.begin(), _asyncHandlers.end(), nullptr),
            _asyncHandlers.end()
        );

        // Add pending handlers
        for (auto& handler : _addedHandlers)
            _handlers.push_back(handler);
        _addedHandlers.clear();
        for (auto asyncHandler : _addedAsyncHandlers)
            _asyncHandlers.push_back(asyncHandler);
        _addedAsyncHandlers.clear();
    }

    // Id of 0 is reserved for moved subscriptions
    std::atomic<uint64_t> _ID_COUNTER = 1;
    uint64_t _GenerateId()
    {
        return _ID_COUNTER.fetch_add(1);
    }
};

#include "EventSubscription.h"

template<class _Ret, class... _Types>
EventSubscription<_Ret, _Types...> _EventEmitter<_Ret, _Types...>::Subscribe(const std::function<_Ret(_Types...)>& handler)
{
    std::unique_lock<std::mutex> lock(_m_handlers, std::defer_lock);
    if (_threadMode == EventEmitterThreadMode::MULTITHREADED && std::this_thread::get_id() != _invokingThreadId.load())
        lock.lock();

    uint64_t handlerId = _GenerateId();
    if (_invoking.load())
        _addedHandlers.push_back({ handler, handlerId });
    else
        _handlers.push_back({ handler, handlerId });
    return _SubscriptionType(handlerId, this->weak_from_this());
}

template<class _Ret, class... _Types>
void _EventEmitter<_Ret, _Types...>::_Unsubscribe(uint64_t handlerId)
{
    std::unique_lock<std::mutex> lock(_m_handlers, std::defer_lock);
    if (_threadMode == EventEmitterThreadMode::MULTITHREADED && std::this_thread::get_id() != _invokingThreadId.load())
        lock.lock();

    for (auto it = _handlers.begin(); it != _handlers.end(); it++)
    {
        if (it->handlerId == handlerId)
        {
            if (_invoking.load())
                it->removed = true;
            else
                _handlers.erase(it);
            return;
        }
    }
}

template<class _Ret, class... _Types>
std::unique_ptr<AsyncEventSubscription<_Ret, _Types...>> _EventEmitter<_Ret, _Types...>::SubscribeAsync(const std::function<void(_Types...)>& syncHandler)
{
    std::unique_lock<std::mutex> lock(_m_handlers, std::defer_lock);
    if (_threadMode == EventEmitterThreadMode::MULTITHREADED && std::this_thread::get_id() != _invokingThreadId.load())
        lock.lock();

    uint64_t handlerId = _GenerateId();
    // can't use make_unique since AsyncEventSubscription constructor is private
    auto subscription = std::unique_ptr<_AsyncSubscriptionType>(new _AsyncSubscriptionType(handlerId, this->weak_from_this(), syncHandler));
    if (_invoking.load())
        _addedAsyncHandlers.push_back(subscription.get());
    else
        _asyncHandlers.push_back(subscription.get());
    return subscription;
}

template<class _Ret, class... _Types>
void _EventEmitter<_Ret, _Types...>::_UnsubscribeAsync(uint64_t handlerId)
{
    std::unique_lock<std::mutex> lock(_m_handlers, std::defer_lock);
    if (_threadMode == EventEmitterThreadMode::MULTITHREADED && std::this_thread::get_id() != _invokingThreadId.load())
        lock.lock();

    for (auto it = _asyncHandlers.begin(); it != _asyncHandlers.end(); it++)
    {
        if ((*it)->_subId == handlerId)
        {
            if (_invoking.load())
                *it = nullptr;
            else
                _asyncHandlers.erase(it);
            return;
        }
    }
}


// shared_ptr wrapper class for _EventEmitter
template<class _Ret, class... _Types>
class EventEmitter
{
public:
    EventEmitter(EventEmitterThreadMode mode = EventEmitterThreadMode::SINGLETHREADED)
    {
        _emitter = std::make_shared<_EventEmitter<_Ret, _Types...>>(mode);
    }

    ~EventEmitter()
    {
        _emitter.reset();
    }

    std::shared_ptr<_EventEmitter<_Ret, _Types...>> operator->()
    {
        return _emitter;
    }

private:
    std::shared_ptr<_EventEmitter<_Ret, _Types...>> _emitter;
};