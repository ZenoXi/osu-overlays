#include "App.h" // App.h and Window.h must be included first
#include "Window/Window.h"
#include "DefaultNonClientAreaScene.h"

zcom::DefaultNonClientAreaScene::DefaultNonClientAreaScene(App* app, zwnd::Window* window)
    : Scene(app, window)
{}

RECT zcom::DefaultNonClientAreaScene::GetResizingBorderWidths()
{
    return _resizingBorderWidths;
}

RECT zcom::DefaultNonClientAreaScene::GetClientAreaMargins()
{
    // TODO: These don't work when maximizing a window
    return _clientAreaMargins;
}

void zcom::DefaultNonClientAreaScene::SetClientAreaBitmap(ID2D1Bitmap* clientAreaBitmap)
{
    _clientAreaBitmap = clientAreaBitmap;
}

void zcom::DefaultNonClientAreaScene::SetTitleBarBitmap(ID2D1Bitmap* titleBarBitmap)
{
    _titleBarBitmap = titleBarBitmap;
}

void zcom::DefaultNonClientAreaScene::SetContentBitmap(ID2D1Bitmap* contentBitmap)
{
    _contentBitmap = contentBitmap;
}

void zcom::DefaultNonClientAreaScene::_Init(SceneOptionsBase* options)
{
    DefaultNonClientAreaSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const DefaultNonClientAreaSceneOptions*>(options);

    _resizingBorderWidths = opt.resizingBorderWidths;
    _clientAreaMargins = opt.clientAreaMargins;
    _drawWindowShadow = opt.drawWindowShadow;
    _drawWindowBorder = opt.drawWindowBorder;

    _windowActivationSubscription = _window->SubscribeToWindowMessages(nullptr);
}

void zcom::DefaultNonClientAreaScene::_Uninit()
{
    _canvas->ClearComponents();
    GetWindow()->Backend().Graphics()->ReleaseResource((IUnknown**)&_ccanvas);
}

void zcom::DefaultNonClientAreaScene::_Focus()
{

}

void zcom::DefaultNonClientAreaScene::_Unfocus()
{

}

void zcom::DefaultNonClientAreaScene::_Update()
{
    _canvas->Update();

    if (_windowActivationSubscription)
    {
        _windowActivationSubscription->HandlePendingEvents([=](zwnd::WindowMessage message) {
            if (message.id == zwnd::WindowActivateMessage::ID())
            {
                zwnd::WindowActivateMessage msg{};
                msg.Decode(message);
                if (msg.activationType == zwnd::WindowActivateMessage::ACTIVATED || msg.activationType == zwnd::WindowActivateMessage::CLICK_ACTIVATED)
                {
                    _borderColor = D2D1::ColorF(0.3f, 0.3f, 0.3f, 0.6f);
                    _shadowColor = D2D1::Vector4F(0.0f, 0.0f, 0.0f, 0.4f);
                    _redraw = true;
                }
                else
                {
                    _borderColor = D2D1::ColorF(0.3f, 0.3f, 0.3f, 0.3f);
                    _shadowColor = D2D1::Vector4F(0.0f, 0.0f, 0.0f, 0.2f);
                    _redraw = true;
                }
            }
            });
    }
}

bool zcom::DefaultNonClientAreaScene::_Redraw()
{
    return _redraw || !_ccanvas || _canvas->Redraw();
}

ID2D1Bitmap* zcom::DefaultNonClientAreaScene::_Draw(Graphics g)
{
    _redraw = false;

    // Create _ccanvas
    if (!_ccanvas)
    {
        g.target->CreateBitmap(
            D2D1::SizeU(_canvas->GetWidth(), _canvas->GetHeight()),
            nullptr,
            0,
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET,
                { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
            ),
            &_ccanvas
        );
        g.refs->push_back({ (IUnknown**)&_ccanvas, "Default non client area scene common canvas" });
    }

    RECT clientAreaMargins = GetClientAreaMargins();

    ID2D1Image* stash;
    g.target->GetTarget(&stash);
    g.target->SetTarget(_ccanvas);
    g.target->Clear();

    if (_drawWindowShadow)
    {
        // Draw client area shadow
        ID2D1Effect* shadowEffect = nullptr;
        g.target->CreateEffect(CLSID_D2D1Shadow, &shadowEffect);
        shadowEffect->SetInput(0, _clientAreaBitmap);
        shadowEffect->SetValue(D2D1_SHADOW_PROP_COLOR, _shadowColor);
        shadowEffect->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, 3.0f);
        g.target->DrawImage(shadowEffect, D2D1::Point2F(clientAreaMargins.left, clientAreaMargins.top));
        shadowEffect->Release();
    }

    // Draw content bitmap to window
    g.target->DrawBitmap(
        _clientAreaBitmap,
        D2D1::RectF(
            clientAreaMargins.left,
            clientAreaMargins.top,
            _canvas->GetWidth() - clientAreaMargins.right,
            _canvas->GetHeight() - clientAreaMargins.bottom
        )
    );

    if (_drawWindowBorder)
    {
        // Draw window border
        ID2D1SolidColorBrush* borderBrush;
        g.target->CreateSolidColorBrush(_borderColor, &borderBrush);
        D2D1_RECT_F borderRect = {
            clientAreaMargins.left - 0.5f,
            clientAreaMargins.top - 0.5f,
            _canvas->GetWidth() - (clientAreaMargins.right - 0.5f),
            _canvas->GetHeight() - (clientAreaMargins.bottom - 0.5f)
        };
        g.target->DrawRectangle(borderRect, borderBrush);
        borderBrush->Release();
    }

    // Draw scene elements
    if (_canvas->Redraw())
        _canvas->Draw(g);
    g.target->DrawBitmap(_canvas->ContentImage());

    // Unstash
    g.target->SetTarget(stash);
    stash->Release();

    return _ccanvas;
}

ID2D1Bitmap* zcom::DefaultNonClientAreaScene::_Image()
{
    return _ccanvas;
}

void zcom::DefaultNonClientAreaScene::_Resize(int width, int height, ResizeInfo info)
{
    GetWindow()->Backend().Graphics()->ReleaseResource((IUnknown**)&_ccanvas);
    _redraw = true;
}