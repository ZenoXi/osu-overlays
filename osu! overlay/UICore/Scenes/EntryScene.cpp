#include "App.h" // App.h must be included first
#include "SharedContext.h"
#include "Window/Window.h"
#include "EntryScene.h"
#include "DefaultNonClientAreaScene.h"
#include "DefaultTitleBarScene.h"
#include "OsuDataProvider/DataProviderConfig.h"
#include "OsuDataProvider/DataProviderSetupScene.h"
#include "Components/Base/ScrollPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Dummy.h"
#include "Shared/Styles/Styles.h"
#include "Settings/IntegrationSettingsTabOptions.h"
#include "Overlays/pp-counter/PPCounterOverlay.h"
#include "Overlays/ur-counter/URCounterOverlay.h"
#include "Overlays/rt-leaderboard/RTLeaderboardOverlay.h"
#include "Overlays/combo-counter/ComboCounterOverlay.h"
#include "Overlays/cursor-trail/CursorTrailOverlay.h"
#include "Overlays/smoke-sim/smoke-trail/SmokeTrailOverlay.h"
#include "Overlays/smoke-sim/enhanced-smoke/EnhancedSmokeOverlay.h"

void zcom::EntryScene::Init(SceneOptionsBase* options)
{
    EntrySceneOptions opt;
    if (options)
    {
        opt = *reinterpret_cast<const EntrySceneOptions*>(options);
    }

    _dataProviderConnectionEvent = _app->Shared<SharedContext*>()->dataProvider.SubscribeOnConnectionEvent();
    _waitingForDataProvider = true;

    _mainPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    _mainPanel->parentSize = { 1.0f, 1.0f };
    _mainPanel->size = { 0, -1 };
    _mainPanel->position = { 0, 1 };
    _mainPanel->spacing = 1;

    _selectionPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _selectionPanel->parentSize = { 1.0f, 1.0f };
    _selectionPanel->SetProperty(FlexShrink());
    _selectionPanel->backgroundColor = Color(0x1A1A1A);

    _loadingBar = Create<LoadingAnimation>();
    _loadingBar->parentSize = { 1.0f, 0.0f };
    _loadingBar->size = { 0, 1 };
    _loadingBar->mainColor = Color(0x5421FF, 0.5f);
    //_loadingBar->SetAccentColor(Color(0x8A2BFF, 1.0f));
    _loadingBar->accentColor = Color(0xF966AB, 1.0f);

    auto osuMemoryPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    osuMemoryPanel->parentSize = { 1.0f, 0.0f };
    osuMemoryPanel->size = { 0, 40 };
    osuMemoryPanel->spacing = 10;
    osuMemoryPanel->padding = { 10, 10, 10, 0 };
    osuMemoryPanel->itemAlignment = Alignment::CENTER;
    auto osuMemoryFiller = Create<Dummy>();
    osuMemoryFiller->size = { 0, 1 };
    osuMemoryFiller->SetProperty(FlexGrow());
    auto osuMemoryIcon = Create<Image>(_window->resourceManager.GetImage("osu_memory"));
    osuMemoryIcon->size = { 26, 26 };
    osuMemoryIcon->position = { 5, 0 };
    auto osuMemoryLabel = Create<Label>(L"osu! data provider");
    osuMemoryLabel->size = { 0, 20 };
    osuMemoryLabel->autoWidth = true;
    osuMemoryLabel->yTextAlign = Alignment::CENTER;
    auto osuMemorySeparator = Create<Dummy>();
    osuMemorySeparator->size = { 1, 22 };
    osuMemorySeparator->backgroundColor = Color(0x404040);
    osuMemorySeparator->border.cornerRadius = 0.5f;
    _osuMemoryToggle = Create<Toggle>(false);
    _osuMemoryToggle->size = { 40, 22 };
    _osuMemoryToggle->marginToBorder = 4.0f;
    _osuMemoryToggle->border.cornerRadius = 11.0f;
    _osuMemoryToggle->border.visible = false;
    _osuMemoryToggle->border.selectedColor = Color(0, 0.0f);
    _osuMemoryToggle->toggledOnBackgroundColor = Color(0x308020);
    _osuMemoryToggle->toggledOffBackgroundColor = Color(0x303030);
    _osuMemoryToggle->toggledOnAnchorColor = Color(0xC0C0C0);
    _osuMemoryToggle->toggledOffAnchorColor = Color(0xC0C0C0);
    _osuMemoryToggle->SetProperty(Shadow{});
    _osuMemoryToggle->toggledOn = _app->Shared<SharedContext*>()->dataProvider.Ready();
    _osuMemoryToggle->SubscribeOnToggled([=](bool* newValue) {
        if (*newValue)
        {
            std::wstring url = _app->config.GetConfigValue(osu::DataProviderConfig::URL, Config::ADD_AND_SAVE_IF_MISSING);
            if (_app->Shared<SharedContext*>()->dataProvider.Connect(url))
            {
                _waitingForDataProvider = true;
                _osuMemoryToggle->disabled = true;
                _loadingBar->showAnimation = true;
                *newValue = false;
            }
        }
        else
        {
            if (_app->Shared<SharedContext*>()->dataProvider.Disconnect())
            {
                _waitingForDataProvider = true;
                _osuMemoryToggle->disabled = true;
                _loadingBar->showAnimation = true;
                *newValue = true;
            }
        }
    }).Detach();
    auto osuMemoryButton = Create<Button>(L"");
    osuMemoryButton->size = { 30, 30 };
    osuMemoryButton->border.visible = false;
    osuMemoryButton->backgroundColor = Color(0x303030);
    osuMemoryButton->ValueFromButtonState<Color>(osuMemoryButton->buttonColor, Color(0, 0.0f), Color(0xFFFFFF, 0.1f), Color(0x000000, 0.1f));
    osuMemoryButton->Image()->image = _window->resourceManager.GetImage("settings_22x22");
    osuMemoryButton->Image()->imagePlacement = ImagePlacement::CENTER;
    osuMemoryButton->Image()->tintColor = Color(0xC0C0C0);
    osuMemoryButton->border.cornerRadius = 3.0f;
    osuMemoryButton->border.selectedColor = Color();
    osuMemoryButton->SetProperty(Shadow{});
    osuMemoryButton->SubscribeOnActivated([=]() {
        _app->Shared<SharedContext*>()->settingsWindow.OpenSettings(SettingsTab::INTEGRATION);
    }).Detach();
    osuMemoryPanel->AddItem(std::move(osuMemoryFiller));
    osuMemoryPanel->AddItem(std::move(osuMemoryIcon));
    osuMemoryPanel->AddItem(std::move(osuMemoryLabel));
    osuMemoryPanel->AddItem(std::move(osuMemorySeparator));
    osuMemoryPanel->AddItem(_osuMemoryToggle.get());
    osuMemoryPanel->AddItem(std::move(osuMemoryButton));

    _overlayListPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _overlayListPanel->parentSize = { 1.0f, 0.0f };
    _overlayListPanel->SetProperty(FlexGrow());
    _overlayListPanel->spacing = 3;
    _overlayListPanel->itemAlignment = Alignment::CENTER;

    auto overlayListHeader = Create<FlexPanel>(FlexDirection::RIGHT);
    overlayListHeader->parentSize = { 1.0f, 0.0f };
    overlayListHeader->size = { 0, 30 };
    overlayListHeader->padding = { 12, 0, 12, 0 };
    overlayListHeader->spacing = 5;
    auto headerLabel = Create<Label>(L"Available overlays");
    headerLabel->autoWidth = true;
    headerLabel->size = { 0, 30 };
    headerLabel->yTextAlign = Alignment::CENTER;
    headerLabel->fontSize = 14.0f;
    headerLabel->fontColor = Color(0xCCCCCC);
    headerLabel->font = L"Arial";
    auto headerSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(headerSeparator.get());
    //headerSeparator->SetBaseHeight(1);
    headerSeparator->position = { 0, 1 };
    headerSeparator->yAlign = Alignment::CENTER;
    headerSeparator->SetProperty(FlexShrink());
    //headerSeparator->backgroundColor = D2D1::ColorF(0x404040);
    overlayListHeader->AddItem(std::move(headerLabel));
    overlayListHeader->AddItem(std::move(headerSeparator));

    _overlayListPanel->AddItem(std::move(overlayListHeader));

    _registeredOverlays.push_back(std::make_shared<const RTLeaderboardOverlay>());
    _registeredOverlays.push_back(std::make_shared<const PPCounterOverlay>());
    _registeredOverlays.push_back(std::make_shared<const URCounterOverlay>());
    _registeredOverlays.push_back(std::make_shared<const ComboCounterOverlay>());
    _registeredOverlays.push_back(std::make_shared<const CursorTrailOverlay>());
    _registeredOverlays.push_back(std::make_shared<const EnhancedSmokeOverlay>());
    _registeredOverlays.push_back(std::make_shared<const SmokeTrailOverlay>());

    for (auto& overlay : _registeredOverlays)
    {
        _OverlaySelector selector;
        selector.overlayView = std::make_unique<OverlayView>(overlay->Id(), _basePanel);

        selector.selectorComponent = Create<Panel>();
        selector.selectorComponent->parentSize = { 1.0f, 0.0f };
        selector.selectorComponent->size = { -20, 30 };
        selector.selectorComponent->SetProperty(Shadow{});

        auto backPanel = Create<FlexPanel>(FlexDirection::RIGHT);
        backPanel->parentSize = { 1.0f, 1.0f };
        backPanel->spacing = 3;
        auto onOffButton = Create<Button>(L"");
        onOffButton->size = { 30, 30 };
        onOffButton->selectable = false;
        onOffButton->border.visible = false;
        onOffButton->backgroundColor.ComputedFrom([](bool overlayEnabled) { return overlayEnabled ? Color(0x307E20) : Color(0x303030); }, selector.overlayView->enabled_);
        onOffButton->ValueFromButtonState<Color>(onOffButton->buttonColor, Color(0, 0.0f), Color(0xFFFFFF, 0.1f), Color(0x000000, 0.1f));
        onOffButton->border.cornerRadius = 3;
        onOffButton->Image()->image = _window->resourceManager.GetImage("on_off");
        onOffButton->Image()->imagePlacement = ImagePlacement::CENTER;
        onOffButton->Image()->tintColor = Color(0xCCCCCC);
        onOffButton->SubscribeOnActivated([=, overlayView = selector.overlayView.get()]() {
            if (!overlayView->enabled_)
                _app->Shared<SharedContext*>()->overlayManager.EnableOverlay(overlay);
            else
                _app->Shared<SharedContext*>()->overlayManager.DisableOverlay(overlay->Id());
        }).Detach();
        auto button = Create<Button>(overlay->GetTitle());
        button->parentSize = { 1.0f, 0.0f };
        button->size = { 0, 30 };
        button->SetProperty(FlexShrink());
        button->selectable = false;
        button->border.visible = false;
        button->backgroundColor = Color(0x303030);
        button->ValueFromButtonState<Color>(button->buttonColor, Color(0, 0.0f), Color(0xFFFFFF, 0.1f), Color(0x000000, 0.1f));
        button->border.cornerRadius = 3;
        button->SubscribeOnActivated([=] {
            if (_currentPropertyPanel)
                _mainPanel->RemoveItem(_currentPropertyPanel);

            auto paramPanel = overlay->CreateSetupComponent(overlay, _mainPanel.get());
            _currentPropertyPanel = paramPanel.get();
            _mainPanel->AddItem(std::move(paramPanel));
        }).Detach();

        if (overlay->RequiresGameData())
        {
            auto osuMemoryIcon = Create<Image>(_window->resourceManager.GetImage("osu_memory"));
            osuMemoryIcon->size = { 30, 30 };
            osuMemoryIcon->position = { -5, 0 };
            osuMemoryIcon->xAlign = Alignment::END;
            osuMemoryIcon->imagePlacement = ImagePlacement::CENTER;
            osuMemoryIcon->interactable = false;
            osuMemoryIcon->zIndex = 1;
            selector.selectorComponent->AddItem(std::move(osuMemoryIcon));
        }

        backPanel->AddItem(std::move(onOffButton));
        backPanel->AddItem(std::move(button));
        selector.selectorComponent->AddItem(std::move(backPanel));
        _overlayListPanel->AddItem(selector.selectorComponent.get());

        _overlaySelectors.push_back(std::move(selector));
    }

    auto creditsPanel = Create<FlexPanel>(FlexDirection::DOWN);
    creditsPanel->parentSize = { 1.0f, 0.0f };
    creditsPanel->autoHeight = true;
    creditsPanel->padding = { 0, 5, 0, 5 };
    creditsPanel->spacing = 5;
    auto creditsLabel1 = Create<Label>(L"v" + string_to_wstring(OVERLAY_ENGINE_VERSION.ToString()) + L" | Made by Zenox");
    creditsLabel1->parentSize = { 1.0f, 0.0f };
    creditsLabel1->size = { -20, 0 };
    creditsLabel1->autoHeight = true;
    creditsLabel1->xAlign = Alignment::CENTER;
    creditsLabel1->textSelectable = true;
    creditsLabel1->SubscribePostLeftReleased([=](Component*, std::vector<EventContext::Params>, std::optional<Point>) {
        _window->Backend().SetWindowRectangle({ 0, 0, 300, 300 });
    }).Detach();
    auto creditsLabel2 = Create<Label>(L"If you have any questions, you can message me directly on osu!, username: ZenoXLTU\nFor updates and FAQ check the app page: https://github.com/ZenoXi/osu-overlays");
    creditsLabel2->parentSize = { 1.0f, 0.0f };
    creditsLabel2->size = { -20, 0 };
    creditsLabel2->autoHeight = true;
    creditsLabel2->xAlign = Alignment::CENTER;
    creditsLabel2->fontStyle = FontStyle::ITALIC;
    creditsLabel2->textSelectable = true;
    creditsLabel2->wordWrapping = WordWrapping::WRAP;
    creditsPanel->AddItem(std::move(creditsLabel1));
    creditsPanel->AddItem(std::move(creditsLabel2));

    _selectionPanel->AddItem(_loadingBar.get());
    _selectionPanel->AddItem(std::move(osuMemoryPanel));
    _selectionPanel->AddItem(_overlayListPanel.get());
    _selectionPanel->AddItem(std::move(creditsPanel));

    _mainPanel->AddItem(_selectionPanel.get());

    _basePanel->AddItem(_mainPanel.get());
    _basePanel->backgroundColor = Color(0);
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
            _osuMemoryToggle->disabled = false;
            _loadingBar->showAnimation = false;
        }

        auto event = mostRecentEvent.value();
        if (event == osu::DataProvider::CONNECTION_SUCCESSFUL)
            _osuMemoryToggle->toggledOn = true;
        else if (event == osu::DataProvider::DISCONNECT_COMPLETED)
            _osuMemoryToggle->toggledOn = false;
        else if (event == osu::DataProvider::CONNECTION_FAILED)
        {
            IntegrationSettingsTabOptions opt;
            opt.showDataProviderError = true;
            _app->Shared<SharedContext*>()->settingsWindow.OpenSettings(SettingsTab::INTEGRATION, std::make_any<IntegrationSettingsTabOptions>(opt));
        }
    }
}
