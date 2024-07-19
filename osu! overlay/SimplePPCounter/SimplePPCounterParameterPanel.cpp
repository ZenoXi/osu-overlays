#include "App.h"
#include "Scenes/Scene.h"
#include "Scenes/DefaultNonClientAreaScene.h"
#include "Scenes/DefaultTitleBarScene.h"
#include "Scenes/Scene.h"
#include "SimplePPCounterParameterPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Button.h"
#include "Components/Base/Dummy.h"
#include "SimplePPCounterConfig.h"
#include "SimplePPCounterScene.h"
#include "Helper/StringHelper.h"
#include "Shared/Styles/Styles.h"

void zcom::SimplePPCounterParameterPanel::Init()
{
    ScrollPanel::Init();

    Handle<zwnd::Window> windowHandle = _scene->GetApp()->FindWindowByClassName(SimplePPCounterConfig::OVERLAY_WINDOW_NAME);
    if (windowHandle.Valid())
        _overlayWindowId = windowHandle->GetWindowId();

    _dataProviderConnectionEvent = _scene->GetApp()->dataProvider.SubscribeOnConnectionEvent();
    _dataProviderConnectionEvent->ResetSynchronousHandler([=](osu::DataProvider::ConnectionEvent event) {
        ExecuteSynchronously([=]() {
            if (event == osu::DataProvider::CONNECTION_SUCCESSFUL)
            {
                _enableButton->SetActive(true);
            }
            else
            {
                _enableButton->SetActive(false);
                if (_overlayWindowId)
                    _CloseOverlayWindow();
            }
        });
    });

    SetBackgroundColor(D2D1::ColorF(0x202020));
    Scrollable(Scrollbar::VERTICAL, true);
    ScrollBackgroundVisible(Scrollbar::VERTICAL, true);

    auto flexPanel = Create<FlexPanel>(FlexDirection::DOWN);
    flexPanel->FillContainerWidth();

    auto generalPanel = Create<FlexPanel>(FlexDirection::DOWN);
    generalPanel->FillContainerWidth();

    auto titleRow = Create<FlexPanel>(FlexDirection::RIGHT);
    titleRow->FillContainerWidth();
    titleRow->SetPadding({ 15, 15, 15, 15 });
    auto generalLabel = Create<Label>(L"PP counter");
    generalLabel->SetBaseHeight(30);
    generalLabel->SetVerticalTextAlignment(Alignment::CENTER);
    generalLabel->SetFontSize(20.0f);
    generalLabel->SetProperty(FlexGrow());
    _enableButton = Create<Button>(L"Enable");
    _enableButton->SetBaseSize(90, 30);
    _enableButton->SetBorderVisibility(false);
    _enableButton->Label()->SetFontColor(D2D1::ColorF(0xEAEAEA));
    _enableButton->SetButtonColor(D2D1::ColorF(0x307020));
    _enableButton->SetButtonHoverColor(D2D1::ColorF(0x309020));
    _enableButton->SetButtonClickColor(D2D1::ColorF(0x308020));
    _enableButton->SetCornerRounding(2.0f);
    _enableButton->SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));
    _enableButton->SetProperty(PROP_Shadow{});
    if (!_overlayWindowId)
    {
        _enableButton->Label()->SetText(L"Enable");
        EnableButtonStyle::Apply(_enableButton.get());
    }
    else
    {
        _enableButton->Label()->SetText(L"Disable");
        DisableButtonStyle::Apply(_enableButton.get());
    }
    _enableButton->SetActive(_scene->GetApp()->dataProvider.Ready());
    _enableButton->SetActivation(ButtonActivation::RELEASE);
    _enableButton->SubscribeOnActivated([=]() {
        if (!_overlayWindowId)
        {
            _OpenOverlayWindow();
        }
        else
        {
            _CloseOverlayWindow();
        }
    }).Detach();


    titleRow->AddItem(std::move(generalLabel));
    titleRow->AddItem(_enableButton.get());
    generalPanel->AddItem(std::move(titleRow));
    flexPanel->AddItem(std::move(generalPanel));
    AddItem(std::move(flexPanel));
}

void zcom::SimplePPCounterParameterPanel::_OpenOverlayWindow()
{
    auto props = zwnd::WindowProperties()
        .WindowClassName(SimplePPCounterConfig::OVERLAY_WINDOW_NAME)
        .InitialSize(140, 80)
        .FixedSize()
        .TopMost()
        .DisableWindowAnimations()
        .DisableWindowActivation()
        .DisableMouseInteraction()
        .DisableFastTooltips();

    _overlayWindowId = _scene->GetApp()->CreateChildWindow(
        _scene->GetWindow()->GetWindowId(),
        props,
        [=](zwnd::Window* wnd) {
            // Remove window decorations
            DefaultNonClientAreaSceneOptions ncOpt;
            ncOpt.drawWindowShadow = false;
            ncOpt.drawWindowBorder = false;
            ncOpt.resizingBorderWidths = { 0, 0, 0, 0 };
            ncOpt.clientAreaMargins = { 0, 0, 0, 0 };
            wnd->LoadNonClientAreaScene<DefaultNonClientAreaScene>(&ncOpt);

            DefaultTitleBarSceneOptions tbOpt;
            tbOpt.showCloseButton = false;
            tbOpt.showMaximizeButton = false;
            tbOpt.showMinimizeButton = false;
            tbOpt.showTitle = false;
            tbOpt.showIcon = false;
            tbOpt.titleBarHeight = 0;
            // Make entire window act as caption to enable easy window moving
            tbOpt.captionHeight = 10000;
            wnd->LoadTitleBarScene<DefaultTitleBarScene>(&tbOpt);

            wnd->LoadStartingScene<SimplePPCounterScene>(nullptr);
        }
    );

    if (_overlayWindowId)
    {
        _enableButton->Label()->SetText(L"Disable");
        DisableButtonStyle::Apply(_enableButton.get());
    }
}

void zcom::SimplePPCounterParameterPanel::_CloseOverlayWindow()
{
    Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_overlayWindowId.value());
    if (handle.Valid())
        handle->Close();
    _overlayWindowId = std::nullopt;

    _enableButton->Label()->SetText(L"Enable");
    EnableButtonStyle::Apply(_enableButton.get());
}