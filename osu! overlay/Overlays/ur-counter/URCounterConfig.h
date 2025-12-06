#pragma once

#include "Helper/ConfigValue.h"

struct URCounterConfig
{
    inline static const ConfigValue<std::wstring> LAYOUT_STRING = ConfigValue<std::wstring>(L"urCounter.layoutString", L"0|100|30|0|-50|7|7");
    inline static const ConfigValue<float> TEXT_SIZE = ConfigValue<float>(L"urCounter.textSize", 16.0f);
};