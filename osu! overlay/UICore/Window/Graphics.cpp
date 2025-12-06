#include "Graphics.h"

#include <iostream>

void zcom::Graphics::PushTarget(const Bitmap& bitmap)
{
    if (!bitmap.CanBeTarget())
    {
        // TODO: Logging
        return;
    }

    if (!_targetStack.empty())
    {
        _target->PopAxisAlignedClip();
    }
    else
    {
        _target->GetTarget(&_initialTarget);
    }

    _target->SetTarget(bitmap.GetSource());
    _target->PushAxisAlignedClip(zcom::RectToD2D1_RECT_F(bitmap.GetSourceRect()), D2D1_ANTIALIAS_MODE_ALIASED);
    _targetStack.push_back({ bitmap.GetSource(), bitmap.GetSourceRect() });
}

void zcom::Graphics::PushAndClearTarget(const Bitmap& bitmap, zcom::Color clearColor)
{
    PushTarget(bitmap);
    Clear(clearColor);
}

void zcom::Graphics::PopTarget()
{
    if (_targetStack.empty())
        return;

    _targetStack.pop_back();
    _target->PopAxisAlignedClip();
    if (!_targetStack.empty())
    {
        _target->SetTarget(_targetStack.back().target);
        _target->PushAxisAlignedClip(zcom::RectToD2D1_RECT_F(_targetStack.back().rect), D2D1_ANTIALIAS_MODE_ALIASED);
    }
    else
    {
        _target->SetTarget(_initialTarget);
        _initialTarget->Release();
    }
}

zcom::Rect zcom::Graphics::GetTargetSourceClipRect() const
{
    if (_targetStack.empty())
        return Rect{ 0, 0, (int)_target->GetSize().width, (int)_target->GetSize().height };
    return _targetStack.back().rect;
}

zcom::Size zcom::Graphics::GetTargetSize() const
{
    auto rect = GetTargetSourceClipRect();
    return { rect.right - rect.left, rect.bottom - rect.top };
}

zcom::Rect zcom::Graphics::GetTargetRect() const
{
    return GetTargetSize().ToRect();
}

ID2D1Image* zcom::Graphics::GetCurrentTarget() const
{
    if (_targetStack.empty())
        return _initialTarget;
    return _targetStack.back().target;
}

std::optional<zcom::Bitmap> zcom::Graphics::CreateBitmap(int w, int h, int index)
{
    std::optional<zwnd::BitmapSegment> segmentOpt = _allocator->Allocate(w, h, index);
    if (segmentOpt)
        return zcom::Bitmap(std::make_shared<zcom::BitmapSegment>(zcom::Size{ w, h }, segmentOpt.value()));
    else
        return std::nullopt;
}

std::optional<zcom::Bitmap> zcom::Graphics::CreateBitmap(Size size, int index)
{
    return CreateBitmap(size.width, size.height, index);
}

void zcom::Graphics::Clear(const Color& color)
{
    _target->Clear(zcom::ColorToD2D1_COLOR_F(color));
}

void zcom::Graphics::Flush()
{
    _target->Flush();
    // TODO: Logging
}

void zcom::Graphics::DrawBitmap(const Bitmap& bitmap, std::optional<zcom::RectF> destinationRect, std::optional<zcom::RectF> sourceRect, float opacity)
{
    std::optional<Bitmap> b1;
    std::optional<Bitmap> b2;
    b1 = b2;

    zcom::RectF destinationBitmapRect = GetTargetSourceClipRect().ToRectF();
    zcom::RectF sourceBitmapRect = bitmap.GetSourceRect().ToRectF();
    if (destinationRect)
        destinationBitmapRect = destinationBitmapRect.Subrect(destinationRect.value());
    if (sourceRect)
        sourceBitmapRect = sourceBitmapRect.SubrectBounded(sourceRect.value());

    _target->DrawBitmap(
        bitmap.GetSource(),
        zcom::RectFToD2D1_RECT_F(destinationBitmapRect),
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        zcom::RectFToD2D1_RECT_F(sourceBitmapRect)
    );
}

void zcom::Graphics::DrawBitmap(const Bitmap& bitmap, float opacity)
{
    DrawBitmap(bitmap, std::nullopt, std::nullopt, opacity);
}

void zcom::Graphics::DrawBitmapIndirect(const Bitmap& bitmap, int proxyBitmapPoolIndex, bool clearTargetBitmap, std::optional<zcom::RectF> destinationRect, std::optional<zcom::RectF> sourceRect, float opacity)
{
    auto proxyBitmap = CreateBitmap(bitmap.GetSize().width, bitmap.GetSize().height, proxyBitmapPoolIndex);
    if (proxyBitmap)
    {
        PushAndClearTarget(proxyBitmap.value());
        DrawBitmap(bitmap);
        PopTarget();
        if (clearTargetBitmap)
            Clear();
        DrawBitmap(proxyBitmap.value(), destinationRect, sourceRect, opacity);
    }
    else
    {
        // TODO: Logging
    }
}

void zcom::Graphics::DrawBitmapIndirect(const Bitmap& bitmap, int proxyBitmapPoolIndex, bool clearTargetBitmap, float opacity)
{
    DrawBitmapIndirect(bitmap, proxyBitmapPoolIndex, clearTargetBitmap, std::nullopt, std::nullopt, opacity);
}

void zcom::Graphics::DrawEffect(Effect* effect)
{
    zcom::Point targetOrigin = GetTargetSourceClipRect().GetTopLeftPoint();
    ID2D1Effect* resolvedEffect = effect->ResolveEffect(this);
    _target->DrawImage(resolvedEffect, D2D1::Point2F((float)targetOrigin.x, (float)targetOrigin.y));
}

void zcom::Graphics::DrawEffectIndirect(Effect* effect, int proxyBitmapPoolIndex, bool clearTargetBitmap)
{
    zcom::Rect targetRect = GetTargetSourceClipRect();
    auto proxyBitmap = CreateBitmap(targetRect.GetWidth(), targetRect.GetHeight(), proxyBitmapPoolIndex);
    if (proxyBitmap)
    {
        PushAndClearTarget(proxyBitmap.value());
        zcom::Rect proxyTargetRect = GetTargetSourceClipRect();
        ID2D1Effect* resolvedEffect = effect->ResolveEffect(this);
        _target->DrawImage(resolvedEffect, D2D1::Point2F((float)proxyTargetRect.left, (float)proxyTargetRect.top));
        PopTarget();
        if (clearTargetBitmap)
            Clear();
        DrawBitmap(proxyBitmap.value());
    }
    else
    {
        // TODO: Logging
    }

}

void zcom::Graphics::FillFromData(const void* data, uint32_t pitch, std::optional<zcom::Rect> rect)
{
    ID2D1Image* image;
    _target->GetTarget(&image);

    ID2D1Bitmap* bitmap;
    HRESULT hr = image->QueryInterface(&bitmap);
    if (hr != S_OK)
    {
        std::cout << "[FillFromData] Failed to query Bitmap interface, aborting fill: " << hr << '\n';
        // TODO: Logging

        image->Release();
        return;
    }

    zcom::Rect destRect = GetTargetSourceClipRect();
    if (rect)
        destRect = destRect.SubrectBounded(rect.value());

    D2D1_RECT_U rectu = D2D1::RectU(destRect.left, destRect.top, destRect.right, destRect.bottom);
    bitmap->CopyFromMemory(&rectu, data, pitch);

    image->Release();
    bitmap->Release();
}

void zcom::Graphics::DrawLine(const zcom::PointF& startPoint, const zcom::PointF& endPoint, const zcom::Color& color, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    _DrawLine(startPoint, endPoint, _CreateSolidColorBrush(color), strokeWidth, strokeStyle);
}

void zcom::Graphics::DrawLine(const zcom::PointF& startPoint, const zcom::PointF& endPoint, const zcom::LinearGradient& gradient, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    _DrawLine(startPoint, endPoint, _CreateLinearGradientBrush(gradient), strokeWidth, strokeStyle);
}

void zcom::Graphics::DrawRectangle(const zcom::RectF& rect, const zcom::Color& color, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    _DrawRectangle(rect, _CreateSolidColorBrush(color), strokeWidth, strokeStyle);
}

void zcom::Graphics::DrawRectangle(const zcom::RectF& rect, const zcom::LinearGradient& gradient, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    _DrawRectangle(rect, _CreateLinearGradientBrush(gradient), strokeWidth, strokeStyle);
}

void zcom::Graphics::DrawRoundedRectangle(const zcom::RoundedRect& rect, const zcom::Color& color, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    _DrawRoundedRectangle(rect, _CreateSolidColorBrush(color), strokeWidth, strokeStyle);
}

void zcom::Graphics::DrawRoundedRectangle(const zcom::RoundedRect& rect, const zcom::LinearGradient& gradient, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    _DrawRoundedRectangle(rect, _CreateLinearGradientBrush(gradient), strokeWidth, strokeStyle);
}

void zcom::Graphics::DrawEllipse(const zcom::Ellipse& ellipse, const zcom::Color& color, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    _DrawEllipse(ellipse, _CreateSolidColorBrush(color), strokeWidth, strokeStyle);
}

void zcom::Graphics::DrawEllipse(const zcom::Ellipse& ellipse, const zcom::LinearGradient& gradient, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    _DrawEllipse(ellipse, _CreateLinearGradientBrush(gradient), strokeWidth, strokeStyle);
}

void zcom::Graphics::FillRectangle(const zcom::RectF& rect, const zcom::Color& color)
{
    _FillRectangle(rect, _CreateSolidColorBrush(color));
}

void zcom::Graphics::FillRectangle(const zcom::RectF& rect, const zcom::LinearGradient& gradient)
{
    _FillRectangle(rect, _CreateLinearGradientBrush(gradient));
}

void zcom::Graphics::FillRoundedRectangle(const zcom::RoundedRect& rect, const zcom::Color& color)
{
    _FillRoundedRectangle(rect, _CreateSolidColorBrush(color));
}

void zcom::Graphics::FillRoundedRectangle(const zcom::RoundedRect& rect, const zcom::LinearGradient& gradient)
{
    _FillRoundedRectangle(rect, _CreateLinearGradientBrush(gradient));
}

void zcom::Graphics::FillEllipse(const zcom::Ellipse& ellipse, const zcom::Color& color)
{
    _FillEllipse(ellipse, _CreateSolidColorBrush(color));
}

void zcom::Graphics::FillEllipse(const zcom::Ellipse& ellipse, const zcom::LinearGradient& gradient)
{
    _FillEllipse(ellipse, _CreateLinearGradientBrush(gradient).Get());
}

void zcom::Graphics::FillOpacityMask(
    const Bitmap& bitmap,
    const Bitmap& opacityMask,
    std::optional<zcom::RectF> destinationRect,
    std::optional<zcom::RectF> opacityMaskRect,
    std::optional<zcom::RectF> bitmapRect
) {
    // Destination rect subrects dont need to be bounded since they are already clipped using PushAxisAlignedClip
    // Source rect subrects aren't affected by the D2D clipping and need to be clipped manually to not expose
    // contents of other bitmap segments in the pool

    zcom::RectF finalDestinationRect = GetTargetSourceClipRect().ToRectF();
    if (destinationRect)
        finalDestinationRect = finalDestinationRect.Subrect(destinationRect.value());
    zcom::RectF finalOpacityMaskRect = opacityMask.GetSourceRect().ToRectF();
    if (opacityMaskRect)
        finalOpacityMaskRect = finalOpacityMaskRect.SubrectBounded(opacityMaskRect.value());
    zcom::RectF finalBitmapRect = bitmap.GetSourceRect().ToRectF();
    if (bitmapRect)
        finalBitmapRect = finalBitmapRect.SubrectBounded(bitmapRect.value());

    if (finalDestinationRect.GetWidth() <= 0 || finalDestinationRect.GetHeight() <= 0 ||
        finalOpacityMaskRect.GetWidth() <= 0 || finalOpacityMaskRect.GetHeight() <= 0 ||
        finalBitmapRect.GetWidth() <= 0 || finalBitmapRect.GetHeight() <= 0)
    {
        return;
    }

    ID2D1BitmapBrush* bitmapBrush;
    _target->CreateBitmapBrush(
        bitmap.GetSource(),
        D2D1::BitmapBrushProperties(
            D2D1_EXTEND_MODE_CLAMP,
            D2D1_EXTEND_MODE_CLAMP,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
        ),
        &bitmapBrush
    );
    float xScale = finalDestinationRect.GetWidth() / finalBitmapRect.GetWidth();
    float yScale = finalDestinationRect.GetHeight() / finalBitmapRect.GetHeight();
    bitmapBrush->SetTransform(D2D1::Matrix3x2F::Translation(
        -finalBitmapRect.left + finalDestinationRect.left,
        -finalBitmapRect.top + finalDestinationRect.top
    ) * D2D1::Matrix3x2F::Scale(
        xScale, yScale, D2D1::Point2F(finalDestinationRect.left, finalDestinationRect.top)
    ));
    if (bitmapBrush)
    {
        _target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        _target->FillOpacityMask(opacityMask.GetSource(), bitmapBrush, D2D1_OPACITY_MASK_CONTENT_GRAPHICS, zcom::RectFToD2D1_RECT_F(finalDestinationRect), zcom::RectFToD2D1_RECT_F(finalOpacityMaskRect));
        _target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        _target->Flush();
        bitmapBrush->Release();
    }
    else
    {
        // TODO: Logging
    }
}

void zcom::Graphics::DrawTextLayout(zcom::TextDesc& desc, zcom::PointF origin, const zcom::Color& color)
{
    auto layout = _textRenderContext->GetLayoutFromTextDesc(desc);
    ID2D1SolidColorBrush* brush;
    _target->CreateSolidColorBrush(zcom::ColorToD2D1_COLOR_F(color), &brush);
    if (brush)
    {
        zcom::PointF adjustedOrigin = AdjustPointToTargetSpace(origin);
        _target->DrawTextLayout({ adjustedOrigin.x, adjustedOrigin.y }, layout, brush);
        brush->Release();
    }
    else
    {
        // TODO: Logging
    }
}

zcom::Rect zcom::Graphics::AdjustRectToTargetSpace(const zcom::Rect& rect)
{
    auto currentClip = GetTargetSourceClipRect();
    return {
        rect.left + currentClip.left,
        rect.top + currentClip.top,
        rect.right + currentClip.left,
        rect.bottom + currentClip.top
    };
}

zcom::RectF zcom::Graphics::AdjustRectToTargetSpace(const zcom::RectF& rect)
{
    auto currentClip = GetTargetSourceClipRect();
    return {
        rect.left + currentClip.left,
        rect.top + currentClip.top,
        rect.right + currentClip.left,
        rect.bottom + currentClip.top
    };
}

zcom::Point zcom::Graphics::AdjustPointToTargetSpace(const zcom::Point& point)
{
    auto currentClip = GetTargetSourceClipRect();
    return {
        point.x + currentClip.left,
        point.y + currentClip.top
    };
}

zcom::PointF zcom::Graphics::AdjustPointToTargetSpace(const zcom::PointF& point)
{
    auto currentClip = GetTargetSourceClipRect();
    return {
        point.x + currentClip.left,
        point.y + currentClip.top
    };
}

void zcom::Graphics::_DrawLine(const zcom::PointF& startPoint, const zcom::PointF& endPoint, Microsoft::WRL::ComPtr<ID2D1Brush> brush, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    if (!brush)
        return;

    Microsoft::WRL::ComPtr<ID2D1StrokeStyle> strokeStylePtr;
    if (strokeStyle)
        strokeStylePtr = _CreateStrokeStyle(strokeStyle.value());

    _target->DrawLine(
        { startPoint.x, startPoint.y },
        { endPoint.x, endPoint.y },
        brush.Get(),
        strokeWidth,
        strokeStylePtr.Get()
    );
}

void zcom::Graphics::_DrawRectangle(const zcom::RectF& rect, Microsoft::WRL::ComPtr<ID2D1Brush> brush, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    if (!brush)
        return;

    Microsoft::WRL::ComPtr<ID2D1StrokeStyle> strokeStylePtr;
    if (strokeStyle)
        strokeStylePtr = _CreateStrokeStyle(strokeStyle.value());

    _target->DrawRectangle(zcom::RectFToD2D1_RECT_F(AdjustRectToTargetSpace(rect)), brush.Get(), strokeWidth, strokeStylePtr.Get());
}

void zcom::Graphics::_DrawRoundedRectangle(const zcom::RoundedRect& rect, Microsoft::WRL::ComPtr<ID2D1Brush> brush, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    if (!brush)
        return;

    Microsoft::WRL::ComPtr<ID2D1StrokeStyle> strokeStylePtr;
    if (strokeStyle)
        strokeStylePtr = _CreateStrokeStyle(strokeStyle.value());

    D2D1_ROUNDED_RECT roundedrect{};
    roundedrect.radiusX = rect.radiusX;
    roundedrect.radiusY = rect.radiusY;
    roundedrect.rect = zcom::RectFToD2D1_RECT_F(AdjustRectToTargetSpace(rect.rect));
    _target->DrawRoundedRectangle(roundedrect, brush.Get(), strokeWidth, strokeStylePtr.Get());
}

void zcom::Graphics::_DrawEllipse(const zcom::Ellipse& ellipse, Microsoft::WRL::ComPtr<ID2D1Brush> brush, float strokeWidth, std::optional<zcom::StrokeStyle> strokeStyle)
{
    if (!brush)
        return;

    Microsoft::WRL::ComPtr<ID2D1StrokeStyle> strokeStylePtr;
    if (strokeStyle)
        strokeStylePtr = _CreateStrokeStyle(strokeStyle.value());

    PointF adjustedPoint = AdjustPointToTargetSpace(ellipse.centerPosition);
    D2D1_ELLIPSE d2d1Ellipse{};
    d2d1Ellipse.point = { adjustedPoint.x, adjustedPoint.y };
    d2d1Ellipse.radiusX = ellipse.radiusX;
    d2d1Ellipse.radiusY = ellipse.radiusY;
    _target->DrawEllipse(d2d1Ellipse, brush.Get(), strokeWidth, strokeStylePtr.Get());
}

void zcom::Graphics::_FillRectangle(const zcom::RectF& rect, Microsoft::WRL::ComPtr<ID2D1Brush> brush)
{
    if (!brush)
        return;

    _target->FillRectangle(zcom::RectFToD2D1_RECT_F(AdjustRectToTargetSpace(rect)), brush.Get());
}

void zcom::Graphics::_FillRoundedRectangle(const zcom::RoundedRect& rect, Microsoft::WRL::ComPtr<ID2D1Brush> brush)
{
    if (!brush)
        return;

    D2D1_ROUNDED_RECT roundedrect{};
    roundedrect.radiusX = rect.radiusX;
    roundedrect.radiusY = rect.radiusY;
    roundedrect.rect = zcom::RectFToD2D1_RECT_F(AdjustRectToTargetSpace(rect.rect));
    _target->FillRoundedRectangle(roundedrect, brush.Get());
}

void zcom::Graphics::_FillEllipse(const zcom::Ellipse& ellipse, Microsoft::WRL::ComPtr<ID2D1Brush> brush)
{
    if (!brush)
        return;

    PointF adjustedPoint = AdjustPointToTargetSpace(ellipse.centerPosition);
    D2D1_ELLIPSE d2d1Ellipse{};
    d2d1Ellipse.point = { adjustedPoint.x, adjustedPoint.y };
    d2d1Ellipse.radiusX = ellipse.radiusX;
    d2d1Ellipse.radiusY = ellipse.radiusY;
    _target->FillEllipse(d2d1Ellipse, brush.Get());
}

Microsoft::WRL::ComPtr<ID2D1Brush> zcom::Graphics::_CreateSolidColorBrush(const zcom::Color& color)
{
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
    _target->CreateSolidColorBrush(zcom::ColorToD2D1_COLOR_F(color), brush.GetAddressOf());
    if (brush)
    {
        return brush;
    }
    else
    {
        // TODO: Logging
    }
    return nullptr;
}

Microsoft::WRL::ComPtr<ID2D1Brush> zcom::Graphics::_CreateLinearGradientBrush(const zcom::LinearGradient& gradient)
{
    Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> stopCollection = nullptr;
    Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> brush = nullptr;

    std::vector<D2D1_GRADIENT_STOP> gradientStops;
    for (auto& stop : gradient.gradientStops)
        gradientStops.push_back({ stop.position, zcom::ColorToD2D1_COLOR_F(stop.color) });

    _target->CreateGradientStopCollection(
        gradientStops.data(),
        (UINT32)gradientStops.size(),
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        stopCollection.GetAddressOf()
    );
    if (stopCollection)
    {
        zcom::PointF startPosition = AdjustPointToTargetSpace(gradient.startPosition);
        zcom::PointF endPosition = AdjustPointToTargetSpace(gradient.endPosition);
        _target->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(startPosition.x, startPosition.y),
                D2D1::Point2F(endPosition.x, endPosition.y)),
            stopCollection.Get(),
            brush.GetAddressOf()
        );

        if (brush)
        {
            return brush;
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }

    return nullptr;
}



Microsoft::WRL::ComPtr<ID2D1StrokeStyle> zcom::Graphics::_CreateStrokeStyle(const zcom::StrokeStyle& style)
{
    Microsoft::WRL::ComPtr<ID2D1StrokeStyle> strokeStyle;

    auto properties = D2D1::StrokeStyleProperties(
        _AppCapStyleToD2DCapStyle(style.startCap),
        _AppCapStyleToD2DCapStyle(style.endCap),
        _AppCapStyleToD2DCapStyle(style.dashCap),
        _AppLineJoinToD2DLineJoin(style.lineJoin),
        style.miterLimit,
        D2D1_DASH_STYLE_SOLID,
        style.dashOffset
    );

    if (style.dashes && !style.dashes->empty())
    {
        properties.dashStyle = D2D1_DASH_STYLE_CUSTOM;
        _factory->CreateStrokeStyle(properties, style.dashes->data(), (UINT32)style.dashes->size(), strokeStyle.GetAddressOf());
    }
    else
    {
        _factory->CreateStrokeStyle(properties, nullptr, 0, strokeStyle.GetAddressOf());
    }

    return strokeStyle;
}

D2D1_CAP_STYLE zcom::Graphics::_AppCapStyleToD2DCapStyle(zcom::CapStyle style)
{
    switch (style)
    {
        case zcom::CapStyle::FLAT: return D2D1_CAP_STYLE_FLAT;
        case zcom::CapStyle::SQUARE: return D2D1_CAP_STYLE_SQUARE;
        case zcom::CapStyle::ROUND: return D2D1_CAP_STYLE_ROUND;
        case zcom::CapStyle::TRIANGLE: return D2D1_CAP_STYLE_TRIANGLE;
        default: return D2D1_CAP_STYLE_FLAT;
    }
}

D2D1_LINE_JOIN zcom::Graphics::_AppLineJoinToD2DLineJoin(zcom::LineJoin join)
{
    switch (join)
    {
        case zcom::LineJoin::MITER: return D2D1_LINE_JOIN_MITER;
        case zcom::LineJoin::BEVEL: return D2D1_LINE_JOIN_BEVEL;
        case zcom::LineJoin::ROUND: return D2D1_LINE_JOIN_ROUND;
        case zcom::LineJoin::MITER_OR_BEVEL: return D2D1_LINE_JOIN_MITER_OR_BEVEL;
        default: return D2D1_LINE_JOIN_MITER;
    }
}
