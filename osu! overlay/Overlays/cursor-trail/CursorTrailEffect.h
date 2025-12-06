#pragma once

#include <d2d1_1.h>
#include <d2d1effectauthor.h>  
#include <d2d1effecthelpers.h>

#pragma comment( lib,"d2d1.lib" )

#include <array>

DEFINE_GUID(GUID_CursorTrailShader  , 0xB1EC1D9B, 0x1D55, 0x40C7, 0xA7, 0x5A, 0xCE, 0x5A, 0x45, 0xEF, 0x22, 0x2B);
DEFINE_GUID(CLSID_CursorTrailEffect , 0x7729110B, 0x53AD, 0x4277, 0x88, 0x3B, 0xE3, 0x72, 0x70, 0x68, 0x38, 0x27);

typedef enum CURSOR_TRAIL_EFFECT_PROP
{
    CURSOR_TRAIL_PROP_HEAD_COLOR = 0,
    CURSOR_TRAIL_PROP_TRAIL_WIDTH,
    CURSOR_TRAIL_PROP_HEAD_SIZE,
    CURSOR_TRAIL_PROP_TRAIL_EDGE_WIDTH,
    CURSOR_TRAIL_PROP_HEAD_EDGE_WIDTH,
    CURSOR_TRAIL_PROP_POINT_COUNT,
    CURSOR_TRAIL_PROP_STOP_COUNT,
    CURSOR_TRAIL_PROP_COLOR_CYCLE_DURATION,
    CURSOR_TRAIL_PROP_TRAIL_RESOLUTION,
    CURSOR_TRAIL_PROP_POINTS,
    CURSOR_TRAIL_PROP_POINT_DATA,
    CURSOR_TRAIL_PROP_STOPS,
    CURSOR_TRAIL_PROP_STOP_DATA
} CURSOR_TRAIL_EFFECT_PROP;

class CursorTrailEffect : public ID2D1EffectImpl, public ID2D1DrawTransform
{
public:
    static constexpr size_t MAX_POINT_COUNT = 512;
    static constexpr size_t MAX_STOP_COUNT = 512;

public:
    // ID2D1EffectImpl methods
    IFACEMETHODIMP Initialize(
        _In_ ID2D1EffectContext* pContextInternal,
        _In_ ID2D1TransformGraph* pTransformGraph
    ) noexcept;
    IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept;
    IFACEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* pGraph) noexcept;

    // Effect registration
    static HRESULT Register(_In_ ID2D1Factory1* pFactory);
    static HRESULT CreateEffect(_Outptr_ IUnknown** ppEffectImpl);

    // ID2D1TransformNode methods
    IFACEMETHODIMP_(UINT32) GetInputCount() const noexcept;

    // ID2D1Transform methods
    IFACEMETHODIMP MapInputRectsToOutputRect(
        _In_reads_(inputRectCount) const D2D1_RECT_L* pInputRects,
        _In_reads_(inputRectCount) const D2D1_RECT_L* pInputOpaqueSubRects,
        UINT32 inputRectCount,
        _Out_ D2D1_RECT_L* pOutputRect,
        _Out_ D2D1_RECT_L* pOutputOpaqueSubRect
    ) noexcept;
    IFACEMETHODIMP MapOutputRectToInputRects(
        _In_ const D2D1_RECT_L* pOutputRect,
        _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
        UINT32 inputRectCount
    ) const noexcept;
    IFACEMETHODIMP MapInvalidRect(
        UINT32 inputIndex,
        D2D1_RECT_L invalidInputRect,
        _Out_ D2D1_RECT_L* pInvalidOutputRect
    ) const noexcept;

    // ID2D1DrawTransform methods
    IFACEMETHODIMP SetDrawInfo(
        _In_ ID2D1DrawInfo* drawInfo
    ) noexcept;

    // IUnknown methods
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void** ppOutput);

private:
    // Constructor should be private since it should never be called externally.
    CursorTrailEffect();

    LONG _refCount;
    ID2D1DrawInfo* _drawInfo;
    D2D1_RECT_L _inputRect;

    struct ShaderData
    {
        // Head edge width
        // Trail edge width
        // Head width

        D2D_VECTOR_4F headColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        FLOAT trailWidth = 36.0f;
        FLOAT headSize = 36.0f;
        FLOAT trailEdgeWidth = 1.5f;
        FLOAT headEdgeWidth = 3.0f;
        INT pointCount = 0;
        INT stopCount = 0;
        FLOAT colorCycleDuration = 5.0f;
        INT trailResolution = 16;
        std::array<D2D1_VECTOR_4F, MAX_POINT_COUNT> points{};
        std::array<D2D1_VECTOR_4F, MAX_POINT_COUNT> pointData{};
        std::array<D2D1_VECTOR_4F, MAX_STOP_COUNT> stops{};
        std::array<D2D1_VECTOR_4F, MAX_STOP_COUNT> stopData{};
    };
    ShaderData _data{};
    HRESULT _SetHeadColor(D2D_VECTOR_4F color);
    D2D_VECTOR_4F _GetHeadColor() const;
    HRESULT _SetTrailWidth(FLOAT width);
    FLOAT _GetTrailWidth() const;
    HRESULT _SetHeadSize(FLOAT size);
    FLOAT _GetHeadSize() const;
    HRESULT _SetTrailEdgeWidth(FLOAT width);
    FLOAT _GetTrailEdgeWidth() const;
    HRESULT _SetHeadEdgeWidth(FLOAT width);
    FLOAT _GetHeadEdgeWidth() const;
    HRESULT _SetPointCount(INT count);
    INT _GetPointCount() const;
    HRESULT _SetStopCount(INT count);
    INT _GetStopCount() const;
    HRESULT _SetColorCycleDuration(FLOAT width);
    FLOAT _GetColorCycleDuration() const;
    HRESULT _SetTrailResolution(INT resolution);
    INT _GetTrailResolution() const;
    HRESULT _SetPoints(std::array<D2D1_VECTOR_4F, MAX_POINT_COUNT> points);
    std::array<D2D1_VECTOR_4F, MAX_POINT_COUNT> _GetPoints() const;
    HRESULT _SetPointData(std::array<D2D1_VECTOR_4F, MAX_POINT_COUNT> data);
    std::array<D2D1_VECTOR_4F, MAX_POINT_COUNT> _GetPointData() const;
    HRESULT _SetStops(std::array<D2D1_VECTOR_4F, MAX_STOP_COUNT> stops);
    std::array<D2D1_VECTOR_4F, MAX_STOP_COUNT> _GetStops() const;
    HRESULT _SetStopData(std::array<D2D1_VECTOR_4F, MAX_STOP_COUNT> data);
    std::array<D2D1_VECTOR_4F, MAX_STOP_COUNT> _GetStopData() const;
};