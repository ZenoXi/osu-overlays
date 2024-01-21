#pragma once

#include <future>

template<typename T>
std::future<T> MakeCompletedFuture(T value)
{
    std::promise<T> promise;
    promise.set_value(value);
    return promise.get_future();
}