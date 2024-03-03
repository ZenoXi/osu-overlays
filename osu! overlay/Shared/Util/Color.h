#pragma once

#include <cstdint>

namespace zutil
{
    class Color
    {
    public:
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
            : r(r), g(g), b(b), a(a) {}
        // 'color' format: 00RRGGBB
        Color(int32_t color, uint8_t a)
            : r((color >> 16) & 0xFF), g((color >> 8) & 0xFF), b((color) & 0xFF), a(a) {}
        // 'color' format: AARRGGBB
        Color(int32_t color = 0)
            : a((color >> 24) & 0xFF), r((color >> 16) & 0xFF), g((color >> 8) & 0xFF), b((color) & 0xFF) {}

        int32_t ToInt() const
        {
            return (a << 24) | (r << 16) | (g << 8) | (b << 0);
        }
        int32_t ToIntNoAlpha() const
        {
            return (r << 16) | (g << 8) | (b << 0);
        }
    };

}