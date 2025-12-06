#pragma once

#include "Helper/ConfigValue.h"

struct PPCounterConfig
{
    inline static const ConfigValue<std::wstring> LAYOUT_STRING = ConfigValue<std::wstring>(L"ppCounter.layoutString", L"0|330|80|-9|150|2|2");
    inline static const ConfigValue<int> BACKGROUND_COLOR = ConfigValue<int>(L"ppCounter.backgroundColor", 0xFF2D282A);
    inline static const ConfigValue<int> BACKGROUND_ACCENT_COLOR = ConfigValue<int>(L"ppCounter.backgroundAccentColor", 0xFFCEC49B);
    inline static const ConfigValue<int> DIFFICULTY_GRAPH_FOREGROUND_COLOR = ConfigValue<int>(L"ppCounter.difficultyGraphForegroundColor", 0xFF4E3335);
    inline static const ConfigValue<int> DIFFICULTY_GRAPH_BACKGROUND_COLOR = ConfigValue<int>(L"ppCounter.difficultyGraphBackgroundColor", 0xFF271315);
    inline static const ConfigValue<bool> SHOW_DIFFICULTY_GRAPH = ConfigValue<bool>(L"ppCounter.showDifficultyGraph", true);
};