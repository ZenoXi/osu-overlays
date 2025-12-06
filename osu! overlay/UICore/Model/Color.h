#pragma once

#include <cstdint>
#include <algorithm>

namespace zcom
{
    class Color
    {
    public:
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 0;

        Color() {}
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
            : r(r), g(g), b(b), a(a) {}

        // 'color' hex format: 00RRGGBB
        Color(int32_t color, float a = 1.0f)
            : r((color >> 16) & 0xFF), g((color >> 8) & 0xFF), b((color) & 0xFF), a(uint8_t(a * 255.0f)) {}

        // 'color' hex format: AARRGGBB
        static Color ARGB(int32_t color) { return Color((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF); }

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

        bool operator==(const Color& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
        bool operator!=(const Color& other) const { return !(*this == other); }

        template<typename _Float>
        const Color Multiply(const _Float& f)
        {
            return Color(
                uint8_t(std::max(std::min(r * f, 255.0f), 0.0f)),
                uint8_t(std::max(std::min(g * f, 255.0f), 0.0f)),
                uint8_t(std::max(std::min(b * f, 255.0f), 0.0f)),
                uint8_t(std::max(std::min(a * f, 255.0f), 0.0f))
            );
        }
        template<typename _Float>
        const Color MultiplyNoAlpha(const _Float& f)
        {
            return Color(
                uint8_t(std::max(std::min(r * f, 255.0f), 0.0f)),
                uint8_t(std::max(std::min(g * f, 255.0f), 0.0f)),
                uint8_t(std::max(std::min(b * f, 255.0f), 0.0f)),
                a
            );
        }

        static Color None()     { return Color(0, 0.0f); }
        static Color White()    { return Color(0xFFFFFF); }
        static Color Gray()     { return Color(0x808080); }
        static Color Black()    { return Color(0x000000); }
        static Color Red()      { return Color(0xFF0000); }
        static Color Green()    { return Color(0x00FF00); }
        static Color Blue()     { return Color(0x0000FF); }
        static Color Yellow()   { return Color(0xFFFF00); }
        static Color Cyan()     { return Color(0x00FFFF); }
        static Color Magenta()  { return Color(0xFF00FF); }
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

        bool operator==(const ColorF& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
        bool operator!=(const ColorF& other) const { return !(*this == other); }
    };
}