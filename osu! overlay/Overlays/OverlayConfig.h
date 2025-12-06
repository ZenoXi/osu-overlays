#pragma once

#include "Helper/ConfigValue.h"

struct OverlayConfig
{
    inline static const ConfigValue<int> MONITOR_INDEX = ConfigValue<int>(L"overlay.monitorIndex", 0);
    inline static const ConfigValue<bool> FILL_MONITOR = ConfigValue<bool>(L"overlay.fillMonitor", true);
    inline static const ConfigValue<int> X_OFFSET = ConfigValue<int>(L"overlay.xOffset", 0);
    inline static const ConfigValue<int> Y_OFFSET = ConfigValue<int>(L"overlay.yOffset", 0);
    inline static const ConfigValue<int> WIDTH = ConfigValue<int>(L"overlay.width", 1920);
    inline static const ConfigValue<int> HEIGHT = ConfigValue<int>(L"overlay.height", 1080);
    inline static const ConfigValue<bool> FIT_TO_GAME_WINDOW = ConfigValue<bool>(L"overlay.fitToGameWindow", true);
    inline static const std::wstring OVERLAY_WINDOW_NAME = L"overlay";
};