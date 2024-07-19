#pragma once

#include <cstdint>
#include <cmath>

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

        Color WithR(uint8_t r)
        {
            return Color(r, this->g, this->b, this->a);
        }
        Color WithG(uint8_t g)
        {
            return Color(this->r, g, this->b, this->a);
        }
        Color WithB(uint8_t b)
        {
            return Color(this->r, this->g, b, this->a);
        }
        Color WithA(uint8_t a)
        {
            return Color(this->r, this->g, this->b, a);
        }

        int32_t ToInt() const
        {
            return (a << 24) | (r << 16) | (g << 8) | (b << 0);
        }
        int32_t ToIntNoAlpha() const
        {
            return (r << 16) | (g << 8) | (b << 0);
        }
    };

    struct ColorF
    {
        float r;
        float g;
        float b;
        float a;

        static ColorF FromHSV(float H, float S, float V)
        {
            if (H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0)
            {
                return { 0.0f, 0.0f, 0.0f, 1.0f };
            }
            float s = S / 100;
            float v = V / 100;
            float C = s * v;
            float X = C * (1 - fabsf(fmodf(H / 60, 2) - 1));
            float m = v - C;

            float r, g, b;
            if (H >= 0 && H < 60)
                r = C, g = X, b = 0;
            else if (H >= 60 && H < 120)
                r = X, g = C, b = 0;
            else if (H >= 120 && H < 180)
                r = 0, g = C, b = X;
            else if (H >= 180 && H < 240)
                r = 0, g = X, b = C;
            else if (H >= 240 && H < 300)
                r = X, g = 0, b = C;
            else
                r = C, g = 0, b = X;

            return { r + m, g + m, b + m, 1.0f };
        }
    };
}