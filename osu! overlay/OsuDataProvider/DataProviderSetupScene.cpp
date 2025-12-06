#include "App.h" // App.h must be included first
#include "SharedContext.h"
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

    _dataProviderConnectionEvent = _app->Shared<SharedContext*>()->dataProvider.SubscribeOnConnectionEvent();
    _waitingForDataProvider = true;

    _mainPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    _mainPanel->parentSize = { 1.0f, 1.0f };
    _mainPanel->spacing = 1;
    _mainPanel->size = { 0, -1 };
    _mainPanel->position = { 0, 1 };

    auto setupPanel = Create<FlexPanel>(FlexDirection::DOWN);
    setupPanel->parentSize = { 1.0f, 1.0f };
    setupPanel->SetProperty(FlexShrink());
    setupPanel->backgroundColor = Color(0x1A1A1A);
    setupPanel->spacing = 10;
    setupPanel->padding = { 0, 0, 0, 10 };
    _loadingBar = Create<zcom::LoadingAnimation>();
    _loadingBar->parentSize = { 1.0f, 0.0f };
    _loadingBar->size = { 0, 1 };
    _loadingBar->mainColor = Color(0x5421FF, 0.5f);
    _loadingBar->accentColor = Color(0xF966AB, 1.0f);
    auto statusRow = Create<FlexPanel>(FlexDirection::RIGHT);
    statusRow->parentSize = { 1.0f, 0.0f };
    statusRow->autoHeight = true;
    statusRow->spacing = 10;
    statusRow->padding = { 10, 0, 10, 0 };
    statusRow->itemAlignment = Alignment::CENTER;
    _osuMemoryToggle = Create<Toggle>(false);
    _osuMemoryToggle->size = { 40, 22 };
    _osuMemoryToggle->border.cornerRadius = 11.0f;
    _osuMemoryToggle->marginToBorder = 4.0f;
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
            _app->config.SetValue(osu::DataProviderConfig::URL.name, _urlInput->text);
            if (_app->Shared<SharedContext*>()->dataProvider.Connect(_urlInput->text))
            {
                _waitingForDataProvider = true;
                _osuMemoryToggle->disabled = true;
                _loadingBar->showAnimation = true;
                _errorLabel->visible = false;
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
                _errorLabel->visible = false;
                *newValue = true;
            }
        }
    }).Detach();
    _statusLabel = Create<Label>(L"Disconnected");
    _statusLabel->size = { 0, 18 };
    _statusLabel->autoWidth = true;
    _statusLabel->yTextAlign = Alignment::CENTER;
    _statusLabel->SetProperty(FlexGrow());
    auto helpButton = Create<Button>(L"?");
    NeutralButtonStyle::Apply(helpButton.get());
    helpButton->size = { 22, 22 };
    helpButton->Label()->fontWeight = FontWeight::BOLD;
    helpButton->SubscribeOnActivated([=]() {
        bool newValue = !_descriptionScrollWrapper->visible;
        _descriptionScrollWrapper->visible = newValue;
        _app->config.SetIntValue(osu::DataProviderConfig::SHOW_HELP_PANEL.name, newValue);
    }).Detach();
    statusRow->AddItem(_osuMemoryToggle.get());
    statusRow->AddItem(_statusLabel.get());
    statusRow->AddItem(std::move(helpButton));
    auto separator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(separator.get(), 10);
    auto urlRow = Create<FlexPanel>(FlexDirection::RIGHT);
    urlRow->parentSize = { 1.0f, 0.0f };
    urlRow->autoHeight = true;
    urlRow->spacing = 10;
    urlRow->padding = { 10, 0, 10, 0 };
    auto urlLabel = Create<Label>(L"URL:");
    urlLabel->size = { 0, 26 };
    urlLabel->autoWidth = true;
    urlLabel->yTextAlign = Alignment::CENTER;
    _urlInput = Create<TextInput>();
    _urlInput->size = { 0, 26 };
    _urlInput->backgroundColor = Color(0x101010);
    _urlInput->border.cornerRadius = 2.0f;
    _urlInput->SetProperty(FlexGrow());
    _urlInput->text = _app->config.GetConfigValue(osu::DataProviderConfig::URL);
    urlRow->AddItem(std::move(urlLabel));
    urlRow->AddItem(_urlInput.get());
    auto spacer = Create<Dummy>();
    spacer->size = { 1, 0 };
    spacer->backgroundColor = Color(0, 0.0f);
    spacer->SetProperty(FlexGrow());
    _errorLabel = Create<Label>(L"Failed to connect to the data provider. Make sure that both the game and the data provider are running without errors and whether the entered URL is correct");
    _errorLabel->parentSize = { 1.0f, 0.0f };
    _errorLabel->size = { -20, 0 };
    _errorLabel->autoHeight = true;
    _errorLabel->xAlign = Alignment::CENTER;
    _errorLabel->wordWrapping = WordWrapping::WRAP;
    _errorLabel->xTextAlign = TextAlignment::JUSTIFIED;
    _errorLabel->padding = { 10.0f, 5.0f, 10.0f, 5.0f };
    _errorLabel->backgroundColor = Color(0x803030);
    _errorLabel->border.cornerRadius = 3;
    _errorLabel->SetProperty(Shadow());
    if (!opt.showError)
        _errorLabel->visible = false;

    setupPanel->AddItem(_loadingBar.get());
    setupPanel->AddItem(std::move(statusRow));
    setupPanel->AddItem(std::move(separator));
    setupPanel->AddItem(std::move(urlRow));
    setupPanel->AddItem(std::move(spacer));
    setupPanel->AddItem(_errorLabel.get());

    _descriptionScrollWrapper = Create<ScrollPanel>();
    _descriptionScrollWrapper->parentSize = { 0.0f, 1.0f };
    _descriptionScrollWrapper->size = { 250, 0 };
    _descriptionScrollWrapper->backgroundColor = Color(0x202020);
    _descriptionScrollWrapper->yScrollbar.scrollable = true;
    _descriptionScrollWrapper->yScrollbar.backgroundVisible = true;
    auto descriptionPanel = Create<FlexPanel>(FlexDirection::DOWN);
    descriptionPanel->parentSize = { 1.0f, 0.0f };
    descriptionPanel->autoHeight = true;
    descriptionPanel->padding = { 10, 5, 10, 15 };
    descriptionPanel->spacing = 5;
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
    descriptionLabel1->xTextAlign = TextAlignment::JUSTIFIED;
    auto descriptionIcon = Create<Image>(_window->resourceManager.GetImage("osu_memory"));
    descriptionIcon->size = { 26, 26 };
    descriptionTopRow->AddItem(std::move(descriptionLabel1));
    descriptionTopRow->AddItem(std::move(descriptionIcon));
    auto descriptionSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(descriptionSeparator.get());
    auto descriptionLabel2 = Create<Label>(L"These features require access to osu! in-game data to work, which is provided using an external memory reader. Currently only gosumemory is supported and you can get it from here: https://github.com/l3lackShark/gosumemory");
    descriptionLabel2->parentSize = { 1.0f, 0.0f };
    descriptionLabel2->autoHeight = true;
    descriptionLabel2->textSelectable = true;
    descriptionLabel2->wordWrapping = WordWrapping::WRAP;
    descriptionLabel2->xTextAlign = TextAlignment::JUSTIFIED;
    auto descriptionLabel3 = Create<Label>(L"After starting both osu! and the memory reader, you can connect to the specified URL. If you didn't change the memory reader config, you shouldn't need to edit the URL");
    descriptionLabel3->parentSize = { 1.0f, 0.0f };
    descriptionLabel3->autoHeight = true;
    descriptionLabel3->textSelectable = true;
    descriptionLabel3->wordWrapping = WordWrapping::WRAP;
    descriptionLabel3->xTextAlign = TextAlignment::JUSTIFIED;
    auto descriptionCloseButton = Create<Button>(L"Got it!");
    NeutralButtonStyle::Apply(descriptionCloseButton.get());
    descriptionCloseButton->size = { 80, 30 };
    descriptionCloseButton->xAlign = Alignment::CENTER;
    descriptionCloseButton->position = { 0, 5 };
    descriptionCloseButton->SubscribeOnActivated([=]() {
        _descriptionScrollWrapper->visible = false;
        _app->config.SetIntValue(osu::DataProviderConfig::SHOW_HELP_PANEL.name, false);
    }).Detach();
    descriptionPanel->AddItem(std::move(descriptionTopRow));
    descriptionPanel->AddItem(std::move(descriptionSeparator));
    descriptionPanel->AddItem(std::move(descriptionLabel2));
    descriptionPanel->AddItem(std::move(descriptionLabel3));
    descriptionPanel->AddItem(std::move(descriptionCloseButton));
    _descriptionScrollWrapper->AddItem(std::move(descriptionPanel));
    _descriptionScrollWrapper->visible = _app->config.GetIntConfigValue(osu::DataProviderConfig::SHOW_HELP_PANEL, Config::ADD_IF_MISSING);

    _mainPanel->AddItem(std::move(setupPanel));
    _mainPanel->AddItem(_descriptionScrollWrapper.get());

    _basePanel->AddItem(_mainPanel.get());
    _basePanel->backgroundColor = Color(0);
    _basePanel->SubscribePostUpdate([=]() {
        _Update();
    }).Detach();
}

void zcom::DataProviderSetupScene::Uninit()
{
    _app->config.SetValue(osu::DataProviderConfig::URL.name, _urlInput->text);
}

void zcom::DataProviderSetupScene::_Update()
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
        {
            _osuMemoryToggle->toggledOn = true;
            _statusLabel->text = L"Connected";
        }
        else if (event == osu::DataProvider::DISCONNECT_COMPLETED)
        {
            _osuMemoryToggle->toggledOn = false;
            _statusLabel->text = L"Disconnected";
        }
        else if (event == osu::DataProvider::CONNECTION_FAILED)
        {
            _osuMemoryToggle->toggledOn = false;
            _errorLabel->visible = true;
        }
    }
}