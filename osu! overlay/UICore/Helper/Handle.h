#pragma once

#include <functional>

template<class T>
class Handle
{
    std::function<void()> _destructor;
    T* _resource;

public:
    Handle() : _resource(nullptr), _destructor(nullptr) {}
    Handle(T* resource, std::function<void()> handleDestructor) : _resource(resource), _destructor(handleDestructor) {}
    Handle(Handle&& other)
    {
        _Swap(other);
    }
    Handle& operator=(Handle&& other)
    {
        if (this != &other)
        {
            _Destroy();
            _Swap(other);
        }
        return *this;
    }
    ~Handle()
    {
        _Destroy();
    }
    T* operator->()
    {
        return _resource;
    }
    bool Valid()
    {
        return _resource != nullptr;
    }
    void Release()
    {
        _Destroy();
        _resource = nullptr;
        _destructor = nullptr;
    }

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

private:
    void _Destroy()
    {
        if (_destructor)
            _destructor();
    }
    void _Swap(Handle& other)
    {
        _resource = other._resource;
        _destructor = std::move(other._destructor);
        other._resource = nullptr;
    }
};