#pragma once

#include "Helper/ConfigValue.h"
#include <string>

namespace osu
{
    struct DataProviderConfig
    {
        inline static const ConfigValue<std::wstring> URL = ConfigValue<std::wstring>(
            L"osu.dataProvider.url",
            std::wstring() + L"ws" + L"://" + L"127" + L"." + L"0" + L"." + L"0" + L"." + L"1" + L":" + L"24050" + L"/ws" // Split url into parts since some anti virus software really does not like hardcoded ips
        );
        inline static const ConfigValue<bool> SHOW_HELP_PANEL = ConfigValue<bool>(L"osu.dataProvider.showHelpPanel", true);

        inline static const std::wstring DATA_PROVIDER_SETUP_WINDOW_NAME = L"dataProviderSetup";
    };
}