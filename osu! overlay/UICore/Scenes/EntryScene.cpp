#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "EntryScene.h"
#include "DefaultNonClientAreaScene.h"
#include "DefaultTitleBarScene.h"
#include "OsuDataProvider/DataProviderConfig.h"
#include "OsuDataProvider/DataProviderSetupScene.h"
#include "SimplePPCounter/SimplePPCounterConfig.h"
#include "SimplePPCounter/SimplePPCounterParameterPanel.h"
#include "ComboBar/ComboBarConfig.h"
#include "ComboBar/ComboBarParameterPanel.h"
#include "SmokeSim/SmokeSimConfig.h"
#include "SmokeSim/SmokeSimParameterPanel.h"
#include "CursorTrail/CursorTrailConfig.h"
#include "CursorTrail/CursorTrailParameterPanel.h"
#include "Components/Base/ScrollPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Dummy.h"
#include "Shared/Styles/Styles.h"

void zcom::EntryScene::Init(SceneOptionsBase* options)
{
    EntrySceneOptions opt;
    if (options)
    {
        opt = *reinterpret_cast<const EntrySceneOptions*>(options);
    }

    _dataProviderConnectionEvent = _app->dataProvider.SubscribeOnConnectionEvent();
    _waitingForDataProvider = true;

    _windowCreatedEventSubscription = _app->SubscribeOnWindowCreated([=](zwnd::WindowId id, zwnd::WindowType, zwnd::WindowProperties props) {
        _basePanel->ExecuteSynchronously([=] {
            _HandleWindowCreatedEvent(id, props);
        });
    });
    _windowClosedEventSubscription = _app->SubscribeOnWindowClosed([=](zwnd::WindowId id) {
        _basePanel->ExecuteSynchronously([=] {
            _HandleWindowClosedEvent(id);
        });
    });

    _mainPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    _mainPanel->FillContainerSize();
    _mainPanel->SetSpacing(1);
    //_mainPanel->SetPadding(RECT{ 0, 1, 0, 0 });
    _mainPanel->SetBaseHeight(-1);
    _mainPanel->SetVerticalOffsetPixels(1);

    _selectionPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _selectionPanel->FillContainerSize();
    _selectionPanel->SetProperty(FlexShrink());
    _selectionPanel->SetBackgroundColor(D2D1::ColorF(0x1A1A1A));

    _loadingBar = Create<zcom::LoadingAnimation>();
    _loadingBar->SetParentWidthPercent(1.0f);
    _loadingBar->SetBaseHeight(1);
    _loadingBar->SetMainColor(D2D1::ColorF(0x5421FF, 0.5f));
    //_loadingBar->SetAccentColor(D2D1::ColorF(0x8A2BFF, 1.0f));
    _loadingBar->SetAccentColor(D2D1::ColorF(0xF966AB, 1.0f));

    auto osuMemoryPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    osuMemoryPanel->FillContainerWidth();
    osuMemoryPanel->SetBaseHeight(40);
    osuMemoryPanel->SetHeightFixed(true);
    osuMemoryPanel->SetSpacing(10);
    osuMemoryPanel->SetPadding(RECT{10, 10, 10, 0});
    osuMemoryPanel->SetItemAlignment(Alignment::CENTER);
    auto osuMemoryFiller = Create<Dummy>();
    osuMemoryFiller->SetBaseHeight(1);
    osuMemoryFiller->SetProperty(FlexGrow());
    auto osuMemoryIcon = Create<Image>(_window->resourceManager.GetImage("osu_memory"));
    osuMemoryIcon->SetBaseSize(26, 26);
    osuMemoryIcon->SetHorizontalOffsetPixels(5);
    auto osuMemoryLabel = Create<Label>(L"osu! data provider");
    osuMemoryLabel->SetBaseHeight(20);
    osuMemoryLabel->AutomaticWidth();
    osuMemoryLabel->SetVerticalTextAlignment(Alignment::CENTER);
    auto osuMemorySeparator = Create<Dummy>();
    osuMemorySeparator->SetBaseSize(1, 22);
    osuMemorySeparator->SetBackgroundColor(D2D1::ColorF(0x404040));
    _osuMemoryToggle = Create<Toggle>(false);
    _osuMemoryToggle->SetBaseSize(40, 22);
    _osuMemoryToggle->SetCornerRounding(11.0f);
    _osuMemoryToggle->SetMarginToBorder(4.0f);
    _osuMemoryToggle->SetBorderVisibility(false);
    _osuMemoryToggle->SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));
    _osuMemoryToggle->SetToggledOnBackgroundColor(D2D1::ColorF(0x308020));
    _osuMemoryToggle->SetToggledOffBackgroundColor(D2D1::ColorF(0x303030));
    _osuMemoryToggle->SetToggledOnAnchorColor(D2D1::ColorF(0xC0C0C0));
    _osuMemoryToggle->SetToggledOffAnchorColor(D2D1::ColorF(0xC0C0C0));
    _osuMemoryToggle->SetProperty(PROP_Shadow{});
    _osuMemoryToggle->SetToggledOn(_app->dataProvider.Ready());
    _osuMemoryToggle->SubscribeOnToggled([=](bool* newValue) {
        if (*newValue)
        {
            std::wstring url = _app->config.GetConfigValue(osu::DataProviderConfig::URL, Config::ADD_AND_SAVE_IF_MISSING);
            if (_app->dataProvider.Connect(url))
            {
                _waitingForDataProvider = true;
                _osuMemoryToggle->SetActive(false);
                _loadingBar->ShowAnimation();
                *newValue = false;
            }
        }
        else
        {
            if (_app->dataProvider.Disconnect())
            {
                _waitingForDataProvider = true;
                _osuMemoryToggle->SetActive(false);
                _loadingBar->ShowAnimation();
                *newValue = true;
            }
        }
    }).Detach();
    auto osuMemoryButton = Create<Button>(L"");
    osuMemoryButton->SetBaseSize(30, 30);
    osuMemoryButton->SetBorderVisibility(false);
    osuMemoryButton->SetBackgroundColor(D2D1::ColorF(0x303030));
    osuMemoryButton->SetButtonColor(D2D1::ColorF(0, 0.0f));
    osuMemoryButton->SetButtonHoverColor(D2D1::ColorF(0xFFFFFF, 0.1f));
    osuMemoryButton->SetButtonClickColor(D2D1::ColorF(0x000000, 0.1f));
    osuMemoryButton->SetButtonImageAll(_window->resourceManager.GetImage("settings_22x22"));
    osuMemoryButton->ButtonImage()->SetPlacement(zcom::ImagePlacement::CENTER);
    osuMemoryButton->ButtonImage()->SetTintColor(D2D1::ColorF(0xC0C0C0));
    osuMemoryButton->UseImageParamsForAll(osuMemoryButton->ButtonImage());
    osuMemoryButton->SetCornerRounding(3.0f);
    osuMemoryButton->SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));
    osuMemoryButton->SetProperty(PROP_Shadow{});
    osuMemoryButton->SubscribeOnActivated([=]() {
        if (!_dataProviderSetupWindowId)
        {
            _OpenDataProviderSetup(false);
        }
        else
        {
            Handle<zwnd::Window> handle = _app->GetWindow(_dataProviderSetupWindowId.value());
            if (handle.Valid())
                handle->Backend().Focus();
        }
    }).Detach();
    osuMemoryPanel->AddItem(std::move(osuMemoryFiller));
    osuMemoryPanel->AddItem(std::move(osuMemoryIcon));
    osuMemoryPanel->AddItem(std::move(osuMemoryLabel));
    osuMemoryPanel->AddItem(std::move(osuMemorySeparator));
    osuMemoryPanel->AddItem(_osuMemoryToggle.get());
    osuMemoryPanel->AddItem(std::move(osuMemoryButton));

    _overlayListPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _overlayListPanel->FillContainerWidth();
    _overlayListPanel->SetHeightFixed(true);
    _overlayListPanel->SetProperty(FlexGrow());
    _overlayListPanel->SetSpacing(3);
    _overlayListPanel->SetItemAlignment(Alignment::CENTER);

    auto overlayListHeader = Create<FlexPanel>(FlexDirection::RIGHT);
    overlayListHeader->FillContainerWidth();
    overlayListHeader->SetBaseHeight(30);
    overlayListHeader->SetPadding(RECT{ 12, 0, 12, 0 });
    overlayListHeader->SetSpacing(5);
    auto headerLabel = Create<Label>(L"Available overlays");
    headerLabel->AutomaticWidth();
    headerLabel->SetBaseHeight(30);
    headerLabel->SetVerticalTextAlignment(Alignment::CENTER);
    headerLabel->SetFontSize(14.0f);
    headerLabel->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
    headerLabel->SetFont(L"Arial");
    auto headerSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(headerSeparator.get());
    //headerSeparator->SetBaseHeight(1);
    headerSeparator->SetVerticalOffsetPixels(1);
    headerSeparator->SetVerticalAlignment(Alignment::CENTER);
    headerSeparator->SetProperty(FlexShrink());
    //headerSeparator->SetBackgroundColor(D2D1::ColorF(0x404040));
    overlayListHeader->AddItem(std::move(headerLabel));
    overlayListHeader->AddItem(std::move(headerSeparator));

    _overlayListPanel->AddItem(std::move(overlayListHeader));

    _CreateOverlaySelector(L"Enhanced smoke", SmokeSimConfig::ENHANCED_SMOKE_OVERLAY_WINDOW_NAME, [=] {
        auto paramPanel = Create<SmokeSimParameterPanel>(SmokeSimType::ENHANCED_SMOKE);
        paramPanel->SetParentHeightPercent(1.0f);
        paramPanel->SetBaseWidth(300);
        return paramPanel;
    });
    _CreateOverlaySelector(L"Trail smoke", SmokeSimConfig::CURSOR_TRAIL_OVERLAY_WINDOW_NAME, [=] {
        auto paramPanel = Create<SmokeSimParameterPanel>(SmokeSimType::CURSOR_TRAIL);
        paramPanel->SetParentHeightPercent(1.0f);
        paramPanel->SetBaseWidth(300);
        return paramPanel;
    });
    _CreateOverlaySelector(L"Cursor trail", CursorTrailConfig::OVERLAY_WINDOW_NAME, [=] {
        auto paramPanel = Create<CursorTrailParameterPanel>();
        paramPanel->SetParentHeightPercent(1.0f);
        paramPanel->SetBaseWidth(300);
        return paramPanel;
    });
    _CreateOverlaySelector(L"Combo counter", ComboBarConfig::OVERLAY_WINDOW_NAME, [=] {
        auto paramPanel = Create<ComboBarParameterPanel>();
        paramPanel->SetParentHeightPercent(1.0f);
        paramPanel->SetBaseWidth(300);
        return paramPanel;
    }, true);
    _CreateOverlaySelector(L"PP counter", SimplePPCounterConfig::OVERLAY_WINDOW_NAME, [=] {
        auto paramPanel = Create<SimplePPCounterParameterPanel>();
        paramPanel->SetParentHeightPercent(1.0f);
        paramPanel->SetBaseWidth(300);
        return paramPanel;
    }, true);

    auto creditsPanel = Create<FlexPanel>(FlexDirection::DOWN);
    creditsPanel->FillContainerWidth();
    creditsPanel->SetPadding(RECT{ 0, 5, 0, 5 });
    creditsPanel->SetSpacing(5);

    auto creditsLabel1 = Create<Label>(L"v2.0.0 | Made by Zenox");
    creditsLabel1->SetParentWidthPercent(1.0f);
    creditsLabel1->SetBaseWidth(-20);
    creditsLabel1->AutomaticHeight();
    creditsLabel1->SetHorizontalAlignment(Alignment::CENTER);
    creditsLabel1->SetTextSelectable(true);
    auto creditsLabel2 = Create<Label>(L"If you have any questions, you can message me directly on osu!, username: ZenoXLTU\nFor updates and FAQ check the app page: https://github.com/ZenoXi/osu-overlays");
    creditsLabel2->SetParentWidthPercent(1.0f);
    creditsLabel2->SetBaseWidth(-20);
    creditsLabel2->AutomaticHeight();
    creditsLabel2->SetHorizontalAlignment(Alignment::CENTER);
    creditsLabel2->SetFontStyle(DWRITE_FONT_STYLE_ITALIC);
    creditsLabel2->SetTextSelectable(true);
    creditsLabel2->SetWordWrap(true);
    creditsPanel->AddItem(std::move(creditsLabel1));
    creditsPanel->AddItem(std::move(creditsLabel2));

    _selectionPanel->AddItem(_loadingBar.get());
    _selectionPanel->AddItem(std::move(osuMemoryPanel));
    _selectionPanel->AddItem(_overlayListPanel.get());
    _selectionPanel->AddItem(std::move(creditsPanel));

    _mainPanel->AddItem(_selectionPanel.get());

    _basePanel->AddItem(_mainPanel.get());
    _basePanel->SetBackgroundColor(D2D1::ColorF(0));
    _basePanel->SubscribePostUpdate([=]() {
        _Update();
    }).Detach();
}

void zcom::EntryScene::_Update()
{
    std::optional<osu::DataProvider::ConnectionEvent> mostRecentEvent = std::nullopt;
    _dataProviderConnectionEvent->HandlePendingEvents([&](osu::DataProvider::ConnectionEvent event) {
        mostRecentEvent = event;
    });

    if (mostRecentEvent)
    {
        if (_waitingForDataProvider)
        {
            _waitingForDataProvider = false;
            _osuMemoryToggle->SetActive(true);
            _loadingBar->HideAnimation();
        }

        auto event = mostRecentEvent.value();
        if (event == osu::DataProvider::CONNECTION_SUCCESSFUL)
            _osuMemoryToggle->SetToggledOn(true);
        else if (event == osu::DataProvider::DISCONNECT_COMPLETED)
            _osuMemoryToggle->SetToggledOn(false);
        else if (event == osu::DataProvider::CONNECTION_FAILED)
        {
            _osuMemoryToggle->SetToggledOn(false);
            if (!_dataProviderSetupWindowId)
            {
                _OpenDataProviderSetup(true);
            }
            else
            {
                Handle<zwnd::Window> handle = _app->GetWindow(_dataProviderSetupWindowId.value());
                if (handle.Valid())
                    handle->Backend().Focus();
            }
        }
    }
}

void zcom::EntryScene::_CreateOverlaySelector(std::wstring buttonText, std::wstring windowClassName, std::function<std::unique_ptr<Component>()> parameterPanelInitFunc, bool dataProviderRequired)
{
    auto row = Create<Panel>();
    row->SetParentWidthPercent(1.0f);
    row->SetBaseSize(-20, 30);
    row->SetProperty(PROP_Shadow{});

    auto backPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    backPanel->FillContainerSize();
    backPanel->SetSpacing(3);
    auto statusIndicator = Create<Dummy>();
    statusIndicator->SetBaseSize(10, 30);
    statusIndicator->SetCornerRounding(3);
    statusIndicator->SetBackgroundColor(D2D1::ColorF(0x30B020));
    statusIndicator->SetVisible(false);
    auto button = Create<Button>(L"");
    button->SetParentWidthPercent(1.0f);
    button->SetBaseHeight(30);
    button->SetProperty(FlexShrink());
    button->SetSelectable(false);
    button->SetBorderVisibility(false);
    button->SetBackgroundColor(D2D1::ColorF(0x303030));
    button->SetButtonColor(D2D1::ColorF(0, 0.0f));
    button->SetButtonHoverColor(D2D1::ColorF(0xFFFFFF, 0.1f));
    button->SetButtonClickColor(D2D1::ColorF(0x000000, 0.1f));
    button->SetCornerRounding(3);
    button->SubscribeOnActivated([=] {
        if (_currentPropertyPanel)
            _mainPanel->RemoveItem(_currentPropertyPanel);

        auto paramPanel = parameterPanelInitFunc();
        _currentPropertyPanel = paramPanel.get();
        _mainPanel->AddItem(std::move(paramPanel));
    }).Detach();

    auto text = Create<Label>(buttonText);
    text->AutomaticSize();
    text->SetAlignment(Alignment::CENTER, Alignment::CENTER);
    text->SetInteractable(false);
    text->SetZIndex(1);
    if (dataProviderRequired)
    {
        auto osuMemoryIcon = Create<Image>(_window->resourceManager.GetImage("osu_memory"));
        osuMemoryIcon->SetBaseSize(30, 30);
        osuMemoryIcon->SetHorizontalOffsetPixels(-5);
        osuMemoryIcon->SetHorizontalAlignment(Alignment::END);
        osuMemoryIcon->SetPlacement(ImagePlacement::CENTER);
        osuMemoryIcon->SetInteractable(false);
        osuMemoryIcon->SetZIndex(1);
        row->AddItem(std::move(osuMemoryIcon));
    }
    row->AddItem(std::move(text));

    _OverlaySelector selector;
    selector.overlayWindowClassName = windowClassName;
    selector.statusIndicator = statusIndicator.get();
    _overlaySelectors.push_back(std::move(selector));

    backPanel->AddItem(std::move(statusIndicator));
    backPanel->AddItem(std::move(button));
    row->AddItem(std::move(backPanel));
    _overlayListPanel->AddItem(std::move(row));
}

void zcom::EntryScene::_HandleWindowCreatedEvent(zwnd::WindowId windowId, zwnd::WindowProperties props)
{
    for (auto& selector : _overlaySelectors)
    {
        if (selector.overlayWindowClassName == props.windowClassName)
        {
            selector.overlayWindowId = windowId;
            selector.statusIndicator->SetVisible(true);
            break;
        }
    }
}

void zcom::EntryScene::_HandleWindowClosedEvent(zwnd::WindowId windowId)
{
    for (auto& selector : _overlaySelectors)
    {
        if (selector.overlayWindowId.has_value() && selector.overlayWindowId.value() == windowId)
        {
            selector.overlayWindowId = std::nullopt;
            selector.statusIndicator->SetVisible(false);
            break;
        }
    }

    if (_dataProviderSetupWindowId && _dataProviderSetupWindowId.value() == windowId)
        _dataProviderSetupWindowId = std::nullopt;
}

void zcom::EntryScene::_OpenDataProviderSetup(bool showError)
{
    int width = 560;
    int height = 380;
    RECT mainWindowRect = _window->Backend().GetWindowRectangle();
    int mainWindowCenterX = (mainWindowRect.left + mainWindowRect.right) / 2;
    int mainWindowCenterY = (mainWindowRect.top + mainWindowRect.bottom) / 2;

    auto props = zwnd::WindowProperties()
        .WindowClassName(osu::DataProviderConfig::DATA_PROVIDER_SETUP_WINDOW_NAME)
        .InitialSize(width, height)
        .MinSize(500, 200)
        .InitialOffset(mainWindowCenterX - width / 2, mainWindowCenterY - height / 2)
        .DisableMaximizing()
        .DisableMinimizing()
        .DisableFastTooltips();

    _dataProviderSetupWindowId = _app->CreateChildWindow(
        _window->GetWindowId(),
        props,
        [=](zwnd::Window* wnd) {
            wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
            wnd->resourceManager.InitAllImages();

            wnd->LoadNonClientAreaScene<DefaultNonClientAreaScene>(nullptr);

            // Remove unnecessary caption elements
            DefaultTitleBarSceneOptions tbOpt;
            tbOpt.showMaximizeButton = false;
            tbOpt.showMinimizeButton = false;
            tbOpt.windowIconResourceName = "osu_memory";
            tbOpt.windowTitle = L"osu! data provider setup";
            tbOpt.darkMode = true;
            wnd->LoadTitleBarScene<DefaultTitleBarScene>(&tbOpt);

            DataProviderSetupSceneOptions opt;
            opt.showError = showError;
            wnd->LoadStartingScene<DataProviderSetupScene>(&opt);
        }
    );
}