#include <initguid.h>
#include "CursorTrailEffect.h"

#include <fstream>

//#define D2D_INPUT_COUNT 1
//#define D2D_INPUT0_SIMPLE
//#define D2D_REQUIRES_SCENE_POSITION
//#include <d2d1effecthelpers.hlsli>
//
//D2D_PS_ENTRY(D2D_ENTRY)
//{
//    auto color = D2DGetInput(0);
//    //D2DGetScenePosition().xy
//}

CursorTrailEffect::CursorTrailEffect()
{
    _refCount = 1;
    _drawInfo = nullptr;
    _inputRect = {};
}

HRESULT CursorTrailEffect::Register(_In_ ID2D1Factory1* pFactory)
{
#define XML(X) TEXT(#X)
    PCWSTR pszXml = XML(
        <?xml version='1.0'?>
        <Effect>
            <!-- System Properties -->
            <Property name='DisplayName' type='string' value='CursorTrail'/>
            <Property name='Author' type='string' value='Zenox'/>
            <Property name='Category' type='string' value='Custom'/>
            <Property name='Description'
                type='string'
                value='Generates a cursor trail from given points.'/>
            <Inputs>
                <Input name='Source'/>
            </Inputs>
            <Property name='HeadColor' type='vector4'>
                <Property name='DisplayName' type='string' value='Trail head color'/>
            </Property>
            <Property name='TrailWidth' type='float'>
                <Property name='DisplayName' type='string' value='Trail width at head'/>
            </Property>
            <Property name='HeadSize' type='float'>
                <Property name='DisplayName' type='string' value='Trail head size'/>
            </Property>
            <Property name='TrailEdgeWidth' type='float'>
                <Property name='DisplayName' type='string' value='Trail edge width'/>
            </Property>
            <Property name='HeadEdgeWidth' type='float'>
                <Property name='DisplayName' type='string' value='Trail head edge width'/>
            </Property>
            <Property name='PointCount' type='int32'>
                <Property name='DisplayName' type='string' value='Point count'/>
            </Property>
            <Property name='StopCount' type='int32'>
                <Property name='DisplayName' type='string' value='Gradient stop count'/>
            </Property>
            <Property name='ColorCycleDuration' type='float'>
                <Property name='DisplayName' type='string' value='Time (in seconds) it takes to cycle through the all gradient stops'/>
            </Property>
            <Property name='TrailResolution' type='int32'>
                <Property name='DisplayName' type='string' value='How many points to sample between two cursor positions'/>
            </Property>
            <Property name='Points' type='blob'>
                <Property name='DisplayName' type='string' value='Point coordinates'/>
            </Property>
            <Property name='PointData' type='blob'>
                <Property name='DisplayName' type='string' value='Other point data'/>
            </Property>
            <Property name='Stops' type='blob'>
                <Property name='DisplayName' type='string' value='Stop colors'/>
            </Property>
            <Property name='StopData' type='blob'>
                <Property name='DisplayName' type='string' value='Other stop data'/>
            </Property>
        </Effect>
    );

    const D2D1_PROPERTY_BINDING bindings[] =
    {
        D2D1_VALUE_TYPE_BINDING(L"HeadColor", &_SetHeadColor, &_GetHeadColor),
        D2D1_VALUE_TYPE_BINDING(L"TrailWidth", &_SetTrailWidth, &_GetTrailWidth),
        D2D1_VALUE_TYPE_BINDING(L"HeadSize", &_SetHeadSize, &_GetHeadSize),
        D2D1_VALUE_TYPE_BINDING(L"TrailEdgeWidth", &_SetTrailEdgeWidth, &_GetTrailEdgeWidth),
        D2D1_VALUE_TYPE_BINDING(L"HeadEdgeWidth", &_SetHeadEdgeWidth, &_GetHeadEdgeWidth),
        D2D1_VALUE_TYPE_BINDING(L"PointCount", &_SetPointCount, &_GetPointCount),
        D2D1_VALUE_TYPE_BINDING(L"StopCount", &_SetStopCount, &_GetStopCount),
        D2D1_VALUE_TYPE_BINDING(L"ColorCycleDuration", &_SetColorCycleDuration, &_GetColorCycleDuration),
        D2D1_VALUE_TYPE_BINDING(L"TrailResolution", &_SetTrailResolution, &_GetTrailResolution),
        D2D1_VALUE_TYPE_BINDING(L"Points", &_SetPoints, &_GetPoints),
        D2D1_VALUE_TYPE_BINDING(L"PointData", &_SetPointData, &_GetPointData),
        D2D1_VALUE_TYPE_BINDING(L"Stops", &_SetStops, &_GetStops),
        D2D1_VALUE_TYPE_BINDING(L"StopData", &_SetStopData, &_GetStopData)
    };

    return pFactory->RegisterEffectFromString(
        CLSID_CursorTrailEffect,
        pszXml,
        bindings,
        ARRAYSIZE(bindings),
        CreateEffect
    );
}

HRESULT CursorTrailEffect::CreateEffect(_Outptr_ IUnknown** ppEffectImpl)
{
    // This code assumes that the effect class initializes its reference count to 1.
    *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new CursorTrailEffect());
    if (*ppEffectImpl == nullptr)
        return E_OUTOFMEMORY;
    return S_OK;
}

IFACEMETHODIMP_(UINT32) CursorTrailEffect::GetInputCount() const noexcept
{
    return 1;
}

IFACEMETHODIMP CursorTrailEffect::MapInputRectsToOutputRect(
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

IFACEMETHODIMP CursorTrailEffect::MapOutputRectToInputRects(
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

IFACEMETHODIMP CursorTrailEffect::MapInvalidRect(
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

IFACEMETHODIMP CursorTrailEffect::SetDrawInfo(
    _In_ ID2D1DrawInfo* drawInfo
) noexcept
{
    _drawInfo = drawInfo;
    return _drawInfo->SetPixelShader(GUID_CursorTrailShader);
}

IFACEMETHODIMP CursorTrailEffect::Initialize(
    _In_ ID2D1EffectContext* pContextInternal,
    _In_ ID2D1TransformGraph* pTransformGraph
) noexcept
{
    // Load shader data
    std::ifstream fin("CursorTrailEffect.cso", std::ios::binary);
    fin.seekg(0, std::ios::end);
    size_t size = fin.tellg();
    char* data = new char[size];
    fin.seekg(0, std::ios::beg);
    fin.read(data, size);

    HRESULT hr = pContextInternal->LoadPixelShader(GUID_CursorTrailShader, (BYTE*)data, (UINT32)size);
    if (SUCCEEDED(hr))
        hr = pTransformGraph->SetSingleTransformNode(this);

    delete[] data;
    return hr;
}

IFACEMETHODIMP CursorTrailEffect::PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept
{
    return _drawInfo->SetPixelShaderConstantBuffer(reinterpret_cast<BYTE*>(&_data), sizeof(_data));
}

IFACEMETHODIMP CursorTrailEffect::SetGraph(_In_ ID2D1TransformGraph* pGraph) noexcept
{
    return E_NOTIMPL;
}

IFACEMETHODIMP_(ULONG) CursorTrailEffect::AddRef()
{
    InterlockedIncrement(&_refCount);
    return _refCount;
}

IFACEMETHODIMP_(ULONG) CursorTrailEffect::Release()
{
    ULONG ulRefCount = InterlockedDecrement(&_refCount);
    if (0 == _refCount)
        delete this;
    return ulRefCount;
}

IFACEMETHODIMP CursorTrailEffect::QueryInterface(_In_ REFIID riid, _Outptr_ void** ppOutput)
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

HRESULT CursorTrailEffect::_SetHeadColor(D2D_VECTOR_4F color)
{
    _data.headColor = color;
    return S_OK;
}

D2D_VECTOR_4F CursorTrailEffect::_GetHeadColor() const
{
    return _data.headColor;
}

HRESULT CursorTrailEffect::_SetTrailWidth(FLOAT width)
{
    _data.trailWidth = width;
    return S_OK;
}

FLOAT CursorTrailEffect::_GetTrailWidth() const
{
    return _data.trailWidth;
}

HRESULT CursorTrailEffect::_SetHeadSize(FLOAT size)
{
    _data.headSize = size;
    return S_OK;
}

FLOAT CursorTrailEffect::_GetHeadSize() const
{
    return _data.headSize;
}

HRESULT CursorTrailEffect::_SetTrailEdgeWidth(FLOAT width)
{
    _data.trailEdgeWidth = width;
    return S_OK;
}

FLOAT CursorTrailEffect::_GetTrailEdgeWidth() const
{
    return _data.trailEdgeWidth;
}

HRESULT CursorTrailEffect::_SetHeadEdgeWidth(FLOAT width)
{
    _data.headEdgeWidth = width;
    return S_OK;
}

FLOAT CursorTrailEffect::_GetHeadEdgeWidth() const
{
    return _data.headEdgeWidth;
}

HRESULT CursorTrailEffect::_SetPointCount(INT count)
{
    _data.pointCount = count;
    return S_OK;
}

INT CursorTrailEffect::_GetPointCount() const
{
    return _data.pointCount;
}

HRESULT CursorTrailEffect::_SetStopCount(INT count)
{
    _data.stopCount = count;
    return S_OK;
}

INT CursorTrailEffect::_GetStopCount() const
{
    return _data.stopCount;
}

HRESULT CursorTrailEffect::_SetColorCycleDuration(FLOAT duration)
{
    _data.colorCycleDuration = duration;
    return S_OK;
}

FLOAT CursorTrailEffect::_GetColorCycleDuration() const
{
    return _data.colorCycleDuration;
}

HRESULT CursorTrailEffect::_SetTrailResolution(INT resolution)
{
    _data.trailResolution = resolution;
    return S_OK;
}

INT CursorTrailEffect::_GetTrailResolution() const
{
    return _data.trailResolution;
}

HRESULT CursorTrailEffect::_SetPoints(std::array<D2D1_VECTOR_4F, MAX_POINT_COUNT> points)
{
    _data.points = points;
    return S_OK;
}

std::array<D2D1_VECTOR_4F, CursorTrailEffect::MAX_POINT_COUNT> CursorTrailEffect::_GetPoints() const
{
    return _data.points;
}

HRESULT CursorTrailEffect::_SetPointData(std::array<D2D1_VECTOR_4F, MAX_POINT_COUNT> data)
{
    _data.pointData = data;
    return S_OK;
}

std::array<D2D1_VECTOR_4F, CursorTrailEffect::MAX_POINT_COUNT> CursorTrailEffect::_GetPointData() const
{
    return _data.pointData;
}

HRESULT CursorTrailEffect::_SetStops(std::array<D2D1_VECTOR_4F, MAX_STOP_COUNT> stops)
{
    _data.stops = stops;
    return S_OK;
}

std::array<D2D1_VECTOR_4F, CursorTrailEffect::MAX_STOP_COUNT> CursorTrailEffect::_GetStops() const
{
    return _data.stops;
}

HRESULT CursorTrailEffect::_SetStopData(std::array<D2D1_VECTOR_4F, MAX_STOP_COUNT> data)
{
    _data.stopData = data;
    return S_OK;
}

std::array<D2D1_VECTOR_4F, CursorTrailEffect::MAX_STOP_COUNT> CursorTrailEffect::_GetStopData() const
{
    return _data.stopData;
}