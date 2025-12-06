#pragma once

#include "Helper/ConfigValue.h"

struct ComboCounterConfig
{
    inline static const ConfigValue<std::wstring> LAYOUT_STRING = ConfigValue<std::wstring>(L"comboCounter.layoutString", L"0|600|150|120|0|6|6");
    inline static const ConfigValue<int> BAR_COLOR = ConfigValue<int>(L"comboCounter.barColor", 0xFFFFF0AA);
    inline static const ConfigValue<int> TEXT_COLOR = ConfigValue<int>(L"comboCounter.textColor", 0xFFAAAAAA);
    inline static const ConfigValue<int> GRID_COLOR = ConfigValue<int>(L"comboCounter.gridColor", 0xFF444444);
};
