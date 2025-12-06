pragma once

#include <string>

namespace osu
{
    namespace resp
    {
        struct User
        {
            std::string id;
            std::string username;
            std::string countryCode;
            int64_t beatmapPlaycountsCount;
            int64_t scoresBestCount;
            int64_t rank;
            float pp;
            int64_t countryRank;
        };
    }
}