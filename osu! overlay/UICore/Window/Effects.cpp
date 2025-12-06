#include "Effects.h"
#include "Graphics.h"

ID2D1Effect* zcom::BitmapSourceEffect::ResolveEffect(Graphics* g)
{
    g->GetRenderContext()->CreateEffect(CLSID_D2D1Crop, _cropEffect.GetAddressOf());
    g->GetRenderContext()->CreateEffect(CLSID_D2D12DAffineTransform, _offsetEffect.GetAddressOf());
    if (_cropEffect && _offsetEffect)
    {
        // Crop input so that other segment contents don't leak (and less pixels need to be processed)
        _cropEffect->SetValue(D2D1_CROP_PROP_RECT, D2D1::RectF(
            (float)_inputBitmap->GetSourceRect().left,
            (float)_inputBitmap->GetSourceRect().top,
            (float)_inputBitmap->GetSourceRect().right,
            (float)_inputBitmap->GetSourceRect().bottom
        ));
        // Position effect so that 0,0 corresponds to the top left corner of the bitmap segment, then apply bitmap position
        _offsetEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, D2D1::Matrix3x2F::Translation(
            (float)-_inputBitmap->GetSourceRect().left + _bitmapPosition.x,
            (float)-_inputBitmap->GetSourceRect().top + _bitmapPosition.y
        ));

        _cropEffect->SetInput(0, _inputBitmap->GetSource());
        _offsetEffect->SetInputEffect(0, _cropEffect.Get());
        return _offsetEffect.Get();
    }
    else
    {
        // TODO: Logging
    }
    return nullptr;
}

ID2D1Effect* zcom::GrayscaleEffect::ResolveEffect(Graphics* g)
{
    ID2D1Effect* inputEffect = _inputEffect->ResolveEffect(g);
    if (!inputEffect)
        return nullptr;

    g->GetRenderContext()->CreateEffect(CLSID_D2D1Grayscale, _grayscaleEffect.GetAddressOf());
    if (!_grayscaleEffect)
    {
        // TODO: Logging
        return nullptr;
    }

    _grayscaleEffect->SetInputEffect(0, inputEffect);
    return _grayscaleEffect.Get();
}


ID2D1Effect* zcom::BrightnessEffect::ResolveEffect(Graphics* g)
{
    ID2D1Effect* inputEffect = _inputEffect->ResolveEffect(g);
    if (!inputEffect)
        return nullptr;

    g->GetRenderContext()->CreateEffect(CLSID_D2D1Brightness, _brightnessEffect.GetAddressOf());
    if (!_brightnessEffect)
    {
        // TODO: Logging
        return nullptr;
    }

    _brightnessEffect->SetInputEffect(0, inputEffect);
    _brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_WHITE_POINT, D2D1::Vector2F(1.0f, _brightness));
    _brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(1.0f, _brightness));
    return _brightnessEffect.Get();
}

ID2D1Effect* zcom::TintEffect::ResolveEffect(Graphics* g)
{
    ID2D1Effect* inputEffect = _inputEffect->ResolveEffect(g);
    if (!inputEffect)
        return nullptr;

    g->GetRenderContext()->CreateEffect(CLSID_D2D1Tint, _tintEffect.GetAddressOf());
    if (!_tintEffect)
    {
        // TODO: Logging
        return nullptr;
    }

    _tintEffect->SetInputEffect(0, inputEffect);
    D2D1_VECTOR_4F premultiplied = D2D1::Vector4F(_tintColor.r / 255.0f, _tintColor.g / 255.0f, _tintColor.b / 255.0f, _tintColor.a / 255.0f);
    premultiplied.x *= premultiplied.w;
    premultiplied.y *= premultiplied.w;
    premultiplied.z *= premultiplied.w;
    _tintEffect->SetValue(D2D1_TINT_PROP_COLOR, premultiplied);
    return _tintEffect.Get();
}

ID2D1Effect* zcom::OpacityEffect::ResolveEffect(Graphics* g)
{
    ID2D1Effect* inputEffect = _inputEffect->ResolveEffect(g);
    if (!inputEffect)
        return nullptr;

    g->GetRenderContext()->CreateEffect(CLSID_D2D1Opacity, _opacityEffect.GetAddressOf());
    if (!_opacityEffect)
    {
        // TODO: Logging
        return nullptr;
    }

    _opacityEffect->SetInputEffect(0, inputEffect);
    _opacityEffect->SetValue(D2D1_OPACITY_PROP_OPACITY, _opacity);
    return _opacityEffect.Get();
}

ID2D1Effect* zcom::ShadowEffect::ResolveEffect(Graphics* g)
{
    ID2D1Effect* inputEffect = _inputEffect->ResolveEffect(g);
    if (!inputEffect)
        return nullptr;

    g->GetRenderContext()->CreateEffect(CLSID_D2D1Shadow, _shadowEffect.GetAddressOf());
    if (!_shadowEffect)
    {
        // TODO: Logging
        return nullptr;
    }

    _shadowEffect->SetInputEffect(0, inputEffect);
    _shadowEffect->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1::Vector4F(_color.r / 255.0f, _color.g / 255.0f, _color.b / 255.0f, _color.a / 255.0f));
    _shadowEffect->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, _blurStandardDeviation);
    if (_offset.x != 0.0f || _offset.y != 0.0f)
    {
        g->GetRenderContext()->CreateEffect(CLSID_D2D12DAffineTransform, _offsetEffect.GetAddressOf());
        if (!_offsetEffect)
        {
            // TODO: Logging
            return _shadowEffect.Get();
        }

        _offsetEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, D2D1::Matrix3x2F::Translation(_offset.x, _offset.y));
        _offsetEffect->SetInputEffect(0, _shadowEffect.Get());
        return _offsetEffect.Get();
    }
    return _shadowEffect.Get();
}

ID2D1Effect* zcom::CompositeEffect::ResolveEffect(Graphics* g)
{
    if (_inputEffects.empty())
        return nullptr;
    if (_inputEffects.size() == 1)
        return _inputEffects[0]->ResolveEffect(g);

    // Composite effect documentation states, that it "accepts 2 or more inputs", but it doesn't seem
    // to work when more than 2 are provided, and it displays only the first two
    // As a workaround, effects are chained here to always have exactly 2 inputs to the composite effect

    Microsoft::WRL::ComPtr<ID2D1Effect> compositeEffect;
    g->GetRenderContext()->CreateEffect(CLSID_D2D1Composite, compositeEffect.GetAddressOf());
    if (!compositeEffect)
    {
        // TODO: Logging
        return nullptr;
    }

    _compositeEffects.push_back(compositeEffect);
    compositeEffect->SetInputEffect(0, _inputEffects[0]->ResolveEffect(g));
    compositeEffect->SetInputEffect(1, _inputEffects[1]->ResolveEffect(g));

    ID2D1Effect* prevCompositeEffect = compositeEffect.Get();
    for (int i = 2; i < _inputEffects.size(); i++)
    {
        g->GetRenderContext()->CreateEffect(CLSID_D2D1Composite, compositeEffect.GetAddressOf());
        if (!compositeEffect)
        {
            // TODO: Logging
            return nullptr;
        }

        _compositeEffects.push_back(compositeEffect);
        compositeEffect->SetInputEffect(0, prevCompositeEffect);
        compositeEffect->SetInputEffect(1, _inputEffects[i]->ResolveEffect(g));
        prevCompositeEffect = compositeEffect.Get();
    }

    return compositeEffect.Get();
}