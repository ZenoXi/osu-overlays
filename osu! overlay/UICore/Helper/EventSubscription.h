#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <mutex>

template<class _Ret, class... _Types>
class _EventEmitter;

template<class _Ret, class... _Types>
class EventSubscription
{
    template<class _Ret, class... _Types>
    friend class _EventEmitter;

    typedef EventSubscription<_Ret, _Types...> _ThisType;

    uint64_t _subId = 0;
    std::weak_ptr<_EventEmitter<_Ret, _Types...>> _emitterRef;

    EventSubscription(uint64_t subId, std::weak_ptr<_EventEmitter<_Ret, _Types...>> emitterRef) : _subId(subId), _emitterRef(emitterRef)
    {}
    void _ReleaseSubscription();

public:
    EventSubscription() {}
    EventSubscription(_ThisType&& other)
    {
        _subId = other._subId;
        _emitterRef = other._emitterRef;
        // Setting _subId to 0 is enough to make this object functionally useless
        other._subId = 0;
    }
    _ThisType& operator=(_ThisType&& other)
    {
        _ReleaseSubscription();
        _subId = other._subId;
        _emitterRef = other._emitterRef;
        other._subId = 0;
        return *this;
    }
    EventSubscription(const _ThisType&) = delete;
    _ThisType& operator=(const _ThisType&) = delete;
    ~EventSubscription();

    // Detach subscription from event emitter - subscription won't unsubscribe on destruction
    // Useful in reducing variable count when certain the event emitter will be destroyed first
    // Can lead to bad memory access otherwise
    void Detach()
    {
        _subId = 0;
    }

    void Unsubscribe()
    {
        _ReleaseSubscription();
    }
};

template<class _Ret, class... _Types>
class AsyncEventSubscription
{
    template<class _Ret, class... _Types>
    friend class _EventEmitter;

    typedef AsyncEventSubscription<_Ret, _Types...> _ThisType;

    uint64_t _subId;
    std::weak_ptr<_EventEmitter<_Ret, _Types...>> _emitterRef;
    std::function<void(_Types...)> _syncHandlerFunc;
    std::vector<std::tuple<_Types...>> _pendingCalls;
    std::mutex _m_handlerLock;

    AsyncEventSubscription(uint64_t subId, std::weak_ptr<_EventEmitter<_Ret, _Types...>> emitterRef, std::function<void(_Types...)> syncHandlerFunc)
        : _subId(subId), _emitterRef(emitterRef), _syncHandlerFunc(syncHandlerFunc)
    {}
    void _ReleaseAsyncSubscription();

    void _Invoke(_Types... args)
    {
        std::lock_guard<std::mutex> lock(_m_handlerLock);
        if (_syncHandlerFunc)
            _syncHandlerFunc(args...);
        else
            _pendingCalls.push_back(std::make_tuple(args...));
    }

public:
    AsyncEventSubscription(const _ThisType&) = delete;
    AsyncEventSubscription(_ThisType&&) = delete;
    _ThisType& operator=(const _ThisType&) = delete;
    _ThisType& operator=(_ThisType&&) = delete;
    ~AsyncEventSubscription();

    void HandlePendingEvents(std::function<void(_Types...)> handlerFunc)
    {
        std::lock_guard<std::mutex> lock(_m_handlerLock);
        for (auto& eventArgs : _pendingCalls)
        {
            std::apply(handlerFunc, eventArgs);
        }
        _pendingCalls.clear();
    }

    void ResetSynchronousHandler(std::function<void(_Types...)> syncHandlerFunc)
    {
        std::lock_guard<std::mutex> lock(_m_handlerLock);
        _syncHandlerFunc = syncHandlerFunc;
    }

    void Unsubscribe()
    {
        _ReleaseAsyncSubscription();
    }
};

#include "EventEmitter.h"

template<class _Ret, class... _Types>
EventSubscription<_Ret, _Types...>::~EventSubscription()
{
    _ReleaseSubscription();
}

template<class _Ret, class... _Types>
void EventSubscription<_Ret, _Types...>::_ReleaseSubscription()
{
    if (_subId == 0)
        return;

    std::shared_ptr<_EventEmitter<_Ret, _Types...>> emitter = _emitterRef.lock();
    if (emitter)
    {
        emitter->_Unsubscribe(_subId);
    }
}

template<class _Ret, class... _Types>
AsyncEventSubscription<_Ret, _Types...>::~AsyncEventSubscription()
{
    _ReleaseAsyncSubscription();
}

template<class _Ret, class... _Types>
void AsyncEventSubscription<_Ret, _Types...>::_ReleaseAsyncSubscription()
{
    std::shared_ptr<_EventEmitter<_Ret, _Types...>> emitter = _emitterRef.lock();
    if (emitter)
    {
        emitter->_UnsubscribeAsync(_subId);
    }
}