#pragma once

#include "DirectX.h"

#include "UICore/Model/Bitmap.h"
#include "UICore/Model/Color.h"
#include "UICore/Model/LinearGradient.h"
#include "UICore/Model/Ellipse.h"
#include "UICore/Model/StrokeStyle.h"
#include "UICore/Components/ComHelper.h"
#include "BitmapAllocator.h"
#include "Effects.h"
#include "TextRenderContext.h"
#include "Text.h"

#include <vector>
#include <string>

enum SegmentPoolIndex
{
    SEGMENT_POOL_MAIN,
    SEGMENT_POOL_AUX1,
    SEGMENT_POOL_AUX2,
    SEGMENT_POOL_AUX3
};

namespace zwnd
{
    class WindowGraphics;
}

namespace zcom
{
    class BitmapSegment : public BitmapStorage
    {
        Size _size;
        zwnd::BitmapSegment _segment;
        bool _canBeTarget;

    public:
        BitmapSegment(Size size, zwnd::BitmapSegment segment, bool canBeTarget = true) : _size(size), _segment(segment), _canBeTarget(canBeTarget) {}
        ~BitmapSegment() { _segment.Release(); }

        Size GetSize() const override { return _size; }
        bool CanBeTarget() const override { return _canBeTarget; }
        ID2D1Bitmap* GetSource() const override { return _segment.Bitmap(); }
        Rect GetSourceRect() const override { return _segment.rect; }
    };

    class Graphics
    {
        friend class zwnd::WindowGraphics;

        struct _TargetDesc
        {
            ID2D1Image* target;
            Rect rect;
        };

        std::vector<_TargetDesc> _targetStack;
        ID2D1Image* _initialTarget;

        ID2D1DeviceContext* _target = nullptr;
        ID2D1Factory1* _factory = nullptr;
        std::vector<std::pair<IUnknown**, std::string>>* _refs = nullptr;
        zwnd::BitmapAllocator* _allocator = nullptr;
        TextRenderContext* _textRenderContext = nullptr;

    public:
        ID2D1DeviceContext* GetRenderContext() const { return _target; }
        ID2D1Factory1* GetD2DFactory() const { return _factory; }
        zwnd::BitmapAllocator* GetBitmapAllocator() const { return _allocator; }

        void PushTarget(const Bitmap& bitmap);
        void PushAndClearTarget(const Bitmap& bitmap, Color clearColor = Color());
        void PopTarget();
        Rect GetTargetSourceClipRect() const;
        Size GetTargetSize() const;
        Rect GetTargetRect() const;
        ID2D1Image* GetCurrentTarget() const;
        std::optional<Bitmap> CreateBitmap(int w, int h, int index);
        std::optional<Bitmap> CreateBitmap(Size size, int index);
        void Clear(const Color& color = Color(0, 0.0f));
        void Flush();
        //std::optional<Bitmap> CreateBitmap(int w, int h, Bitmap destinationBitmap);
        //std::optional<Bitmap> CreateBitmap(int w, int h, std::vector<Bitmap> destinationBitmaps);
        //std::optional<Bitmap> CreateBitmap(int w, int h, D2D1_BITMAP_PROPERTIES1 properties);
        void DrawBitmap(const Bitmap& bitmap, std::optional<RectF> destinationRect = std::nullopt, std::optional<RectF> sourceRect = std::nullopt, float opacity = 1.0f);
        void DrawBitmap(const Bitmap& bitmap, float opacity);
        // Draws the specified bitmap to the target, by first drawing to a separate bitmap (which is allocated from the specified pool), and then using that bitmap to draw to the target
        // Use this method when the bitmap and target are from the same pool, since drawing a bitmap to itself isn't allowed
        // TODO: Add automatic detection when drawing to the same bitmap is attempted
        void DrawBitmapIndirect(const Bitmap& bitmap, int proxyBitmapPoolIndex, bool clearTargetBitmap, std::optional<RectF> destinationRect = std::nullopt, std::optional<RectF> sourceRect = std::nullopt, float opacity = 1.0f);
        void DrawBitmapIndirect(const Bitmap& bitmap, int proxyBitmapPoolIndex, bool clearTargetBitmap, float opacity);
        void DrawEffect(Effect* effect);
        void DrawEffectIndirect(Effect* effect, int proxyBitmapPoolIndex, bool clearTargetBitmap);
        void FillFromData(const void* data, uint32_t pitch, std::optional<Rect> rect = std::nullopt);

        void DrawLine(const PointF& startPoint, const PointF& endPoint, const Color& color, float strokeWidth = 1.0f, std::optional<StrokeStyle> strokeStyle = std::nullopt);
        void DrawLine(const PointF& startPoint, const PointF& endPoint, const LinearGradient& gradient, float strokeWidth = 1.0f, std::optional<StrokeStyle> strokeStyle = std::nullopt);
        void DrawRectangle(const RectF& rect, const Color& color, float strokeWidth = 1.0f, std::optional<StrokeStyle> strokeStyle = std::nullopt);
        void DrawRectangle(const RectF& rect, const LinearGradient& gradient, float strokeWidth = 1.0f, std::optional<StrokeStyle> strokeStyle = std::nullopt);
        void DrawRoundedRectangle(const RoundedRect& rect, const Color& color, float strokeWidth = 1.0f, std::optional<StrokeStyle> strokeStyle = std::nullopt);
        void DrawRoundedRectangle(const RoundedRect& rect, const LinearGradient& gradient, float strokeWidth = 1.0f, std::optional<StrokeStyle> strokeStyle = std::nullopt);
        void DrawEllipse(const Ellipse& ellipse, const Color& color, float strokeWidth = 1.0f, std::optional<StrokeStyle> strokeStyle = std::nullopt);
        void DrawEllipse(const Ellipse& ellipse, const LinearGradient& gradient, float strokeWidth = 1.0f, std::optional<StrokeStyle> strokeStyle = std::nullopt);
        void FillRectangle(const RectF& rect, const Color& color);
        void FillRectangle(const RectF& rect, const LinearGradient& gradient);
        void FillRoundedRectangle(const RoundedRect& rect, const Color& color);
        void FillRoundedRectangle(const RoundedRect& rect, const LinearGradient& gradient);
        void FillEllipse(const Ellipse& ellipse, const Color& color);
        void FillEllipse(const Ellipse& ellipse, const LinearGradient& gradient);
        void FillOpacityMask(
            const Bitmap& bitmap,
            const Bitmap& opacityMask,
            std::optional<RectF> destinationRect = std::nullopt,
            std::optional<RectF> opacityMaskRect = std::nullopt,
            std::optional<RectF> bitmapRect = std::nullopt
        );

        void DrawTextLayout(TextDesc& desc, PointF origin, const Color& color);

        Rect AdjustRectToTargetSpace(const Rect& rect);
        RectF AdjustRectToTargetSpace(const RectF& rect);
        Point AdjustPointToTargetSpace(const Point& point);
        PointF AdjustPointToTargetSpace(const PointF& point);

    private:
        void _DrawLine(const PointF& startPoint, const PointF& endPoint, Microsoft::WRL::ComPtr<ID2D1Brush> brush, float strokeWidth, std::optional<StrokeStyle> strokeStyle);
        void _DrawRectangle(const RectF& rect, Microsoft::WRL::ComPtr<ID2D1Brush> brush, float strokeWidth, std::optional<StrokeStyle> strokeStyle);
        void _DrawRoundedRectangle(const RoundedRect& rect, Microsoft::WRL::ComPtr<ID2D1Brush> brush, float strokeWidth, std::optional<StrokeStyle> strokeStyle);
        void _DrawEllipse(const Ellipse& ellipse, Microsoft::WRL::ComPtr<ID2D1Brush> brush, float strokeWidth, std::optional<StrokeStyle> strokeStyle);
        void _FillRectangle(const RectF& rect, Microsoft::WRL::ComPtr<ID2D1Brush> brush);
        void _FillRoundedRectangle(const RoundedRect& rect, Microsoft::WRL::ComPtr<ID2D1Brush> brush);
        void _FillEllipse(const Ellipse& ellipse, Microsoft::WRL::ComPtr<ID2D1Brush> brush);

        Microsoft::WRL::ComPtr<ID2D1Brush> _CreateSolidColorBrush(const Color& color);
        Microsoft::WRL::ComPtr<ID2D1Brush> _CreateLinearGradientBrush(const LinearGradient& gradient);
        Microsoft::WRL::ComPtr<ID2D1StrokeStyle> _CreateStrokeStyle(const StrokeStyle& style);
        D2D1_CAP_STYLE _AppCapStyleToD2DCapStyle(CapStyle style);
        D2D1_LINE_JOIN _AppLineJoinToD2DLineJoin(LineJoin join);
    };
}

inline bool operator==(const D2D1_COLOR_F& cl, const D2D1_COLOR_F& cr)
{
    return cl.a == cr.a
        && cl.b == cr.b
        && cl.g == cr.g
        && cl.r == cr.r;
}