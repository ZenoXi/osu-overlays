#pragma once

#include <functional>
#include <algorithm>
#include <vector>

namespace zutil
{
    template<class _Container, class _Predicate>
    void EraseIf(_Container& container, _Predicate predicate)
    {
        container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
    }

    template<typename _Value, class _Predicate>
    std::vector<_Value> Extract(std::vector<_Value>& container, _Predicate predicate)
    {
        std::vector<_Value> extractedValues;
        auto newEndIt = std::remove_if(container.begin(), container.end(), predicate);
        for (auto it = newEndIt; it != container.end(); it++)
            extractedValues.push_back(std::move(*it));
        container.erase(newEndIt, container.end());
        return extractedValues;
    }
}