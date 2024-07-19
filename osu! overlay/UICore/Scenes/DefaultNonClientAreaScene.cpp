#include "App.h" // App.h and Window.h must be included first
#include "Window/Window.h"
#include "DefaultNonClientAreaScene.h"
#include "DefaultTitleBarScene.h"

void zcom::DefaultNonClientAreaScene::Init(SceneOptionsBase* options)
{
    DefaultNonClientAreaSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const DefaultNonClientAreaSceneOptions*>(options);

    _resizingBorderWidths = opt.resizingBorderWidths;
    _clientAreaMargins = opt.clientAreaMargins;
    _drawWindowShadow = opt.drawWindowShadow;
    _drawWindowBorder = opt.drawWindowBorder;

    _windowActivationSubscription = _window->SubscribeToWindowMessages(nullptr);

    // Initialize primordial panel containing entire UI layout
    _nonClientAreaPanel = Create<Panel>();
    _basePanel = _nonClientAreaPanel.get();

    _clientAreaPanel = Create<Panel>();
    _clientAreaPanel->SetParentSizePercent(1.0f, 1.0f);
    _UpdateClientAreaShadow();
    _contentPanel = Create<Panel>();
    _contentPanel->SetParentSizePercent(1.0f, 1.0f);
    _contentPanel->SetVerticalAlignment(Alignment::END);

    _clientAreaPanel->AddItem(_contentPanel.get());

    _basePanel->AddItem(_clientAreaPanel.get());
    _basePanel->SubscribePostUpdate([=]() {
        _Update();
    }).Detach();
    _basePanel->SubscribePostDraw([=](Component*, Graphics g) {
        _Draw(g);
    }).Detach();
}

void zcom::DefaultNonClientAreaScene::ProcessWindowResize(int newWidth, int newHeight, zwnd::ResizeFlags flags)
{
    bool fullscreen = flags.windowFullscreened;
    RECT finalClientAreaMargins = !fullscreen ? _clientAreaMargins : RECT{ 0, 0, 0, 0 };
    int finalTitleBarHeight = !fullscreen && _titleBarScene ? _titleBarScene->TitleBarSceneHeight() : 0;

    _basePanel->DeferLayoutUpdates();
    _clientAreaPanel->DeferLayoutUpdates();

    _basePanel->SetPadding(finalClientAreaMargins);
    if (_titleBarPanel)
        _titleBarPanel->SetBaseHeight(finalTitleBarHeight);
    _contentPanel->SetBaseHeight(-finalTitleBarHeight);

    // Do layout update using Resize, because automatic one waits until next frame
    _clientAreaPanel->ResumeLayoutUpdates(false);
    _basePanel->ResumeLayoutUpdates(false);
    _basePanel->Resize(newWidth, newHeight);
}

zcom::Panel* zcom::DefaultNonClientAreaScene::ProcessCreatedTitleBarScene(DefaultTitleBarScene* titleBarScene)
{
    _titleBarScene = titleBarScene;
    _titleBarPanel = CreatePanelForScene(titleBarScene);
    _titleBarPanel->SetParentWidthPercent(1.0f);

    _clientAreaPanel->AddItem(_titleBarPanel.get());
    return _titleBarPanel.get();
}

void zcom::DefaultNonClientAreaScene::ProcessDeletedTitleBarScene(DefaultTitleBarScene* titleBarScene)
{
    _clientAreaPanel->RemoveItem(_titleBarPanel.get());
    _titleBarPanel.reset();
}

zcom::Panel* zcom::DefaultNonClientAreaScene::ProcessCreatedScene(Scene* scene)
{
    auto panel = CreatePanelForScene(scene);
    panel->SetParentSizePercent(1.0f, 1.0f);
    Panel* rawPtr = panel.get();
    _contentPanel->AddItem(std::move(panel));
    return rawPtr;
}

zcom::Panel* zcom::DefaultNonClientAreaScene::ProcessRecreatedScene(Scene* scene)
{
    _contentPanel->RemoveItem(scene->GetBasePanel());
    auto panel = CreatePanelForScene(scene);
    panel->SetParentSizePercent(1.0f, 1.0f);
    Panel* rawPtr = panel.get();
    _contentPanel->AddItem(std::move(panel));
    return rawPtr;
}

void zcom::DefaultNonClientAreaScene::ProcessDeletedScene(Scene* scene)
{
    _contentPanel->RemoveItem(scene->GetBasePanel());
}

RECT zcom::DefaultNonClientAreaScene::GetResizingBorderWidths()
{
    return _resizingBorderWidths;
}

RECT zcom::DefaultNonClientAreaScene::GetClientAreaMargins()
{
    return _clientAreaMargins;
}

void zcom::DefaultNonClientAreaScene::_Update()
{
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
                    _shadowColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.6f);
                }
                else
                {
                    _borderColor = D2D1::ColorF(0.3f, 0.3f, 0.3f, 0.3f);
                    _shadowColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.2f);
                }
                _UpdateClientAreaShadow();
                _basePanel->InvokeRedraw();
            }
        });
    }
}

void zcom::DefaultNonClientAreaScene::_Draw(Graphics g)
{
    if (_drawWindowBorder)
    {
        // Draw window border
        ID2D1SolidColorBrush* borderBrush;
        g.target->CreateSolidColorBrush(_borderColor, &borderBrush);
        if (borderBrush)
        {
            D2D1_RECT_F borderRect = {
                _clientAreaMargins.left - 0.5f,
                _clientAreaMargins.top - 0.5f,
                _basePanel->GetWidth() - (_clientAreaMargins.right - 0.5f),
                _basePanel->GetHeight() - (_clientAreaMargins.bottom - 0.5f)
            };
            g.target->DrawRectangle(borderRect, borderBrush);
            borderBrush->Release();
        }
        else
        {
            // TODO: Logging
        }
    }
}

void zcom::DefaultNonClientAreaScene::_UpdateClientAreaShadow()
{
    if (_drawWindowShadow)
    {
        PROP_Shadow prop;
        prop.color = _shadowColor;
        _clientAreaPanel->SetProperty(prop);
    }
    else
    {
        _clientAreaPanel->RemoveProperty<PROP_Shadow>();
    }
}