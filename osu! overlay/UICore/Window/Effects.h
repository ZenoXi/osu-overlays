#pragma once

#include "DirectX.h"
#include "UICore/Components/ComHelper.h"

namespace zcom
{
    class Graphics;
    class Bitmap;

    class Effect
    {
    public:
        virtual ~Effect() {}
        virtual ID2D1Effect* ResolveEffect(Graphics* g) = 0;
    };

    class BitmapSourceEffect : public Effect
    {
        const Bitmap* _inputBitmap = nullptr;
        zcom::PointF _bitmapPosition = { 0.0f, 0.0f };

        Microsoft::WRL::ComPtr<ID2D1Effect> _cropEffect;
        Microsoft::WRL::ComPtr<ID2D1Effect> _offsetEffect;

    public:
        BitmapSourceEffect(const Bitmap* inputBitmap, zcom::PointF bitmapPosition = { 0.0f, 0.0f }) : _inputBitmap(inputBitmap), _bitmapPosition(bitmapPosition) {}
        ID2D1Effect* ResolveEffect(Graphics* g) override;
    };

    class GrayscaleEffect : public Effect
    {
        Effect* _inputEffect = nullptr;
        Microsoft::WRL::ComPtr<ID2D1Effect> _grayscaleEffect;

    public:
        GrayscaleEffect(Effect* inputEffect) : _inputEffect(inputEffect) {}
        ID2D1Effect* ResolveEffect(Graphics* g) override;
    };

    class BrightnessEffect : public Effect
    {
        Effect* _inputEffect = nullptr;
        float _brightness = 1.0f;

        Microsoft::WRL::ComPtr<ID2D1Effect> _brightnessEffect;

    public:
        BrightnessEffect(Effect* inputEffect, float brightness) : _inputEffect(inputEffect), _brightness(brightness) {}
        ID2D1Effect* ResolveEffect(Graphics* g) override;
    };

    class TintEffect : public Effect
    {
        Effect* _inputEffect = nullptr;
        zcom::Color _tintColor = zcom::Color(0xFFFFFF);

        Microsoft::WRL::ComPtr<ID2D1Effect> _tintEffect;

    public:
        TintEffect(Effect* inputEffect, zcom::Color tintColor) : _inputEffect(inputEffect), _tintColor(tintColor) {}
        ID2D1Effect* ResolveEffect(Graphics* g) override;
    };

    class OpacityEffect : public Effect
    {
        Effect* _inputEffect = nullptr;
        float _opacity = 1.0f;

        Microsoft::WRL::ComPtr<ID2D1Effect> _opacityEffect;

    public:
        OpacityEffect(Effect* inputEffect, float opacity) : _inputEffect(inputEffect), _opacity(opacity) {}
        ID2D1Effect* ResolveEffect(Graphics* g) override;
    };

    class ShadowEffect : public Effect
    {
        Effect* _inputEffect = nullptr;
        zcom::PointF _offset = { 0.0f, 0.0f };
        float _blurStandardDeviation = 3.0f;
        zcom::Color _color = zcom::Color(0, 0.75f);

        Microsoft::WRL::ComPtr<ID2D1Effect> _shadowEffect;
        Microsoft::WRL::ComPtr<ID2D1Effect> _offsetEffect;

    public:
        ShadowEffect(Effect* inputEffect, zcom::PointF offset = { 0.0f, 0.0f }, float blurStandardDeviation = 3.0f, zcom::Color color = zcom::Color(0, 0.75f))
            : _inputEffect(inputEffect), _offset(offset), _blurStandardDeviation(blurStandardDeviation), _color(color) {}
        ID2D1Effect* ResolveEffect(Graphics* g) override;
    };

    class CompositeEffect : public Effect
    {
        std::vector<Effect*> _inputEffects;
        std::vector<Microsoft::WRL::ComPtr<ID2D1Effect>> _compositeEffects;

    public:
        CompositeEffect(std::vector<Effect*> inputEffects) : _inputEffects(inputEffects) {}
        ID2D1Effect* ResolveEffect(Graphics* g) override;
    };
}