#pragma once

#include "../Base/ComponentBase.h"

#include <random>

namespace zcom
{
    // Completely barebones component, only contains base component functionality
    class DitheredBackground : public Component
    {
#pragma region base_class
    protected:
        void _OnUpdate() {}
        void _OnDraw(Graphics g)
        {
            ID2D1Bitmap1* backgroundBitmap = nullptr;
            g.target->CreateBitmap(
                D2D1::SizeU(GetWidth(), GetHeight()),
                nullptr,
                0,
                D2D1::BitmapProperties1(
                    D2D1_BITMAP_OPTIONS_TARGET,
                    { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
                ),
                &backgroundBitmap
            );

            // Gnerate source data
            auto sourceData = std::make_unique<float[]>(GetWidth() * GetHeight());
            float lColor = 0.2f;
            float rColor = 0.0f;
            for (int y = 0; y < GetHeight(); y++)
            {
                for (int x = 0; x < GetWidth(); x++)
                {
                    float color = lColor + (rColor - lColor) * (x / (float)GetWidth());
                    sourceData[y * GetWidth() + x] = color;
                }
            }

            // Dither
            auto ditheredData = std::make_unique<unsigned char[]>(GetWidth() * GetHeight() * 4);

            std::mt19937 engine;
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            int ditherRange = 0;

            for (int i = 0; i < GetWidth() * GetHeight(); i++)
            {
                float srcColor = sourceData[i];
                // Normalize to 0.0-255.0 range
                srcColor *= 255.0f;

                float lowerValue = std::floorf(srcColor) - ditherRange;
                float upperValue = std::ceilf(srcColor) + ditherRange;
                float weight = (srcColor - lowerValue) / (1.0f + ditherRange * 2);

                float finalColor;
                if (i < GetWidth() * GetHeight() / 2)
                {
                    finalColor = std::roundf(srcColor);
                }
                else
                {
                    if (dist(engine) <= weight)
                        finalColor = upperValue;
                    else
                        finalColor = lowerValue;
                }

                if (finalColor < 0.0f)
                    finalColor = 0.0f;
                else if (finalColor > 255.0f)
                    finalColor = 255.0f;

                unsigned char finalColorByte = (unsigned char)(int)finalColor;

                int index = i * 4;
                ditheredData[index + 0] = finalColorByte;
                ditheredData[index + 1] = finalColorByte;
                ditheredData[index + 2] = finalColorByte;
                ditheredData[index + 3] = 255; // Alpha
            }

            D2D1_RECT_U destRect = D2D1::RectU(0, 0, GetWidth(), GetHeight());
            backgroundBitmap->CopyFromMemory(&destRect, ditheredData.get(), GetWidth() * 4);

            g.target->DrawBitmap(backgroundBitmap);
            backgroundBitmap->Release();
        }
        void _OnResize(int width, int height) {}

    public:
        const char* GetName() const { return Name(); }
        static const char* Name() { return "dithered_background"; }
#pragma endregion

    protected:
        friend class Scene;
        friend class Component;
        DitheredBackground(Scene* scene) : Component(scene) {}
        void Init() {}
    public:
        ~DitheredBackground() {}
        DitheredBackground(DitheredBackground&&) = delete;
        DitheredBackground& operator=(DitheredBackground&&) = delete;
        DitheredBackground(const DitheredBackground&) = delete;
        DitheredBackground& operator=(const DitheredBackground&) = delete;
    };
}