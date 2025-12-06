#include "App.h" // If not included first, results in winsock redefinition errors
#include "IntegrationSettingsPanel.h"
#include "SharedContext.h"
#include "Window/Window.h"

#include "OsuWebApi/WebApiConfig.h"
#include "OsuDataProvider/DataProviderConfig.h"

#include "Components/Base/Image.h"
#include "Components/Base/Toggle.h"
#include "Components/Base/Button.h"
#include "Components/Base/Dummy.h"
#include "Shared/Components/LoadingAnimation.h"

void zcom::IntegrationSettingsPanel::Init(std::optional<std::any> extraOptions)
{
    ScrollPanel::Init();

    yScrollbar.scrollable = true;
    yScrollbar.backgroundVisible = true;

    _dataProviderConnectionEvent = _scene->GetApp()->Shared<SharedContext*>()->dataProvider.SubscribeOnConnectionEvent();
    _showDataProviderHelp = _scene->GetApp()->config.GetIntConfigValue(osu::DataProviderConfig::SHOW_HELP_PANEL, Config::ADD_IF_MISSING);
    _showWebApiHelp = _scene->GetApp()->config.GetIntConfigValue(webapi::WebApiConfig::SHOW_HELP_PANEL, Config::ADD_IF_MISSING);
    Reinit(extraOptions);

    _contentPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _contentPanel->parentSize = { 1.0f, 0.0f };
    _contentPanel->autoHeight = true;

    _CreateDataProviderSection();
    _CreateWebSection();
    AddItem(_contentPanel.get());
}

void zcom::IntegrationSettingsPanel::Reinit(std::optional<std::any> extraOptions)
{
    IntegrationSettingsTabOptions opt{};
    if (extraOptions)
        opt = std::any_cast<IntegrationSettingsTabOptions>(extraOptions.value());
    _showDataProviderError = opt.showDataProviderError;
}

void zcom::IntegrationSettingsPanel::_CreateDataProviderSection()
{
    auto dataSectionHeader = Create<SectionHeader>(L"osu! real-time data provider", Rect{ 5, 0, 10, 0 });
    dataSectionHeader->GetSeparator()->showAnimation.ComputedFrom([](bool waiting) { return waiting; }, _waitingForDataProvider);
    auto dataSectionImage = Create<Image>(_scene->GetWindow()->resourceManager.GetImage("osu_memory"));
    dataSectionImage->size = { 30, 30 };
    dataSectionImage->imagePlacement = ImagePlacement::CENTER;
    dataSectionImage->snapToPixels = true;
    dataSectionHeader->InsertItem(std::move(dataSectionImage), 0);

    auto dataSectionPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    dataSectionPanel->parentSize = { 1.0f, 0.0f };
    dataSectionPanel->autoHeight = true;
    dataSectionPanel->padding = { 10, 0, 0, 20 };

    auto setupPanel = Create<FlexPanel>(FlexDirection::DOWN);
    setupPanel->autoHeight = true;
    setupPanel->SetProperty(FlexGrow());
    setupPanel->padding.ComputedFrom([](bool showHelp) { return showHelp ? Rect{ 0, 0, 0, 10 } : Rect{}; }, _showDataProviderHelp);
    auto statusRow = Create<FlexPanel>(FlexDirection::RIGHT);
    statusRow->parentSize = { 1.0f, 0.0f };
    statusRow->autoHeight = true;
    statusRow->spacing = 10;
    statusRow->padding = { 10, 10, 10, 10 };
    statusRow->itemAlignment = Alignment::CENTER;
    auto osuMemoryToggle = Create<Toggle>(false);
    osuMemoryToggle->size = { 40, 22 };
    osuMemoryToggle->border.cornerRadius = 11.0f;
    osuMemoryToggle->marginToBorder = 4.0f;
    osuMemoryToggle->border.visible = false;
    osuMemoryToggle->border.selectedColor = Color(0, 0.0f);
    osuMemoryToggle->toggledOnBackgroundColor = Color(0x308020);
    osuMemoryToggle->toggledOffBackgroundColor = Color(0x303030);
    osuMemoryToggle->toggledOnAnchorColor = Color(0xC0C0C0);
    osuMemoryToggle->toggledOffAnchorColor = Color(0xC0C0C0);
    osuMemoryToggle->SetProperty(Shadow{});
    osuMemoryToggle->disabled.ComputedFrom([](bool waiting) { return waiting; }, _waitingForDataProvider);
    osuMemoryToggle->toggledOn.ComputedFrom([](bool connected) { return connected; }, _dataProviderConnected);
    osuMemoryToggle->toggledOn = _scene->GetApp()->Shared<SharedContext*>()->dataProvider.Ready();
    osuMemoryToggle->SubscribeOnToggled([=](bool* newValue) {
        if (*newValue)
        {
            _scene->GetApp()->config.SetValue(osu::DataProviderConfig::URL.name, _dataProviderUrlInput->text);
            if (_scene->GetApp()->Shared<SharedContext*>()->dataProvider.Connect(_dataProviderUrlInput->text))
            {
                _waitingForDataProvider = true;
                _showDataProviderError = false;
                *newValue = false;
            }
        }
        else
        {
            if (_scene->GetApp()->Shared<SharedContext*>()->dataProvider.Disconnect())
            {
                _waitingForDataProvider = true;
                *newValue = true;
            }
        }
    }).Detach();
    auto statusLabel = Create<Label>(L"Disconnected");
    statusLabel->size = { 0, 18 };
    statusLabel->yTextAlign = Alignment::CENTER;
    statusLabel->SetProperty(FlexGrow());
    statusLabel->text.ComputedFrom([](bool connected) -> std::wstring { return connected ? L"Connected" : L"Disconnected"; }, _dataProviderConnected);
    auto helpButton = Create<Button>(L"?");
    NeutralButtonStyle::Apply(helpButton.get());
    helpButton->size = { 22, 22 };
    helpButton->Label()->fontWeight = FontWeight::BOLD;
    helpButton->visible.ComputedFrom([](bool showHelp) { return !showHelp; }, _showDataProviderHelp);
    helpButton->SubscribeOnActivated([=]() {
        _showDataProviderHelp = !_showDataProviderHelp;
    }).Detach();
    statusRow->AddItem(std::move(osuMemoryToggle));
    statusRow->AddItem(std::move(statusLabel));
    statusRow->AddItem(std::move(helpButton));
    auto urlRow = Create<FlexPanel>(FlexDirection::RIGHT);
    urlRow->parentSize = { 1.0f, 0.0f };
    urlRow->autoHeight = true;
    urlRow->spacing = 10;
    urlRow->padding = { 10, 10, 10, 10 };
    auto urlLabel = Create<Label>(L"URL:");
    urlLabel->size = { 0, 26 };
    urlLabel->autoWidth = true;
    urlLabel->yTextAlign = Alignment::CENTER;
    _dataProviderUrlInput = Create<TextInput>();
    _dataProviderUrlInput->size = { 0, 26 };
    _dataProviderUrlInput->backgroundColor = Color(0x101010);
    _dataProviderUrlInput->border.cornerRadius = 2.0f;
    _dataProviderUrlInput->SetProperty(FlexGrow());
    _dataProviderUrlInput->text = _scene->GetApp()->config.GetConfigValue(osu::DataProviderConfig::URL);
    urlRow->AddItem(std::move(urlLabel));
    urlRow->AddItem(_dataProviderUrlInput.get());
    auto errorLabel = Create<Label>(L"Failed to connect to the data provider. Make sure that both the game and the data provider are running without errors and whether the entered URL is correct");
    errorLabel->parentSize = { 1.0f, 0.0f };
    errorLabel->size = { -20, 0 };
    errorLabel->autoHeight = true;
    errorLabel->xAlign = Alignment::CENTER;
    errorLabel->wordWrapping = WordWrapping::WRAP;
    errorLabel->padding = { 10.0f, 5.0f, 10.0f, 5.0f };
    errorLabel->backgroundColor = Color(0x803030);
    errorLabel->border.cornerRadius = 3;
    errorLabel->SetProperty(Shadow());
    errorLabel->visible.ComputedFrom([](bool showError) { return showError; }, _showDataProviderError);

    setupPanel->AddItem(std::move(statusRow));
    setupPanel->AddItem(std::move(urlRow));
    setupPanel->AddItem(std::move(errorLabel));

    auto descriptionPanelWrapper = Create<FlexPanel>(FlexDirection::RIGHT);
    descriptionPanelWrapper->autoWidth = true;
    descriptionPanelWrapper->autoHeight = true;
    descriptionPanelWrapper->padding = { 10, 10, 10, 10 };
    descriptionPanelWrapper->visible.ComputedFrom([](bool showHelp) { return showHelp; }, _showDataProviderHelp);
    auto descriptionPanel = Create<FlexPanel>(FlexDirection::DOWN);
    descriptionPanel->size = { 250, 0 };
    descriptionPanel->autoHeight = true;
    descriptionPanel->padding = { 10, 5, 10, 15 };
    descriptionPanel->spacing = 5;
    descriptionPanel->backgroundColor = Color(0x282828);
    descriptionPanel->border.cornerRadius = 3;
    descriptionPanel->SetProperty(Shadow());
    auto descriptionTopRow = Create<FlexPanel>(FlexDirection::RIGHT);
    descriptionTopRow->parentSize = { 1.0f, 0.0f };
    descriptionTopRow->autoHeight = true;
    descriptionTopRow->itemAlignment = Alignment::CENTER;
    descriptionTopRow->spacing = 10;
    auto descriptionLabel1 = Create<Label>(L"Some features are marked with the following icon:");
    descriptionLabel1->parentSize = { 1.0f, 0.0f };
    descriptionLabel1->SetProperty(FlexShrink());
    descriptionLabel1->autoHeight = true;
    descriptionLabel1->textSelectable = true;
    descriptionLabel1->wordWrapping = WordWrapping::WRAP;
    auto descriptionIcon = Create<Image>(_scene->GetWindow()->resourceManager.GetImage("osu_memory"));
    descriptionIcon->size = { 26, 26 };
    descriptionTopRow->AddItem(std::move(descriptionLabel1));
    descriptionTopRow->AddItem(std::move(descriptionIcon));
    auto descriptionSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(descriptionSeparator.get());
    auto descriptionLabel2 = Create<Label>(L"These features require access to osu! in-game data to work, which is provided using an external memory reader. Currently only tosu is supported and you can get it from here: https://github.com/tosuapp/tosu/releases");
    descriptionLabel2->parentSize = { 1.0f, 0.0f };
    descriptionLabel2->autoHeight = true;
    descriptionLabel2->textSelectable = true;
    descriptionLabel2->wordWrapping = WordWrapping::WRAP;
    auto descriptionLabel3 = Create<Label>(L"After starting both osu! and the memory reader, you can connect to the specified URL. If you didn't change the memory reader config, you shouldn't need to edit the URL");
    descriptionLabel3->parentSize = { 1.0f, 0.0f };
    descriptionLabel3->autoHeight = true;
    descriptionLabel3->textSelectable = true;
    descriptionLabel3->wordWrapping = WordWrapping::WRAP;
    auto descriptionCloseButton = Create<Button>(L"Got it!");
    NeutralButtonStyle::Apply(descriptionCloseButton.get());
    descriptionCloseButton->size = { 80, 30 };
    descriptionCloseButton->xAlign = Alignment::CENTER;
    descriptionCloseButton->position = { 0, 5 };
    descriptionCloseButton->SubscribeOnActivated([=]() {
        _showDataProviderHelp = false;
    }).Detach();
    descriptionPanel->AddItem(std::move(descriptionTopRow));
    descriptionPanel->AddItem(std::move(descriptionSeparator));
    descriptionPanel->AddItem(std::move(descriptionLabel2));
    descriptionPanel->AddItem(std::move(descriptionLabel3));
    descriptionPanel->AddItem(std::move(descriptionCloseButton));
    descriptionPanelWrapper->AddItem(std::move(descriptionPanel));

    dataSectionPanel->AddItem(std::move(setupPanel));
    dataSectionPanel->AddItem(std::move(descriptionPanelWrapper));

    _contentPanel->AddItem(std::move(dataSectionHeader));
    _contentPanel->AddItem(std::move(dataSectionPanel));
}

void zcom::IntegrationSettingsPanel::_CreateWebSection()
{
    auto webSectionHeader = Create<SectionHeader>(L"Web API", Rect{ 5, 0, 10, 0 });
    webSectionHeader->GetSeparator()->showAnimation.ComputedFrom([](bool waiting) { return waiting; }, _waitingForApiConnection);
    auto webSectionImage = Create<Image>(_scene->GetWindow()->resourceManager.GetImage("osu_web"));
    webSectionImage->size = { 30, 30 };
    webSectionImage->imagePlacement = ImagePlacement::CENTER;
    webSectionImage->snapToPixels = true;
    webSectionHeader->InsertItem(std::move(webSectionImage), 0);

    auto webSectionPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    webSectionPanel->parentSize = { 1.0f, 0.0f };
    webSectionPanel->autoHeight = true;
    webSectionPanel->padding = { 10, 0, 0, 20 };

    auto setupPanel = Create<FlexPanel>(FlexDirection::DOWN);
    setupPanel->autoHeight = true;
    setupPanel->SetProperty(FlexGrow());
    setupPanel->padding.ComputedFrom([](bool showHelp) { return showHelp ? Rect{ 0, 0, 0, 10 } : Rect{}; }, _showWebApiHelp);

    auto statusRow = Create<FlexPanel>(FlexDirection::RIGHT);
    statusRow->parentSize = { 1.0f, 0.0f };
    statusRow->autoHeight = true;
    statusRow->spacing = 10;
    statusRow->padding = { 10, 10, 10, 15 };
    statusRow->itemAlignment = Alignment::CENTER;
    auto saveCredentialsButton = Create<Button>(L"Save");
    NeutralButtonStyle::Apply(saveCredentialsButton.get());
    saveCredentialsButton->size = { 70, 26 };
    //saveCredentialsButton->disabled.ComputedFrom([](bool credentialsChanged) { return !credentialsChanged; }, _credentialsChanged);
    saveCredentialsButton->disabled.ComputedFrom([](bool apiSettingsChanged) { return !apiSettingsChanged; }, _apiSettingsChanged);
    saveCredentialsButton->SubscribeOnActivated([=]() {
        //_SaveCredentials();
        _SaveApiSettings();
    }).Detach();
    auto apiTestButton = Create<Button>(L"Test API");
    NeutralButtonStyle::Apply(apiTestButton.get());
    apiTestButton->size = { 70, 26 };
    apiTestButton->SubscribeOnActivated([=]() {
        _SaveApiSettings();
        _playerDataGetEvent = _scene->GetApp()->Shared<SharedContext*>()->webApi.GetPlayerData("2" /* id of peppy */, "osu");
        _playerDataGetEvent->ResetSynchronousHandler([=](webapi::resp::WebResponse<webapi::resp::PlayerData> resp) {
            ExecuteSynchronously([=, status = resp.status]() {
                _waitingForApiConnection = false;
                _showApiConnectionResult = true;
                _apiTestResponse = status == 200 ? _ApiTestResponse::OK : _ApiTestResponse::FAILED;
            });
        });
        _waitingForApiConnection = true;
        _showApiConnectionResult = false;
    }).Detach();
    auto resetDefaultsButton = Create<Button>(L"Reset defaults");
    NeutralButtonStyle::Apply(resetDefaultsButton.get());
    resetDefaultsButton->size = { 100, 26 };
    resetDefaultsButton->SubscribeOnActivated([=]() {
        _apiUrlInput->text = webapi::WebApiConfig::API_URL.defaultValue;
        _apiSettingsChanged = true;
    }).Detach();
    auto statusRowPadding = Create<Dummy>();
    statusRowPadding->SetProperty(FlexGrow());
    auto helpButton = Create<Button>(L"?");
    NeutralButtonStyle::Apply(helpButton.get());
    helpButton->size = { 22, 22 };
    helpButton->Label()->fontWeight = FontWeight::BOLD;
    helpButton->visible.ComputedFrom([](bool showHelp) { return !showHelp; }, _showWebApiHelp);
    helpButton->SubscribeOnActivated([=]() {
        _showWebApiHelp = !_showWebApiHelp;
    }).Detach();
    statusRow->AddItem(std::move(saveCredentialsButton));
    statusRow->AddItem(std::move(apiTestButton));
    statusRow->AddItem(std::move(resetDefaultsButton));
    statusRow->AddItem(std::move(statusRowPadding));
    statusRow->AddItem(std::move(helpButton));

    auto apiUrlRow = Create<FlexPanel>(FlexDirection::RIGHT);
    apiUrlRow->parentSize = { 1.0f, 0.0f };
    apiUrlRow->autoHeight = true;
    apiUrlRow->spacing = 10;
    apiUrlRow->padding = { 10, 0, 10, 15 };
    auto apiUrlLabel = Create<Label>(L"Api URL:");
    apiUrlLabel->size = { 60, 26 };
    apiUrlLabel->yTextAlign = Alignment::CENTER;
    _apiUrlInput = Create<TextInput>();
    _apiUrlInput->size = { 0, 26 };
    _apiUrlInput->backgroundColor = Color(0x101010);
    _apiUrlInput->border.cornerRadius = 2.0f;
    _apiUrlInput->SetProperty(FlexGrow());
    _apiUrlInput->text = _scene->GetApp()->config.GetConfigValue(webapi::WebApiConfig::API_URL);
    _apiUrlInput->SubscribeOnTextChanged([=](std::wstring*) { _apiSettingsChanged = true; }).Detach();
    apiUrlRow->AddItem(std::move(apiUrlLabel));
    apiUrlRow->AddItem(_apiUrlInput.get());

    auto resultLabel = Create<Label>(L"");
    resultLabel->parentSize = { 1.0f, 0.0f };
    resultLabel->size = { -20, 0 };
    resultLabel->autoHeight = true;
    resultLabel->xAlign = Alignment::CENTER;
    resultLabel->wordWrapping = WordWrapping::WRAP;
    resultLabel->padding = { 10.0f, 5.0f, 10.0f, 5.0f };
    resultLabel->backgroundColor = Color(0x803030);
    resultLabel->border.cornerRadius = 3;
    resultLabel->SetProperty(Shadow());
    resultLabel->visible.ComputedFrom([](bool showResult) { return showResult; }, _showApiConnectionResult);
    resultLabel->SetComputedStyle("style", [](Component* item, _ApiTestResponse testResponse) {
        Label* label = (Label*)item;
        if (testResponse == _ApiTestResponse::OK)
        {
            label->text = L"API connection successful!";
            label->backgroundColor = Color(0x308030);
        }
        else if (testResponse == _ApiTestResponse::FAILED)
        {
            label->text = L"Could not reach the specified URL";
            label->backgroundColor = Color(0x803030);
        }
    }, _apiTestResponse);

    setupPanel->AddItem(std::move(statusRow));
    setupPanel->AddItem(std::move(apiUrlRow));
    setupPanel->AddItem(std::move(resultLabel));

    auto descriptionPanelWrapper = Create<FlexPanel>(FlexDirection::RIGHT);
    descriptionPanelWrapper->autoWidth = true;
    descriptionPanelWrapper->autoHeight = true;
    descriptionPanelWrapper->padding = { 10, 10, 10, 10 };
    descriptionPanelWrapper->visible.ComputedFrom([](bool showHelp) { return showHelp; }, _showWebApiHelp);
    auto descriptionPanel = Create<FlexPanel>(FlexDirection::DOWN);
    descriptionPanel->size = { 250, 0 };
    descriptionPanel->autoHeight = true;
    descriptionPanel->padding = { 10, 5, 10, 15 };
    descriptionPanel->spacing = 5;
    descriptionPanel->backgroundColor = Color(0x282828);
    descriptionPanel->border.cornerRadius = 3;
    descriptionPanel->SetProperty(Shadow());
    auto descriptionTopRow = Create<FlexPanel>(FlexDirection::RIGHT);
    descriptionTopRow->parentSize = { 1.0f, 0.0f };
    descriptionTopRow->autoHeight = true;
    descriptionTopRow->itemAlignment = Alignment::CENTER;
    descriptionTopRow->spacing = 10;
    auto descriptionLabel1 = Create<Label>(L"Some features are marked with the following icon:");
    descriptionLabel1->parentSize = { 1.0f, 0.0f };
    descriptionLabel1->SetProperty(FlexShrink());
    descriptionLabel1->autoHeight = true;
    descriptionLabel1->textSelectable = true;
    descriptionLabel1->wordWrapping = WordWrapping::WRAP;
    auto descriptionIcon = Create<Image>(_scene->GetWindow()->resourceManager.GetImage("osu_web"));
    descriptionIcon->size = { 26, 26 };
    descriptionTopRow->AddItem(std::move(descriptionLabel1));
    descriptionTopRow->AddItem(std::move(descriptionIcon));
    auto descriptionSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(descriptionSeparator.get());
    auto descriptionLabel2 = Create<Label>(L"These features require the Overlay Engine web API to work. If the features aren't working, click the \"Test API\" button to see if the API is working. If not, contact ZenoXLTU on osu!.");
    descriptionLabel2->parentSize = { 1.0f, 0.0f };
    descriptionLabel2->autoHeight = true;
    descriptionLabel2->textSelectable = true;
    descriptionLabel2->wordWrapping = WordWrapping::WRAP;
    auto descriptionLabel3 = Create<Label>(L"If you're running a custom web API server, you can set the URL here.");
    descriptionLabel3->parentSize = { 1.0f, 0.0f };
    descriptionLabel3->autoHeight = true;
    descriptionLabel3->textSelectable = true;
    descriptionLabel3->wordWrapping = WordWrapping::WRAP;
    auto descriptionCloseButton = Create<Button>(L"Got it!");
    NeutralButtonStyle::Apply(descriptionCloseButton.get());
    descriptionCloseButton->size = { 80, 30 };
    descriptionCloseButton->xAlign = Alignment::CENTER;
    descriptionCloseButton->position = { 0, 5 };
    descriptionCloseButton->SubscribeOnActivated([=]() {
        _showWebApiHelp = false;
    }).Detach();
    descriptionPanel->AddItem(std::move(descriptionTopRow));
    descriptionPanel->AddItem(std::move(descriptionSeparator));
    descriptionPanel->AddItem(std::move(descriptionLabel2));
    descriptionPanel->AddItem(std::move(descriptionLabel3));
    descriptionPanel->AddItem(std::move(descriptionCloseButton));
    descriptionPanelWrapper->AddItem(std::move(descriptionPanel));

    webSectionPanel->AddItem(std::move(setupPanel));
    webSectionPanel->AddItem(std::move(descriptionPanelWrapper));

    _contentPanel->AddItem(std::move(webSectionHeader));
    _contentPanel->AddItem(std::move(webSectionPanel));
}

void zcom::IntegrationSettingsPanel::_SaveApiSettings()
{
    if (_apiSettingsChanged)
    {
        _apiSettingsChanged = false;
        _scene->GetApp()->config.SetValue(webapi::WebApiConfig::API_URL.name, _apiUrlInput->text, false);
        _scene->GetApp()->config.SaveConfig();
        _scene->GetApp()->Shared<SharedContext*>()->webApi.SetUrl(_apiUrlInput->text);
    }
}

void zcom::IntegrationSettingsPanel::_SaveShowWebApiHelpValue(bool value)
{
    _scene->GetApp()->config.SetIntValue(webapi::WebApiConfig::SHOW_HELP_PANEL.name, value);
}

void zcom::IntegrationSettingsPanel::_SaveShowDataProviderHelpValue(bool value)
{
    _scene->GetApp()->config.SetIntValue(osu::DataProviderConfig::SHOW_HELP_PANEL.name, value);
}

void zcom::IntegrationSettingsPanel::_OnUpdate()
{
    ScrollPanel::_OnUpdate();

    std::optional<osu::DataProvider::ConnectionEvent> mostRecentEvent = std::nullopt;
    _dataProviderConnectionEvent->HandlePendingEvents([&](osu::DataProvider::ConnectionEvent event) {
        mostRecentEvent = event;
    });

    if (mostRecentEvent)
    {
        if (_waitingForDataProvider)
            _waitingForDataProvider = false;

        auto event = mostRecentEvent.value();
        if (event == osu::DataProvider::CONNECTION_SUCCESSFUL)
            _dataProviderConnected = true;
        else if (event == osu::DataProvider::DISCONNECT_COMPLETED)
            _dataProviderConnected = false;
        else if (event == osu::DataProvider::CONNECTION_FAILED)
        {
            _dataProviderConnected = false;
            _showDataProviderError = true;
        }
    }
}
