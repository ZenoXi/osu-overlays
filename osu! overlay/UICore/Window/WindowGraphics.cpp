#include "WindowGraphics.h"
#include "WindowsEx.h"

#include "D2DEffects/TintEffect.h"
#include "Overlays/cursor-trail/CursorTrailEffect.h"

#include <iostream>

void zwnd::WindowGraphics::Initialize(HWND* hwnd, int width, int height, bool syncFramerate)
{
    p_hwnd = hwnd;
    _syncFramerate = syncFramerate;
    HRESULT hr;

    // Obtain the size of the drawing area.
    GetClientRect(*p_hwnd, &_windowRect);
    
    D2D1CreateFactory(
        D2D1_FACTORY_TYPE_MULTI_THREADED,
        { D2D1_DEBUG_LEVEL_INFORMATION },
        p_D2DFactory.GetAddressOf()
    );

    // Register custom effects
    hr = CustomTintEffect::Register(p_D2DFactory.Get());
    hr = CursorTrailEffect::Register(p_D2DFactory.Get());
    
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        p_D3DDevice.GetAddressOf(),
        nullptr,
        nullptr
    );

    // Acquire D3D resources
    HR(p_D3DDevice.As(&p_DXGIDevice));
    HR(p_DXGIDevice->GetAdapter(p_DXGIAdapter.GetAddressOf()));
    HR(p_DXGIAdapter->GetParent(__uuidof(p_DXGIFactory), reinterpret_cast<void**>(p_DXGIFactory.GetAddressOf())));

    DCompositionCreateDevice2(
        p_DXGIDevice.Get(),
        __uuidof(IDCompositionDesktopDevice),
        reinterpret_cast<void**>(p_DCompDevice.GetAddressOf())
    );
    p_DCompDevice->CreateTargetForHwnd(*p_hwnd, FALSE, p_DCompTarget.GetAddressOf());
    p_DCompDevice->CreateVisual(p_DCompVisual.GetAddressOf());
    p_DCompTarget->SetRoot(p_DCompVisual.Get());

    // Set up swap chain
    DXGI_SWAP_CHAIN_DESC1 scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));
    scd.Width = (UINT)width;
    scd.Height = (UINT)height;
    scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 2;
    scd.Flags |= DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
    scd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    hr = p_DXGIFactory->CreateSwapChainForComposition(
        p_D3DDevice.Get(),
        &scd,
        nullptr,
        p_SwapChain.GetAddressOf()
    );
    p_DCompVisual->SetContent(p_SwapChain.Get());
    //p_DCompVisual->SetOffsetX(10.0f);
    p_DCompDevice->Commit();

    //hr = p_DXGIFactory->CreateSwapChainForHwnd(
    //    p_D3DDevice.Get(),
    //    *p_hwnd,
    //    &scd,
    //    nullptr,
    //    nullptr,
    //    p_SwapChain.GetAddressOf()
    //);
    //std::cout << std::hex << hr << std::dec << "hiii" << '\n';

    HR(p_D2DFactory->CreateDevice(p_DXGIDevice.Get(), p_D2DDevice.GetAddressOf()));
    HR(p_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, p_Target.GetAddressOf()));
    HR(p_SwapChain->GetBuffer(
        0, // buffer index
        __uuidof(p_Surface),
        reinterpret_cast<void**>(p_Surface.GetAddressOf())
    ));
    auto props = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    HR(p_Target->CreateBitmapFromDxgiSurface(
        p_Surface.Get(),
        props,
        p_Bitmap.GetAddressOf()
    ));
    p_Target->SetTarget(p_Bitmap.Get());

    _allocator.SetTarget(p_Target.Get());
    _allocator.AddSource(2048, D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET,
        { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
    ), SEGMENT_POOL_MAIN);
    _allocator.AddSource(2048, D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET,
        { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
    ), SEGMENT_POOL_AUX1);
    _allocator.AddSource(2048, D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET,
        { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
    ), SEGMENT_POOL_AUX2);
    _allocator.AddSource(2048, D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET,
        { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
    ), SEGMENT_POOL_AUX3);

    _graphicsContext._target = p_Target.Get();
    _graphicsContext._factory = p_D2DFactory.Get();
    _graphicsContext._refs = &_references;
    _graphicsContext._allocator = &_allocator;
    _graphicsContext._textRenderContext = &_textRenderContext;

    _initialized = true;
}

void zwnd::WindowGraphics::Close()
{
    // Release all references
    for (int i = 0; i < _references.size(); i++)
    {
        if (*_references[i].first)
        {
            (*_references[i].first)->Release();
            (*_references[i].first) = nullptr;
        }
    }
    _references.clear();

    _allocator.ClearSources();

    // Release all objects
    p_Bitmap.Reset();
    p_Surface.Reset();
    p_Target.Reset();
    p_D2DDevice.Reset();
    p_SwapChain.Reset();
    p_DXGIAdapter.Reset();
    p_DXGIDevice.Reset();
    p_D3DDevice.Reset();
    p_D2DFactory.Reset();
}

void zwnd::WindowGraphics::BeginFrame()
{

}

void zwnd::WindowGraphics::EndFrame(bool swap, bool forceSync)
{
    static int c = 0;

    if (swap)
    {
        //std::cout << "presenting\n";
        HRESULT hr = p_SwapChain->Present(_syncFramerate || forceSync ? 1 : 0, 0);
        if (hr != S_OK)
        {
            std::cout << hr << '\n';
        }

        if (!_syncFramerate)
        {
            //std::cout << "not syncing " + std::to_string(c++) + "\n";
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        //std::cout << "presented\n";
    }
    else
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void zwnd::WindowGraphics::SetFullscreen(bool f)
{
    p_SwapChain->SetFullscreenState(f, NULL);
}

void zwnd::WindowGraphics::ResizeBuffers(int width, int height, bool lock)
{
    if (lock)
        Lock();

    HRESULT hr;

    // Release all references
    for (int i = 0; i < _references.size(); i++)
    {
        if (*_references[i].first)
        {
            (*_references[i].first)->Release();
            (*_references[i].first) = nullptr;
        }
    }
    _references.clear();

    _allocator.ClearSources();

    p_Target->Release();
    p_Bitmap->Release();
    p_Surface->Release();

    // Recreate swap chain
    //p_SwapChain->Release();

    //DXGI_SWAP_CHAIN_DESC1 scd;
    //ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));
    //scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    //scd.SampleDesc.Count = 1;
    //scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    //scd.BufferCount = 1;
    //scd.Flags |= DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
    //scd.Width = width;
    //scd.Height = height;

    ////std::cout << width << ':' << height << '\n';

    //HR(p_DXGIFactory->CreateSwapChainForHwnd(
    //    p_D3DDevice.Get(),
    //    *p_hwnd,
    //    &scd,
    //    nullptr,
    //    nullptr,
    //    p_SwapChain.GetAddressOf()
    //));

    // Resize swapchain
    hr = p_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE);
    //HR(p_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0/*DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH*/));
    //p_DCompVisual->SetOffsetX(20.0f);
    p_DCompDevice->Commit();

    // Recreate target reference
    HR(p_SwapChain->GetBuffer(
        0, // buffer index
        __uuidof(p_Surface),
        reinterpret_cast<void**>(p_Surface.GetAddressOf())
    ));
    auto props = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    HR(p_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, p_Target.GetAddressOf()));
    HR(p_Target->CreateBitmapFromDxgiSurface(
        p_Surface.Get(),
        props,
        p_Bitmap.GetAddressOf()
    ));
    p_Target->SetTarget(p_Bitmap.Get());

    _allocator.SetTarget(p_Target.Get());
    _allocator.RecreateInitialGroupSource();

    _graphicsContext._target = p_Target.Get();

    if (lock)
        Unlock();
}

void zwnd::WindowGraphics::ReleaseResource(IUnknown** res)
{
    // Release resource
    if (*res)
    {
        (*res)->Release();
        *res = nullptr;
    }

    // Remove pointer from vector
    for (auto it = _references.begin(); it != _references.end(); it++)
    {
        if ((*it).first == res)
        {
            _references.erase(it);
            break;
        }
    }
}

void zwnd::WindowGraphics::EnableVsync()
{
    _syncFramerate = true;
}

void zwnd::WindowGraphics::DisableVsync()
{
    _syncFramerate = false;
}

void zwnd::WindowGraphics::Lock()
{
    _m_gfx.lock();
}

void zwnd::WindowGraphics::Unlock()
{
    // Disable not-locked mutex warning
#pragma warning( push )
#pragma warning( disable : 26110 )
    _m_gfx.unlock();
#pragma warning( pop )
}

zcom::Graphics* zwnd::WindowGraphics::GetGraphics()
{
    return &_graphicsContext;

    //zcom::Graphics g;
    //g._target = p_Target.Get();
    //g.factory = p_D2DFactory.Get();
    //g.refs = &_references;
    //g.allocator = &_allocator;
    //g._textRenderContext = &_textRenderContext;
    //return g;
    //std::cout << g.target->GetSize().width << ":" << g.target->GetSize().height << '\n';
    //return { p_Target.Get(), p_D2DFactory.Get(), &_references, &_allocator };
}

zcom::TextRenderContext* zwnd::WindowGraphics::GetTextRenderContext()
{
    return &_textRenderContext;
}

IDXGISurface1* zwnd::WindowGraphics::GetSurface()
{
    return p_Surface.Get();
}

D2D1_SIZE_F zwnd::WindowGraphics::GetSurfaceSize()
{
    return p_Bitmap->GetSize();
}