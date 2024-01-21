#pragma once

#include <functional>

template<class T>
class Handle
{
    std::function<void()> _destructor;
    T* _resource;

public:
    Handle(T* resource, std::function<void()> handleDestructor) : _resource(resource), _destructor(handleDestructor) {}
    ~Handle() { _destructor(); }
    T* operator->()
    {
        return _resource;
    }
    bool Valid()
    {
        return _resource != nullptr;
    }

    Handle(const Handle&) = delete;
    Handle(Handle&&) = delete;
    Handle& operator=(const Handle&) = delete;
    Handle& operator=(Handle&&) = delete;
};