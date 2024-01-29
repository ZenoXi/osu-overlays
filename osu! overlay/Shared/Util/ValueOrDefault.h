#pragma once

namespace zutil
{
    template<typename T>
    class ValueOrDefault
    {
    public:
        ValueOrDefault(T value) : _value(value), _default(value) {}
        ValueOrDefault(T value, T defaultValue) : _value(value), _default(defaultValue) {}
        T& Get() { return _value; }
        T Default() { return _default; }
    private:
        T _value;
        T _default;
    };
}