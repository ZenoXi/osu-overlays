#pragma once

#include "Helper/ConfigValue.h"

struct CursorTrailConfig
{
    inline static const ConfigValue<bool> FULL_MONITOR = ConfigValue<bool>(L"cursorTrail.fullMonitor", true);
    inline static const ConfigValue<int> INITIAL_WIDTH = ConfigValue<int>(L"cursorTrail.width", 1920);
    inline static const ConfigValue<int> INITIAL_HEIGHT = ConfigValue<int>(L"cursorTrail.height", 1080);
    inline static const ConfigValue<int> INITIAL_X_OFFSET = ConfigValue<int>(L"cursorTrail.xOffset", 0);
    inline static const ConfigValue<int> INITIAL_Y_OFFSET = ConfigValue<int>(L"cursorTrail.yOffset", 0);
    inline static const ConfigValue<int> HEAD_COLOR = ConfigValue<int>(L"cursorTrail.headColor", 0xFFFFFFFF);
    inline static const ConfigValue<int> TRAIL_WIDTH = ConfigValue<int>(L"cursorTrail.trailWidth", 36);
    inline static const ConfigValue<int> HEAD_SIZE = ConfigValue<int>(L"cursorTrail.headSize", 34);
    inline static const ConfigValue<float> TRAIL_EDGE_WIDTH = ConfigValue<float>(L"cursorTrail.trailEdgeWidth", 1.5f);
    inline static const ConfigValue<float> HEAD_EDGE_WIDTH = ConfigValue<float>(L"cursorTrail.headEdgeWidth", 3.0f);
    inline static const ConfigValue<int> TRAIL_RESOLUTION = ConfigValue<int>(L"cursorTrail.trailResolution", 16);
    inline static const ConfigValue<int> TRAIL_LIFETIME = ConfigValue<int>(L"cursorTrail.trailLifetime", 300);
    inline static const ConfigValue<int> COLOR_CYCLE_DURATION = ConfigValue<int>(L"cursorTrail.colorCycleDuration", 5000);
    inline static const ConfigValue<int> ICON_SPIN_DURATION = ConfigValue<int>(L"cursorTrail.iconSpinDuration", 10000);
    inline static const ConfigValue<std::wstring> PALETTE = ConfigValue<std::wstring>(L"cursorTrail.palette", L"-65536,0.000|-256,0.167|-16711936,0.333|-16711681,0.500|-16776961,0.667|-65281,0.833|-65536,1.000");

    inline static const std::wstring OVERLAY_WINDOW_NAME = L"cursorTrailOverlay";
    inline static const std::wstring HEAD_COLOR_SELECTOR_WINDOW_NAME = L"cursorTrailHeadColorSelector";
};
