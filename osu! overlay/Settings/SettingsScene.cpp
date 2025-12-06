#include "App.h"
#include "SharedContext.h"
#include "SettingsScene.h"
#include "IntegrationSettingsPanel.h"
#include "OverlaySettingsPanel.h"
#include "UpdateSettingsPanel.h"
#include "Components/Base/Dummy.h"
#include "IntegrationSettingsTabOptions.h"

void zcom::SettingsScene::Init(SceneOptionsBase* options)
{
    _openSettingsRequestSubscription = _app->Shared<SharedContext*>()->settingsWindow.SubscribeToOpenSettingsRequests();
    _openSettingsRequestSubscription->ResetSynchronousHandler([=](SettingsTab tab, std::optional<std::any> extraArgs) {
        _HandleOpenSettingsRequest(tab, extraArgs);
    });

    _mainPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _mainPanel->parentSize = { 1.0f, 1.0f };
    _mainPanel->size = { 0, -1 };
    _mainPanel->position = { 0, 1 };

    auto tabBackground = Create<Dummy>();
    tabBackground->parentSize = { 0.0f, 1.0f };
    tabBackground->size = { 200, 0 };
    tabBackground->backgroundColor = Color(0x101010);
    tabBackground->SetProperty(FlexIgnore());

    _mainPanel->AddItem(std::move(tabBackground));

    _CreateTabs();

    if (options)
    {
        SettingsSceneOptions opt = *reinterpret_cast<const SettingsSceneOptions*>(options);
        _HandleOpenSettingsRequest(opt.tab, opt.extraOptions);
    }
    else
    {
        _ShowSettingsTabPanel(std::move(Create<IntegrationSettingsPanel>(std::nullopt)));
        _currentTab = SettingsTab::INTEGRATION;
    }


    _basePanel->AddItem(_mainPanel.get());
    _basePanel->backgroundColor = Color(0);
}

void zcom::SettingsScene::Uninit()
{

}

void zcom::SettingsScene::_HandleOpenSettingsRequest(SettingsTab tab, std::optional<std::any> extraOptions)
{
    switch (tab)
    {
    case SettingsTab::INTEGRATION:
    {
        if (_currentTab != SettingsTab::INTEGRATION)
            _ShowSettingsTabPanel(std::move(Create<IntegrationSettingsPanel>(extraOptions)));
        else
            ((IntegrationSettingsPanel*)_settingsPanel.get())->Reinit(extraOptions);
        _currentTab = SettingsTab::INTEGRATION;
        break;
    }
    case SettingsTab::OVERLAY:
    {
        if (_currentTab != SettingsTab::OVERLAY)
            _ShowSettingsTabPanel(std::move(Create<OverlaySettingsPanel>(extraOptions)));
        else
            ((OverlaySettingsPanel*)_settingsPanel.get())->Reinit(extraOptions);
        _currentTab = SettingsTab::OVERLAY;
        break;
    }
    case SettingsTab::UPDATES:
    {
        if (_currentTab != SettingsTab::UPDATES)
            _ShowSettingsTabPanel(std::move(Create<UpdateSettingsPanel>(extraOptions)));
        _currentTab = SettingsTab::UPDATES;
    }
    default: break;
    }

    // TODO: might have to manually bring settings window to front
}

void zcom::SettingsScene::_CreateTabs()
{
    auto integrationTabButton = _CreateTabButton(L"Integration", _window->resourceManager.GetImage("osu_web"), SettingsTab::INTEGRATION);
    integrationTabButton->SubscribeOnActivated([=]() {
        if (_currentTab != SettingsTab::INTEGRATION)
        {
            _ShowSettingsTabPanel(std::move(Create<IntegrationSettingsPanel>(std::nullopt)));
            _currentTab = SettingsTab::INTEGRATION;
        }
    }).Detach();

    auto overlayTabButton = _CreateTabButton(L"Overlay", _window->resourceManager.GetImage("overlay"), SettingsTab::OVERLAY);
    overlayTabButton->SubscribeOnActivated([=]() {
        if (_currentTab != SettingsTab::OVERLAY)
        {
            _ShowSettingsTabPanel(std::move(Create<OverlaySettingsPanel>(std::nullopt)));
            _currentTab = SettingsTab::OVERLAY;
        }
    }).Detach();
    
    auto updatesTabButton = _CreateTabButton(L"Updates", _window->resourceManager.GetImage("update"), SettingsTab::UPDATES);
    updatesTabButton->Image()->tintColor = Color(0xE3C04B);
    updatesTabButton->SubscribeOnActivated([=]() {
        if (_currentTab != SettingsTab::UPDATES)
        {
            _ShowSettingsTabPanel(std::move(Create<UpdateSettingsPanel>(std::nullopt)));
            _currentTab = SettingsTab::UPDATES;
        }
    }).Detach();

    _mainPanel->AddItem(std::move(integrationTabButton));
    _mainPanel->AddItem(std::move(overlayTabButton));
    _mainPanel->AddItem(std::move(updatesTabButton));
}

std::unique_ptr<zcom::Button> zcom::SettingsScene::_CreateTabButton(std::wstring text, std::optional<Bitmap> icon, SettingsTab tab)
{
    auto tabButton = Create<Button>(text, ButtonPreset::NO_EFFECTS);
    tabButton->size = { 200, 30 };
    tabButton->selectable = false;
    tabButton->ValueFromButtonState(tabButton->buttonColor, Color(0, 0.0f), Color(0xFFFFFF, 0.1f), Color(0, 0.1f));
    tabButton->Image()->image = icon;
    tabButton->Image()->parentSize = { 0.0f, 0.0f };
    tabButton->Image()->size = { 30, 30 };
    tabButton->Image()->imagePlacement = ImagePlacement::CENTER;
    tabButton->Image()->snapToPixels = true;
    tabButton->Label()->parentSize = { 1.0f, 1.0f };
    tabButton->Label()->size = { -30, 0 };
    tabButton->Label()->padding = { 1.0f, 0.0f, 0.0f, 0.0f };
    tabButton->Label()->xAlign = Alignment::END;
    tabButton->Label()->xTextAlign = TextAlignment::LEADING;
    tabButton->activation = ButtonActivation::PRESS;
    tabButton->SetComputedStyle("selectedTab", [tab](Component* component, SettingsTab currentTab) {
        Button* button = (Button*)component;
        if (currentTab == tab)
        {
            button->backgroundColor = Color(0x202020);
            button->zIndex = 10;
            button->SetProperty(Shadow().WithGroup(0));
            button->buttonColor.ResetComputer();
            button->buttonColor = Color::None();
            button->cursorIcon = zwnd::CursorIcon::ARROW;
        }
        else
        {
            button->backgroundColor = Color(0, 0.0f);
            button->zIndex = 0;
            button->RemoveProperty<Shadow>();
            button->ValueFromButtonState(button->buttonColor, Color(0, 0.0f), Color(0xFFFFFF, 0.1f), Color(0, 0.1f));
            button->cursorIcon = zwnd::CursorIcon::HAND;
        }
    }, _currentTab);
    return tabButton;
}

void zcom::SettingsScene::_ShowSettingsTabPanel(std::unique_ptr<Component> panel)
{
    panel->parentSize = { 1.0f, 1.0f };
    panel->size = { -200, 0 };
    panel->xAlign = Alignment::END;
    panel->SetProperty(FlexIgnore());
    panel->backgroundColor = Color(0x202020);
    panel->SetProperty(Shadow().WithGroup(0));
    panel->zIndex = 10;
    _mainPanel->RemoveItem(_settingsPanel.get());
    _settingsPanel = std::move(panel);
    _mainPanel->AddItem(_settingsPanel.get());
}
