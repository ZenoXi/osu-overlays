#pragma once

#include <string>
#include <vector>

namespace webapi
{
    namespace resp
    {
        struct LeaderboardPage
        {
            struct User
            {
                std::string username;
                float pp;
            };

            std::vector<User> users;
            int64_t pageNumber;
        };
    }
}