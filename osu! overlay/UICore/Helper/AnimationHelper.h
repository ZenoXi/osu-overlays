#pragma once

#include <cmath>

namespace zanim
{
    template<typename _Flt>
    _Flt EaseInQuad(_Flt x)
    {
        return x * x;
    }
    template<typename _Flt>
    _Flt EaseOutQuad(_Flt x)
    {
        return 1 - (1 - x) * (1 - x);
    }
    template<typename _Flt>
    _Flt EaseInPow(_Flt x, _Flt p)
    {
        return std::pow(x, p);
    }
    template<typename _Flt>
    _Flt EaseOutPow(_Flt x, _Flt p)
    {
        return 1 - std::pow(1 - x, p);
    }

    template<typename _Val, typename _Flt>
    _Val Interpolate(_Val startValue, _Val endValue, _Flt progress)
    {
        return startValue + (endValue - startValue) * progress;
    }
}