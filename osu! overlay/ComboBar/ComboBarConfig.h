#pragma once

#include "Helper/ConfigValue.h"

struct ComboBarConfig
{
    inline static const ConfigValue<int> INITIAL_WIDTH = ConfigValue<int>(L"comboBar.width", 600);
    inline static const ConfigValue<int> INITIAL_HEIGHT = ConfigValue<int>(L"comboBar.height", 150);
    inline static const ConfigValue<int> INITIAL_X_OFFSET = ConfigValue<int>(L"comboBar.xOffset", 120);
    inline static const ConfigValue<int> INITIAL_Y_OFFSET = ConfigValue<int>(L"comboBar.yOffset", 920);
    inline static const ConfigValue<int> BAR_COLOR = ConfigValue<int>(L"comboBar.barColor", 0xFFFFF0AA);
    inline static const ConfigValue<int> TEXT_COLOR = ConfigValue<int>(L"comboBar.textColor", 0xFFAAAAAA);
    inline static const ConfigValue<int> GRID_COLOR = ConfigValue<int>(L"comboBar.gridColor", 0xFF444444);

    inline static const std::wstring OVERLAY_WINDOW_NAME = L"comboBarOverlay";
    inline static const std::wstring BAR_COLOR_SELECTOR_WINDOW_NAME = L"comboBarBarColorSelector";
    inline static const std::wstring TEXT_COLOR_SELECTOR_WINDOW_NAME = L"comboBarTextColorSelector";
    inline static const std::wstring GRID_COLOR_SELECTOR_WINDOW_NAME = L"comboBarGridColorSelector";
};