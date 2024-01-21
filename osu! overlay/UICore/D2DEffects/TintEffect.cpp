#include <initguid.h>
#include "TintEffect.h"

#include <fstream>

//#define D2D_INPUT_COUNT 1
//#define D2D_INPUT0_SIMPLE
//#include <d2d1effecthelpers.hlsli>
//
//D2D_PS_ENTRY(Func)
//{
//    auto color = D2DGetInput(0);
//}

CustomTintEffect::CustomTintEffect()
{
    _refCount = 1;
}

HRESULT CustomTintEffect::Register(_In_ ID2D1Factory1* pFactory)
{
#define XML(X) TEXT(#X)
    PCWSTR pszXml = XML(
        <?xml version='1.0'?>
        <Effect>
            <!-- System Properties -->
            <Property name='DisplayName' type='string' value='CustomTint'/>
            <Property name='Author' type='string' value='Zenox'/>
            <Property name='Category' type='string' value='Custom'/>
            <Property name='Description'
                type='string'
                value='Multiplies the color of each pixel by the specified color.'/>
            <Inputs>
                <Input name='Source'/>
            </Inputs>
            <Property name='Color' type='vector2'>
                <Property name='DisplayName' type='string' value='Tint color'/>
            </Property>
        </Effect>
    );

    const D2D1_PROPERTY_BINDING bindings[] =
    {
        D2D1_VALUE_TYPE_BINDING(
            L"Color",
            &_SetColor,
            &_GetColor
            )
    };

    return pFactory->RegisterEffectFromString(
        CLSID_CustomTintEffect,
        pszXml,
        bindings,
        ARRAYSIZE(bindings),
        CreateEffect
    );
}

HRESULT CustomTintEffect::CreateEffect(_Outptr_ IUnknown** ppEffectImpl)
{
    // This code assumes that the effect class initializes its reference count to 1.
    *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new CustomTintEffect());
    if (*ppEffectImpl == nullptr)
        return E_OUTOFMEMORY;
    return S_OK;
}

IFACEMETHODIMP_(UINT32) CustomTintEffect::GetInputCount() const noexcept
{
    return 1;
}

IFACEMETHODIMP CustomTintEffect::MapInputRectsToOutputRect(
    _In_reads_(inputRectCount) const D2D1_RECT_L* pInputRects,
    _In_reads_(inputRectCount) const D2D1_RECT_L* pInputOpaqueSubRects,
    UINT32 inputRectCount,
    _Out_ D2D1_RECT_L* pOutputRect,
    _Out_ D2D1_RECT_L* pOutputOpaqueSubRect
) noexcept
{
    if (inputRectCount != 1)
        return E_INVALIDARG;

    *pOutputRect = pInputRects[0];
    *pOutputOpaqueSubRect = { 0, 0, 0, 0 };
    _inputRect = pInputRects[0];
    return S_OK;
}

IFACEMETHODIMP CustomTintEffect::MapOutputRectToInputRects(
    _In_ const D2D1_RECT_L* pOutputRect,
    _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
    UINT32 inputRectCount
) const noexcept
{
    if (inputRectCount != 1)
        return E_INVALIDARG;

    pInputRects[0] = *pOutputRect;
    return S_OK;
}

IFACEMETHODIMP CustomTintEffect::MapInvalidRect(
    UINT32 inputIndex,
    D2D1_RECT_L invalidInputRect,
    _Out_ D2D1_RECT_L* pInvalidOutputRect
) const noexcept
{
    if (inputIndex != 0)
        return E_INVALIDARG;

    *pInvalidOutputRect = invalidInputRect;
    return S_OK;
}

IFACEMETHODIMP CustomTintEffect::SetDrawInfo(
    _In_ ID2D1DrawInfo* drawInfo
) noexcept
{
    _drawInfo = drawInfo;
    return _drawInfo->SetPixelShader(GUID_CustomTintShader);
}

IFACEMETHODIMP CustomTintEffect::Initialize(
    _In_ ID2D1EffectContext* pContextInternal,
    _In_ ID2D1TransformGraph* pTransformGraph
) noexcept
{
    // Load shader data
    std::ifstream fin("TintEffect.cso", std::ios::binary);
    fin.seekg(0, std::ios::end);
    size_t size = fin.tellg();
    char* data = new char[size];
    fin.seekg(0, std::ios::beg);
    fin.read(data, size);

    HRESULT hr = pContextInternal->LoadPixelShader(GUID_CustomTintShader, (BYTE*)data, size);
    if (SUCCEEDED(hr))
        hr = pTransformGraph->SetSingleTransformNode(this);

    delete[] data;
    return hr;
}

IFACEMETHODIMP CustomTintEffect::PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept
{
    return _drawInfo->SetPixelShaderConstantBuffer(reinterpret_cast<BYTE*>(&_tintColor), sizeof(_tintColor));
}

IFACEMETHODIMP CustomTintEffect::SetGraph(_In_ ID2D1TransformGraph* pGraph) noexcept
{
    return E_NOTIMPL;
}

IFACEMETHODIMP_(ULONG) CustomTintEffect::AddRef()
{
    InterlockedIncrement(&_refCount);
    return _refCount;
}

IFACEMETHODIMP_(ULONG) CustomTintEffect::Release()
{
    ULONG ulRefCount = InterlockedDecrement(&_refCount);
    if (0 == _refCount)
        delete this;
    return ulRefCount;
}

IFACEMETHODIMP CustomTintEffect::QueryInterface(_In_ REFIID riid, _Outptr_ void** ppOutput)
{
    *ppOutput = nullptr;
    HRESULT hr = S_OK;

    if (riid == __uuidof(ID2D1EffectImpl))
        *ppOutput = reinterpret_cast<ID2D1EffectImpl*>(this);
    else if (riid == __uuidof(ID2D1DrawTransform))
        *ppOutput = static_cast<ID2D1DrawTransform*>(this);
    else if (riid == __uuidof(ID2D1Transform))
        *ppOutput = static_cast<ID2D1Transform*>(this);
    else if (riid == __uuidof(ID2D1TransformNode))
        *ppOutput = static_cast<ID2D1TransformNode*>(this);
    else if (riid == __uuidof(IUnknown))
        *ppOutput = this;
    else
        hr = E_NOINTERFACE;

    if (*ppOutput != nullptr)
        AddRef();

    return hr;
}

HRESULT CustomTintEffect::_SetColor(D2D_VECTOR_4F color)
{
    _tintColor = color;
    return S_OK;
}

D2D_VECTOR_4F CustomTintEffect::_GetColor() const
{
    return _tintColor;
}