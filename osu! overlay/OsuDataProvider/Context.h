#pragma once

#include "DataProvider.h"

#include <string>

namespace osu
{
    class Context
    {
        DataProvider dataProvider;
        std::wstring connectionUrl;
    };
}