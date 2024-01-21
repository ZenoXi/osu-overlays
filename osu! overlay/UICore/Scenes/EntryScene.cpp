#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "EntryScene.h"
#include "DefaultNonClientAreaScene.h"
#include "DefaultTitleBarScene.h"
#include "SmokeSim/SmokeSimScene.h"

zcom::EntryScene::EntryScene(App* app, zwnd::Window* window)
    : Scene(app, window)
{}

void zcom::EntryScene::_Init(SceneOptionsBase* options)
{
    EntrySceneOptions opt;
    if (options)
    {
        opt = *reinterpret_cast<const EntrySceneOptions*>(options);
    }

    _overlayListLabel = Create<Label>(L"Overlay list");
    _overlayListLabel->SetOffsetPixels(15, 15);
    _overlayListLabel->SetBaseSize(200, 30);
    _overlayListLabel->SetFontSize(20.0f);
    _overlayListLabel->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
    _overlayListLabel->SetFont(L"Arial");

    

    _smokeSimOverlayLabel = Create<Label>(L"Smoke simulation");
    _smokeSimOverlayLabel->SetOffsetPixels(15, 60);
    _smokeSimOverlayLabel->SetBaseSize(300, 30);
    _smokeSimOverlayLabel->SetVerticalTextAlignment(Alignment::CENTER);
    _smokeSimOverlayLabel->SetFontSize(14.0f);
    _smokeSimOverlayLabel->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
    _smokeSimOverlayLabel->SetFont(L"Arial");

    _smokeSimOverlayButton = Create<Button>(L"Enable");
    _smokeSimOverlayButton->SetOffsetPixels(330, 60);
    _smokeSimOverlayButton->SetBaseSize(80, 30);
    _smokeSimOverlayButton->SetActivation(ButtonActivation::RELEASE);
    _smokeSimOverlayButton->SubscribeOnActivated([&] {
        if (!_smokeSimOverlayWindowId)
        {
            _smokeSimOverlayWindowId = _app->CreateChildWindow(
                _window->GetWindowId(),
                zwnd::WindowProperties()
                    .WindowClassName(L"wndClassSmokeSimOverlay")
                    //.InitialSize(1920, 1080)
                    .InitialSize(1000, 1000)
                    .TopMost()
                    .DisableWindowAnimations()
                    .DisableWindowActivation()
                    .DisableMouseInteraction()
                    .DisableFastTooltips(),
                [](zwnd::Window* wnd) {
                    // Remove window decorations
                    zcom::DefaultNonClientAreaSceneOptions ncOpt;
                    ncOpt.drawWindowShadow = false;
                    ncOpt.drawWindowBorder = false;
                    ncOpt.resizingBorderWidths = { 7, 7, 7, 7 };
                    ncOpt.clientAreaMargins = { 0, 0, 0, 0 };
                    wnd->LoadNonClientAreaScene<zcom::DefaultNonClientAreaScene>(&ncOpt);

                    zcom::DefaultTitleBarSceneOptions tbOpt;
                    tbOpt.showCloseButton = false;
                    tbOpt.showMaximizeButton = false;
                    tbOpt.showMinimizeButton = false;
                    tbOpt.showTitle = false;
                    tbOpt.showIcon = false;
                    tbOpt.titleBarHeight = 0;
                    // Make entire window act as caption to enable easy window moving
                    tbOpt.captionHeight = 10000;
                    wnd->LoadTitleBarScene<zcom::DefaultTitleBarScene>(&tbOpt);

                    wnd->LoadStartingScene<zcom::SmokeSimScene>(nullptr);
                }
            );

            if (_smokeSimOverlayWindowId)
            {
                _smokeSimOverlayStatusLabel->SetText(L"Enabled");
                _smokeSimOverlayStatusLabel->SetFontColor(D2D1::ColorF(0.2f, 0.8f, 0.2f));
                _smokeSimOverlayButton->Text()->SetText(L"Disable");
            }
        }
        else
        {
            Handle<zwnd::Window> windowHandle = _app->GetWindow(_smokeSimOverlayWindowId.value());
            if (windowHandle.Valid())
                windowHandle->Close();

            _smokeSimOverlayWindowId = std::nullopt;
            _smokeSimOverlayStatusLabel->SetText(L"Disabled");
            _smokeSimOverlayStatusLabel->SetFontColor(D2D1::ColorF(0.8f, 0.2f, 0.2f));
            _smokeSimOverlayButton->Text()->SetText(L"Enable");
        }
    }).Detach();

    _smokeSimOverlayStatusLabel = Create<Label>(L"Disabled");
    _smokeSimOverlayStatusLabel->SetOffsetPixels(420, 60);
    _smokeSimOverlayStatusLabel->SetBaseSize(100, 30);
    _smokeSimOverlayStatusLabel->SetVerticalTextAlignment(Alignment::CENTER);
    _smokeSimOverlayStatusLabel->SetFontSize(14.0f);
    _smokeSimOverlayStatusLabel->SetFontColor(D2D1::ColorF(0.8f, 0.2f, 0.2f));
    _smokeSimOverlayStatusLabel->SetFont(L"Arial");



    _canvas->AddComponent(_overlayListLabel.get());
    _canvas->AddComponent(_smokeSimOverlayLabel.get());
    _canvas->AddComponent(_smokeSimOverlayButton.get());
    _canvas->AddComponent(_smokeSimOverlayStatusLabel.get());
    _canvas->SetBackgroundColor(D2D1::ColorF(0.1f, 0.1f, 0.1f));
}

void zcom::EntryScene::_Uninit()
{
    _canvas->ClearComponents();
}

void zcom::EntryScene::_Focus()
{

}

void zcom::EntryScene::_Unfocus()
{

}

void zcom::EntryScene::_Update()
{
    _canvas->Update();
}

void zcom::EntryScene::_Resize(int width, int height, ResizeInfo info)
{

}