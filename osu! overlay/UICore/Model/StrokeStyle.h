#pragma once

#include <optional>
#include <vector>

namespace zcom
{
    enum class CapStyle
    {
        FLAT,
        SQUARE,
        ROUND,
        TRIANGLE
    };

    enum class LineJoin
    {
        MITER,
        BEVEL,
        ROUND,
        MITER_OR_BEVEL
    };

    struct StrokeStyle
    {
        CapStyle startCap = CapStyle::FLAT;
        CapStyle endCap = CapStyle::FLAT;
        CapStyle dashCap = CapStyle::FLAT;
        LineJoin lineJoin = LineJoin::MITER;
        float miterLimit = 10.0f;
        float dashOffset = 0.0f;
        std::optional<std::vector<float>> dashes = std::nullopt;
    };
}