#pragma once

#include "Helper/ConfigValue.h"
#include <string>

struct CursorSensitivityConfig
{
    inline static const ConfigValue<std::wstring> GAME_CLIENT = ConfigValue<std::wstring>(L"cursorSensitivity.gameClient", L"stable");
    inline static const ConfigValue<float> SENSITIVITY = ConfigValue<float>(L"cursorSensitivity.sensitivity", 1.0f);
    inline static const ConfigValue<bool> RAW_INPUT_ENABLED = ConfigValue<bool>(L"cursorSensitivity.rawInputEnabled", false);
};