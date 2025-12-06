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
    _clientAreaPanel->parentSize = { 1.0f, 1.0f };
    _UpdateClientAreaShadow();
    _contentPanel = Create<Panel>();
    _contentPanel->parentSize = { 1.0f, 1.0f };
    _contentPanel->yAlign = Alignment::END;

    _clientAreaPanel->AddItem(_contentPanel.get());

    _basePanel->AddItem(_clientAreaPanel.get());
    _basePanel->SubscribePostUpdate([=]() {
        _Update();
    }).Detach();
    _basePanel->SubscribePostDraw([=](Component*, Graphics* g) {
        _Draw(g);
    }).Detach();
}

void zcom::DefaultNonClientAreaScene::ProcessWindowResize(int newWidth, int newHeight, zwnd::ResizeFlags flags)
{
    bool fullscreen = flags.windowFullscreened;
    Rect finalClientAreaMargins = !fullscreen ? _clientAreaMargins : Rect{ 0, 0, 0, 0 };
    int finalTitleBarHeight = !fullscreen && _titleBarScene ? _titleBarScene->TitleBarSceneHeight() : 0;

    _basePanel->DeferLayoutUpdates();
    _clientAreaPanel->DeferLayoutUpdates();

    _basePanel->padding = finalClientAreaMargins;
    if (_titleBarPanel)
        _titleBarPanel->size = { _titleBarPanel->size->width, finalTitleBarHeight };
    _contentPanel->size = { _contentPanel->size->width, -finalTitleBarHeight };

    // Do layout update using Resize, because automatic one waits until next frame
    _clientAreaPanel->ResumeLayoutUpdates(false);
    _basePanel->ResumeLayoutUpdates(false);
    _basePanel->Resize({ newWidth, newHeight });
}

zcom::Panel* zcom::DefaultNonClientAreaScene::ProcessCreatedTitleBarScene(DefaultTitleBarScene* titleBarScene)
{
    _titleBarScene = titleBarScene;
    _titleBarPanel = CreatePanelForScene(titleBarScene);
    _titleBarPanel->parentSize = { 1.0f, _titleBarPanel->parentSize->height };

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
    panel->parentSize = { 1.0f, 1.0f };
    Panel* rawPtr = panel.get();
    _contentPanel->AddItem(std::move(panel));
    return rawPtr;
}

zcom::Panel* zcom::DefaultNonClientAreaScene::ProcessRecreatedScene(Scene* scene)
{
    _contentPanel->RemoveItem(scene->GetBasePanel());
    auto panel = CreatePanelForScene(scene);
    panel->parentSize = { 1.0f, 1.0f };
    Panel* rawPtr = panel.get();
    _contentPanel->AddItem(std::move(panel));
    return rawPtr;
}

void zcom::DefaultNonClientAreaScene::ProcessDeletedScene(Scene* scene)
{
    _contentPanel->RemoveItem(scene->GetBasePanel());
}

zcom::Rect zcom::DefaultNonClientAreaScene::GetResizingBorderWidths()
{
    return _resizingBorderWidths;
}

zcom::Rect zcom::DefaultNonClientAreaScene::GetClientAreaMargins()
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
                    _borderColor = Color(0x4D4D4D, 0.6f);
                    _shadowColor = Color(0, 0.6f);
                }
                else
                {
                    _borderColor = Color(0x4D4D4D, 0.3f);
                    _shadowColor = Color(0, 0.2f);
                }
                _UpdateClientAreaShadow();
                _basePanel->InvokeRedraw();
            }
        });
    }
}

void zcom::DefaultNonClientAreaScene::_Draw(Graphics* g)
{
    if (_drawWindowBorder)
    {
        RectF borderRect = {
            _clientAreaMargins.left - 0.5f,
            _clientAreaMargins.top - 0.5f,
            _basePanel->size_->width - (_clientAreaMargins.right - 0.5f),
            _basePanel->size_->height - (_clientAreaMargins.bottom - 0.5f)
        };
        g->DrawRectangle(borderRect, _borderColor);
    }
}

void zcom::DefaultNonClientAreaScene::_UpdateClientAreaShadow()
{
    if (_drawWindowShadow)
    {
        Shadow prop;
        prop.color = _shadowColor;
        _clientAreaPanel->SetProperty(prop);
    }
    else
    {
        _clientAreaPanel->RemoveProperty<Shadow>();
    }
}