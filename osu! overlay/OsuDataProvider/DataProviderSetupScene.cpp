#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "DataProviderSetupScene.h"
#include "DataProviderConfig.h"
#include "Components/Base/Image.h"
#include "Components/Base/TextInput.h"
#include "Shared/Styles/Styles.h"

void zcom::DataProviderSetupScene::Init(SceneOptionsBase* options)
{
    DataProviderSetupSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const DataProviderSetupSceneOptions*>(options);

    _dataProviderConnectionEvent = _app->dataProvider.SubscribeOnConnectionEvent();
    _waitingForDataProvider = true;

    _mainPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    _mainPanel->FillContainerSize();
    _mainPanel->SetSpacing(1);
    _mainPanel->SetBaseHeight(-1);
    _mainPanel->SetVerticalOffsetPixels(1);

    auto setupPanel = Create<FlexPanel>(FlexDirection::DOWN);
    setupPanel->FillContainerSize();
    setupPanel->SetProperty(FlexShrink());
    setupPanel->SetBackgroundColor(D2D1::ColorF(0x1A1A1A));
    setupPanel->SetSpacing(10);
    setupPanel->SetPadding({ 0, 0, 0, 10 });
    _loadingBar = Create<zcom::LoadingAnimation>();
    _loadingBar->SetParentWidthPercent(1.0f);
    _loadingBar->SetBaseHeight(1);
    _loadingBar->SetMainColor(D2D1::ColorF(0x5421FF, 0.5f));
    _loadingBar->SetAccentColor(D2D1::ColorF(0xF966AB, 1.0f));
    auto statusRow = Create<FlexPanel>(FlexDirection::RIGHT);
    statusRow->FillContainerWidth();
    statusRow->SetSpacing(10);
    statusRow->SetPadding({ 10, 0, 10, 0 });
    statusRow->SetItemAlignment(Alignment::CENTER);
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
            _app->config.SetValue(osu::DataProviderConfig::URL.name, _urlInput->Text()->GetText());
            if (_app->dataProvider.Connect(_urlInput->Text()->GetText()))
            {
                _waitingForDataProvider = true;
                _osuMemoryToggle->SetActive(false);
                _loadingBar->ShowAnimation();
                _errorLabel->SetVisible(false);
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
                _errorLabel->SetVisible(false);
                *newValue = true;
            }
        }
    }).Detach();
    _statusLabel = Create<Label>(L"Disconnected");
    _statusLabel->SetBaseHeight(18);
    _statusLabel->AutomaticWidth();
    _statusLabel->SetVerticalTextAlignment(Alignment::CENTER);
    _statusLabel->SetProperty(FlexGrow());
    auto helpButton = Create<Button>(L"?");
    NeutralButtonStyle::Apply(helpButton.get());
    helpButton->SetBaseSize(22, 22);
    helpButton->Label()->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD);
    helpButton->SubscribeOnActivated([=]() {
        bool newValue = !_descriptionScrollWrapper->GetVisible();
        _descriptionScrollWrapper->SetVisible(newValue);
        _app->config.SetIntValue(osu::DataProviderConfig::SHOW_HELP_PANEL.name, newValue);
    }).Detach();
    statusRow->AddItem(_osuMemoryToggle.get());
    statusRow->AddItem(_statusLabel.get());
    statusRow->AddItem(std::move(helpButton));
    auto separator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(separator.get(), 10);
    auto urlRow = Create<FlexPanel>(FlexDirection::RIGHT);
    urlRow->FillContainerWidth();
    urlRow->SetSpacing(10);
    urlRow->SetPadding({ 10, 0, 10, 0 });
    auto urlLabel = Create<Label>(L"URL:");
    urlLabel->SetBaseHeight(26);
    urlLabel->AutomaticWidth();
    urlLabel->SetVerticalTextAlignment(Alignment::CENTER);
    _urlInput = Create<TextInput>();
    _urlInput->SetBaseHeight(26);
    _urlInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    _urlInput->SetCornerRounding(2.0f);
    _urlInput->SetProperty(FlexGrow());
    _urlInput->Text()->SetText(_app->config.GetConfigValue(osu::DataProviderConfig::URL));
    urlRow->AddItem(std::move(urlLabel));
    urlRow->AddItem(_urlInput.get());
    auto spacer = Create<Dummy>();
    spacer->SetBaseWidth(1);
    spacer->SetBackgroundColor(D2D1::ColorF(0, 0.0f));
    spacer->SetProperty(FlexGrow());
    _errorLabel = Create<Label>(L"Failed to connect to the data provider. Make sure that both the game and the data provider are running without errors and whether the entered URL is correct");
    _errorLabel->SetParentWidthPercent(1.0f);
    _errorLabel->SetBaseWidth(-20);
    _errorLabel->AutomaticHeight();
    _errorLabel->SetHorizontalAlignment(Alignment::CENTER);
    _errorLabel->SetWordWrap(true);
    _errorLabel->SetHorizontalTextAlignment(TextAlignment::JUSTIFIED);
    _errorLabel->SetPadding({ 10.0f, 5.0f, 10.0f, 5.0f });
    _errorLabel->SetBackgroundColor(D2D1::ColorF(0x803030));
    _errorLabel->SetCornerRounding(3);
    _errorLabel->SetProperty(PROP_Shadow());
    if (!opt.showError)
        _errorLabel->SetVisible(false);

    setupPanel->AddItem(_loadingBar.get());
    setupPanel->AddItem(std::move(statusRow));
    setupPanel->AddItem(std::move(separator));
    setupPanel->AddItem(std::move(urlRow));
    setupPanel->AddItem(std::move(spacer));
    setupPanel->AddItem(_errorLabel.get());

    _descriptionScrollWrapper = Create<ScrollPanel>();
    _descriptionScrollWrapper->SetParentHeightPercent(1.0f);
    _descriptionScrollWrapper->SetBaseWidth(250);
    _descriptionScrollWrapper->SetBackgroundColor(D2D1::ColorF(0x202020));
    _descriptionScrollWrapper->Scrollable(Scrollbar::VERTICAL, true);
    _descriptionScrollWrapper->ScrollBackgroundVisible(Scrollbar::VERTICAL, true);
    auto descriptionPanel = Create<FlexPanel>(FlexDirection::DOWN);
    descriptionPanel->FillContainerWidth();
    descriptionPanel->SetPadding(RECT{ 10, 5, 10, 15 });
    descriptionPanel->SetSpacing(5);
    auto descriptionTopRow = Create<FlexPanel>(FlexDirection::RIGHT);
    descriptionTopRow->FillContainerWidth();
    descriptionTopRow->SetItemAlignment(Alignment::CENTER);
    descriptionTopRow->SetSpacing(10);
    auto descriptionLabel1 = Create<Label>(L"Some features are marked with the following icon:");
    descriptionLabel1->SetParentWidthPercent(1.0f);
    descriptionLabel1->SetProperty(FlexShrink());
    descriptionLabel1->AutomaticHeight();
    descriptionLabel1->SetTextSelectable(true);
    descriptionLabel1->SetWordWrap(true);
    descriptionLabel1->SetHorizontalTextAlignment(TextAlignment::JUSTIFIED);
    auto descriptionIcon = Create<Image>(_window->resourceManager.GetImage("osu_memory"));
    descriptionIcon->SetBaseSize(26, 26);
    descriptionTopRow->AddItem(std::move(descriptionLabel1));
    descriptionTopRow->AddItem(std::move(descriptionIcon));
    auto descriptionSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(descriptionSeparator.get());
    auto descriptionLabel2 = Create<Label>(L"These features require access to osu! in-game data to work, which is provided using an external memory reader. Currently only gosumemory is supported and you can get it from here: https://github.com/l3lackShark/gosumemory");
    descriptionLabel2->SetParentWidthPercent(1.0f);
    descriptionLabel2->AutomaticHeight();
    descriptionLabel2->SetTextSelectable(true);
    descriptionLabel2->SetWordWrap(true);
    descriptionLabel2->SetHorizontalTextAlignment(TextAlignment::JUSTIFIED);
    auto descriptionLabel3 = Create<Label>(L"After starting both osu! and the memory reader, you can connect to the specified URL. If you didn't change the memory reader config, you shouldn't need to edit the URL");
    descriptionLabel3->SetParentWidthPercent(1.0f);
    descriptionLabel3->AutomaticHeight();
    descriptionLabel3->SetTextSelectable(true);
    descriptionLabel3->SetWordWrap(true);
    descriptionLabel3->SetHorizontalTextAlignment(TextAlignment::JUSTIFIED);
    auto descriptionCloseButton = Create<Button>(L"Got it!");
    NeutralButtonStyle::Apply(descriptionCloseButton.get());
    descriptionCloseButton->SetBaseSize(80, 30);
    descriptionCloseButton->SetHorizontalAlignment(Alignment::CENTER);
    descriptionCloseButton->SetVerticalOffsetPixels(5);
    descriptionCloseButton->SubscribeOnActivated([=]() {
        _descriptionScrollWrapper->SetVisible(false);
        _app->config.SetIntValue(osu::DataProviderConfig::SHOW_HELP_PANEL.name, false);
    }).Detach();
    descriptionPanel->AddItem(std::move(descriptionTopRow));
    descriptionPanel->AddItem(std::move(descriptionSeparator));
    descriptionPanel->AddItem(std::move(descriptionLabel2));
    descriptionPanel->AddItem(std::move(descriptionLabel3));
    descriptionPanel->AddItem(std::move(descriptionCloseButton));
    _descriptionScrollWrapper->AddItem(std::move(descriptionPanel));
    _descriptionScrollWrapper->SetVisible(_app->config.GetIntConfigValue(osu::DataProviderConfig::SHOW_HELP_PANEL, Config::ADD_IF_MISSING));

    _mainPanel->AddItem(std::move(setupPanel));
    _mainPanel->AddItem(_descriptionScrollWrapper.get());

    _basePanel->AddItem(_mainPanel.get());
    _basePanel->SetBackgroundColor(D2D1::ColorF(0));
    _basePanel->SubscribePostUpdate([=]() {
        _Update();
    }).Detach();
}

void zcom::DataProviderSetupScene::Uninit()
{
    _app->config.SetValue(osu::DataProviderConfig::URL.name, _urlInput->Text()->GetText());
}

void zcom::DataProviderSetupScene::_Update()
{
    //_urlInput->InvokeRedraw();

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
        {
            _osuMemoryToggle->SetToggledOn(true);
            _statusLabel->SetText(L"Connected");
        }
        else if (event == osu::DataProvider::DISCONNECT_COMPLETED)
        {
            _osuMemoryToggle->SetToggledOn(false);
            _statusLabel->SetText(L"Disconnected");
        }
        else if (event == osu::DataProvider::CONNECTION_FAILED)
        {
            _osuMemoryToggle->SetToggledOn(false);
            _errorLabel->SetVisible(true);
        }
    }
}