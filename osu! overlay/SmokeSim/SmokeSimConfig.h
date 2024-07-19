#pragma once

#include "Helper/ConfigValue.h"

struct SmokeSimConfig
{
    inline static const ConfigValue<int> CURSOR_TRAIL_CELL_SIZE = ConfigValue<int>(L"smokesim.cursortrail.cellSize", 4);
    inline static const ConfigValue<int> CURSOR_TRAIL_THREAD_COUNT = ConfigValue<int>(L"smokesim.cursortrail.threadCount", 4);
    inline static const ConfigValue<bool> CURSOR_TRAIL_FULL_MONITOR = ConfigValue<bool>(L"smokesim.cursortrail.fullMonitor", true);
    inline static const ConfigValue<int> CURSOR_TRAIL_WIDTH = ConfigValue<int>(L"smokesim.cursortrail.width", 1920);
    inline static const ConfigValue<int> CURSOR_TRAIL_HEIGHT = ConfigValue<int>(L"smokesim.cursortrail.height", 1080);
    inline static const ConfigValue<int> CURSOR_TRAIL_X_OFFSET = ConfigValue<int>(L"smokesim.cursortrail.xOffset", 0);
    inline static const ConfigValue<int> CURSOR_TRAIL_Y_OFFSET = ConfigValue<int>(L"smokesim.cursortrail.yOffset", 0);
    inline static const ConfigValue<int> ENHANCED_SMOKE_CELL_SIZE = ConfigValue<int>(L"smokesim.enhancedsmoke.cellSize", 4);
    inline static const ConfigValue<int> ENHANCED_SMOKE_THREAD_COUNT = ConfigValue<int>(L"smokesim.enhancedsmoke.threadCount", 4);
    inline static const ConfigValue<bool> ENHANCED_SMOKE_FULL_MONITOR = ConfigValue<bool>(L"smokesim.enhancedsmoke.fullMonitor", true);
    inline static const ConfigValue<int> ENHANCED_SMOKE_WIDTH = ConfigValue<int>(L"smokesim.enhancedsmoke.width", 1920);
    inline static const ConfigValue<int> ENHANCED_SMOKE_HEIGHT = ConfigValue<int>(L"smokesim.enhancedsmoke.height", 1080);
    inline static const ConfigValue<int> ENHANCED_SMOKE_X_OFFSET = ConfigValue<int>(L"smokesim.enhancedsmoke.xOffset", 0);
    inline static const ConfigValue<int> ENHANCED_SMOKE_Y_OFFSET = ConfigValue<int>(L"smokesim.enhancedsmoke.yOffset", 0);

    inline static const ConfigValue<int> TRAIL_COLOR = ConfigValue<int>(L"smokesim.cursortrail.trailColor", 0xFF888888);
    inline static const ConfigValue<int> TRAIL_WIDTH = ConfigValue<int>(L"smokesim.cursortrail.trailWidth", 10);
    inline static const ConfigValue<int> TRAIL_EDGE_FADE_RANGE = ConfigValue<int>(L"smokesim.cursortrail.trailEdgeFadeRange", 8);
    inline static const ConfigValue<float> TRAIL_DENSITY = ConfigValue<float>(L"smokesim.cursortrail.trailDensity", 0.7f);
    inline static const ConfigValue<int> TRAIL_WIND_WIDTH = ConfigValue<int>(L"smokesim.cursortrail.trailWindWidth", 10);
    inline static const ConfigValue<float> TRAIL_WIND_SPEED = ConfigValue<float>(L"smokesim.cursortrail.trailWindSpeed", 0.2f);
    inline static const ConfigValue<float> CURSOR_TEMP = ConfigValue<float>(L"smokesim.cursortrail.cursorTemp", 0.4f);
    inline static const ConfigValue<float> TRAIL_VELOCITY_DIFFUSION = ConfigValue<float>(L"smokesim.cursortrail.velocityDiffusion", 0.0f);
    inline static const ConfigValue<float> TRAIL_DENSITY_DIFFUSION = ConfigValue<float>(L"smokesim.cursortrail.densityDiffusion", 0.0f);
    inline static const ConfigValue<float> TRAIL_TEMPERATURE_DIFFUSION = ConfigValue<float>(L"smokesim.cursortrail.temperatureDiffusion", 6.0f);
    inline static const ConfigValue<float> TRAIL_DENSITY_REDUCTION_RATE = ConfigValue<float>(L"smokesim.cursortrail.densityReductionRate", 0.15f);
    inline static const ConfigValue<float> TRAIL_TEMPERATURE_REDUCTION_RATE = ConfigValue<float>(L"smokesim.cursortrail.temperatureReductionRate", 0.05f);
    inline static const ConfigValue<int> SMOKE_COLOR = ConfigValue<int>(L"smokesim.enhancedsmoke.smokeColor", 0xFF888888);
    inline static const ConfigValue<int> BRUSH_WIDTH = ConfigValue<int>(L"smokesim.enhancedsmoke.brushWidth", 14);
    inline static const ConfigValue<int> BRUSH_EDGE_FADE_RANGE = ConfigValue<int>(L"smokesim.enhancedsmoke.brushEdgeFadeRange", 6);
    inline static const ConfigValue<float> SMOKE_DENSITY = ConfigValue<float>(L"smokesim.enhancedsmoke.smokeDensity", 1.0f);
    inline static const ConfigValue<int> CURSOR_WIND_WIDTH = ConfigValue<int>(L"smokesim.enhancedsmoke.cursorWindWidth", 14);
    inline static const ConfigValue<float> CURSOR_WIND_SPEED = ConfigValue<float>(L"smokesim.enhancedsmoke.cursorWindSpeed", 0.2f);
    inline static const ConfigValue<int> SLOWDOWN_PERSISTENCE_DURATION = ConfigValue<int>(L"smokesim.enhancedsmoke.slowdownPersistenceDuration", 250);
    inline static const ConfigValue<float> SMOKE_VELOCITY_DIFFUSION = ConfigValue<float>(L"smokesim.enhancedsmoke.velocityDiffusion", 0.0f);
    inline static const ConfigValue<float> SMOKE_DENSITY_DIFFUSION = ConfigValue<float>(L"smokesim.enhancedsmoke.densityDiffusion", 0.0f);
    inline static const ConfigValue<float> SMOKE_DENSITY_REDUCTION_RATE = ConfigValue<float>(L"smokesim.enhancedsmoke.densityReductionRate", 0.02f);
    inline static const ConfigValue<int> SMOKE_KEY_CODE = ConfigValue<int>(L"smokesim.enhancedsmoke.smokeKeyCode", 'C');

    inline static const std::wstring CURSOR_TRAIL_OVERLAY_WINDOW_NAME = L"trailSmokeOverlay";
    inline static const std::wstring ENHANCED_SMOKE_OVERLAY_WINDOW_NAME = L"enhancedSmokeOverlay";
    inline static const std::wstring CURSOR_TRAIL_COLOR_SELECTOR_WINDOW_NAME = L"cursorTrailColorSelector";
    inline static const std::wstring ENHANCED_SMOKE_COLOR_SELECTOR_WINDOW_NAME = L"enhancedSmokeColorSelector";
};