#pragma once

#include "Helper/ConfigValue.h"

struct EnhancedSmokeConfig
{
    inline static const ConfigValue<std::wstring> LAYOUT_STRING = ConfigValue<std::wstring>(L"enhancedSmoke.layoutString", L"1|1920|1080|0|0|0|0");
    inline static const ConfigValue<int> CELL_SIZE = ConfigValue<int>(L"enhancedSmoke.cellSize", 4);
    inline static const ConfigValue<int> THREAD_COUNT = ConfigValue<int>(L"enhancedSmoke.threadCount", 4);
    inline static const ConfigValue<int> SMOKE_COLOR = ConfigValue<int>(L"enhancedSmoke.smokeColor", 0xFF888888);
    inline static const ConfigValue<int> BRUSH_WIDTH = ConfigValue<int>(L"enhancedSmoke.brushWidth", 14);
    inline static const ConfigValue<int> BRUSH_EDGE_FADE_RANGE = ConfigValue<int>(L"enhancedSmoke.brushEdgeFadeRange", 6);
    inline static const ConfigValue<float> SMOKE_DENSITY = ConfigValue<float>(L"enhancedSmoke.smokeDensity", 1.0f);
    inline static const ConfigValue<int> CURSOR_WIND_WIDTH = ConfigValue<int>(L"enhancedSmoke.cursorWindWidth", 14);
    inline static const ConfigValue<float> CURSOR_WIND_SPEED = ConfigValue<float>(L"enhancedSmoke.cursorWindSpeed", 0.2f);
    inline static const ConfigValue<int> SLOWDOWN_PERSISTENCE_DURATION = ConfigValue<int>(L"enhancedSmoke.slowdownPersistenceDuration", 250);
    inline static const ConfigValue<float> VELOCITY_DIFFUSION = ConfigValue<float>(L"enhancedSmoke.velocityDiffusion", 0.0f);
    inline static const ConfigValue<float> DENSITY_DIFFUSION = ConfigValue<float>(L"enhancedSmoke.densityDiffusion", 0.0f);
    inline static const ConfigValue<float> DENSITY_REDUCTION_RATE = ConfigValue<float>(L"enhancedSmoke.densityReductionRate", 0.02f);
    inline static const ConfigValue<int> SMOKE_KEY_CODE = ConfigValue<int>(L"enhancedSmoke.smokeKeyCode", 'C');
};
