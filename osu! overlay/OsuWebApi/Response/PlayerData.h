#pragma once

#include <string>
#include <vector>

namespace webapi
{
    namespace resp
    {
        struct PlayerData
        {
            struct Score
            {
                float pp;
                float ppWeighted;
                std::string mapId;
            };

            std::string userId;
            std::string username;
            std::string mode;
            float pp;
            std::vector<Score> scores;
        };
    }
}