#pragma once

#include "Helper/ConfigValue.h"
#include <string>

namespace webapi
{
    struct WebApiConfig
    {
        inline static const ConfigValue<std::wstring> API_URL = ConfigValue<std::wstring>(
            L"webApi.url",
            std::wstring() + L"http" + L"://" + L"oe-api.kutra.lt" + L":" + L"8080" // Split url into parts since some anti virus software really does not like hardcoded ips
        );
        inline static const ConfigValue<bool> SHOW_HELP_PANEL = ConfigValue<bool>(L"webApi.showHelpPanel", true);
    };
}
