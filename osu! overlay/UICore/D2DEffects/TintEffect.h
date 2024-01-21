#pragma once

#include <d2d1_1.h>
#include <d2d1effectauthor.h>  
#include <d2d1effecthelpers.h>

#pragma comment( lib,"d2d1.lib" )

DEFINE_GUID(GUID_CustomTintShader   , 0x80E491EB, 0xD4E4, 0x4584, 0x88, 0x63, 0xD1, 0x60, 0x8D, 0xA4, 0x05, 0x39);
DEFINE_GUID(CLSID_CustomTintEffect  , 0x7219CA11, 0x46B3, 0x4B3E, 0xB7, 0xAB, 0xF0, 0x5D, 0xBB, 0x61, 0xC3, 0xA1);

typedef enum CUSTOM_TINT_EFFECT_PROP
{
    CUSTOM_TINT_PROP_COLOR = 0
} CUSTOM_TINT_EFFECT_PROP;

class CustomTintEffect : public ID2D1EffectImpl, public ID2D1DrawTransform
{
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
    CustomTintEffect();

    LONG _refCount;
    ID2D1DrawInfo* _drawInfo;
    D2D1_RECT_L _inputRect;

    D2D_VECTOR_4F _tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    HRESULT _SetColor(D2D_VECTOR_4F color);
    D2D_VECTOR_4F _GetColor() const;
};