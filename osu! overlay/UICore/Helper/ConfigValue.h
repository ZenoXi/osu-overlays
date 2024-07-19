#pragma once

#include <string>

template<typename T>
class ConfigValue
{
public:
    std::wstring name;
    T defaultValue;

    ConfigValue(std::wstring name, T defaultValue) : name(name), defaultValue(defaultValue) {}
};