#pragma once

#include "UICore/Components/ComHelper.h"
#include "Color.h"

#include <vector>

namespace zcom
{
    struct GradientStop
    {
        float position;
        Color color;
    };

    struct LinearGradient
    {
        PointF startPosition;
        PointF endPosition;
        std::vector<GradientStop> gradientStops;
    };
}