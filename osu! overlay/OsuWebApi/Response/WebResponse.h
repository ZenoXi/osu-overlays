#pragma once

namespace webapi
{
    namespace resp
    {
        template<class T>
        struct WebResponse
        {
            int status;
            std::string error;
            std::optional<T> content;
        };
    }
}