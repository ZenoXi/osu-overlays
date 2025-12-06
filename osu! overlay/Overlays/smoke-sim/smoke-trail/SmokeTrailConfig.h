#pragma once

#include "Helper/ConfigValue.h"

struct SmokeTrailConfig
{
    inline static const ConfigValue<std::wstring> LAYOUT_STRING = ConfigValue<std::wstring>(L"smokeTrail.layoutString", L"1|1920|1080|0|0|0|0");
    inline static const ConfigValue<int> CELL_SIZE = ConfigValue<int>(L"smokeTrail.cellSize", 4);
    inline static const ConfigValue<int> THREAD_COUNT = ConfigValue<int>(L"smokeTrail.threadCount", 4);
    inline static const ConfigValue<int> TRAIL_COLOR = ConfigValue<int>(L"smokeTrail.trailColor", 0xFF888888);
    inline static const ConfigValue<int> TRAIL_WIDTH = ConfigValue<int>(L"smokeTrail.trailWidth", 10);
    inline static const ConfigValue<int> TRAIL_EDGE_FADE_RANGE = ConfigValue<int>(L"smokeTrail.trailEdgeFadeRange", 8);
    inline static const ConfigValue<float> TRAIL_DENSITY = ConfigValue<float>(L"smokeTrail.trailDensity", 0.7f);
    inline static const ConfigValue<int> TRAIL_WIND_WIDTH = ConfigValue<int>(L"smokeTrail.trailWindWidth", 10);
    inline static const ConfigValue<float> TRAIL_WIND_SPEED = ConfigValue<float>(L"smokeTrail.trailWindSpeed", 0.2f);
    inline static const ConfigValue<float> CURSOR_TEMP = ConfigValue<float>(L"smokeTrail.cursorTemp", 0.4f);
    inline static const ConfigValue<float> VELOCITY_DIFFUSION = ConfigValue<float>(L"smokeTrail.velocityDiffusion", 0.0f);
    inline static const ConfigValue<float> DENSITY_DIFFUSION = ConfigValue<float>(L"smokeTrail.densityDiffusion", 0.0f);
    inline static const ConfigValue<float> TEMPERATURE_DIFFUSION = ConfigValue<float>(L"smokeTrail.temperatureDiffusion", 6.0f);
    inline static const ConfigValue<float> DENSITY_REDUCTION_RATE = ConfigValue<float>(L"smokeTrail.densityReductionRate", 0.15f);
    inline static const ConfigValue<float> TEMPERATURE_REDUCTION_RATE = ConfigValue<float>(L"smokeTrail.temperatureReductionRate", 0.05f);
};