#pragma once

#include "Graphics.h"

#include <mutex>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

namespace zwnd
{
    class WindowGraphics
    {
        HWND* p_hwnd = nullptr;
        std::mutex _m_gfx;
        bool _initialized = false;

        ComPtr<ID2D1Factory1> p_D2DFactory = NULL;
        ComPtr<ID2D1Device> p_D2DDevice = NULL;
        ComPtr<ID2D1DeviceContext> p_Target = NULL;
        //ComPtr<ID2D1RenderTarget> p_RTarget = NULL;
        std::vector<std::pair<IUnknown**, std::string>> _references;

        ComPtr<IDXGISwapChain1> p_SwapChain = NULL;
        ComPtr<ID3D11Device> p_D3DDevice = NULL;
        ComPtr<IDXGIDevice> p_DXGIDevice = NULL;
        ComPtr<IDXGIAdapter> p_DXGIAdapter = NULL;
        ComPtr<IDXGIFactory2> p_DXGIFactory = NULL;

        //ComPtr<ID3D11Texture2D> p_Texture;
        ComPtr<IDXGISurface1> p_Surface;
        ComPtr<ID2D1Bitmap1> p_Bitmap;
    public:
        RECT _windowRect;

        WindowGraphics() {};
        void Initialize(HWND* hwnd_t);
        void Close();
        bool Initialized() const { return _initialized; };

        void BeginFrame();
        void EndFrame(bool swap);
        void SetFullscreen(bool f);
        void ResizeBuffers(int width, int height, bool lock = true);
        void ReleaseResource(IUnknown** ref);

        void Lock();
        void Unlock();
        Graphics GetGraphics();

        IDXGISurface1* GetSurface();
        D2D1_SIZE_F GetSurfaceSize();
    };
}