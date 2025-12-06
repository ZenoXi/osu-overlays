#include "App.h" // If not included first, results in winsock redefinition errors
#include "UpdateSettingsPanel.h"
#include "SharedContext.h"
#include "Window/Window.h"

#include "OsuWebApi/WebApiConfig.h"

#include "Components/Base/Image.h"
#include "Components/Base/Toggle.h"
#include "Components/Base/Button.h"
#include "Components/Base/Dummy.h"
#include "Shared/Components/LoadingAnimation.h"
#include "Shared/Util/Streams.h"

void zcom::UpdateSettingsPanel::Init(std::optional<std::any> extraOptions)
{
    ScrollPanel::Init();

    yScrollbar.scrollable = true;
    yScrollbar.backgroundVisible = true;

    _contentPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _contentPanel->parentSize = { 1.0f, 0.0f };
    _contentPanel->autoHeight = true;

    auto checkForUpdatesLoadingBar = Create<LoadingAnimation>();
    checkForUpdatesLoadingBar->parentSize = { 1.0f, 0.0f };
    checkForUpdatesLoadingBar->size = { 0, 1 };
    checkForUpdatesLoadingBar->mainColor = Color(0x5421FF, 0.5f);
    checkForUpdatesLoadingBar->accentColor = Color(0xF966AB, 1.0f);
    checkForUpdatesLoadingBar->showAnimation.ComputedFrom([](bool waitingForUpdateCheck, bool waitingForUpdateInitiation) {
        return waitingForUpdateCheck || waitingForUpdateInitiation;
    }, _waitingForUpdateCheck, _waitingForUpdateInitiation);

    _errorLabel = Create<Label>(L"");
    _errorLabel->parentSize = { 1.0f, 0.0f };
    _errorLabel->size = { -20, 0 };
    _errorLabel->autoHeight = true;
    _errorLabel->xAlign = Alignment::CENTER;
    _errorLabel->SetProperty(FlexMarginBefore(10));
    _errorLabel->padding = { 8.0f, 5.0f, 8.0f, 5.0f };
    _errorLabel->wordWrapping = WordWrapping::EMERGENCY_BREAK;
    _errorLabel->backgroundColor = Color(0x803030);
    _errorLabel->border.cornerRadius = 5.0f;
    _errorLabel->visible = false;

    _contentPanel->AddItem(std::move(checkForUpdatesLoadingBar));
    _contentPanel->AddItem(_errorLabel.get());

    auto headerPanel = Create<FlexPanel>(FlexDirection::DOWN);
    headerPanel->parentSize = { 1.0f, 0.0f };
    headerPanel->autoHeight = true;
    headerPanel->padding = { 10, 10, 10, 10 };

    auto versionTitleLabel = Create<Label>(L"Currently running version:");
    versionTitleLabel->autoWidth = true;
    versionTitleLabel->autoHeight = true;
    versionTitleLabel->fontSize = 16.0f;
    versionTitleLabel->SetProperty(FlexMarginAfter(5));

    _versionLabel = Create<Label>(string_to_wstring(OVERLAY_ENGINE_VERSION.ToString()));
    _versionLabel->autoWidth = true;
    _versionLabel->autoHeight = true;
    _versionLabel->fontSize = 20.0f;
    _versionLabel->fontWeight = FontWeight::BOLD;
    _versionLabel->SetProperty(FlexMarginAfter(10));

    auto checkForUpdatesButton = Create<Button>(L"Check for updates");
    NeutralButtonStyle::Apply(checkForUpdatesButton.get());
    checkForUpdatesButton->size = { 120, 30 };
    checkForUpdatesButton->SubscribeOnActivated([=]() {
        _updatesPanel->ClearItems();
        _versionLabel->text = string_to_wstring(OVERLAY_ENGINE_VERSION.ToString());

        _checkForUpdatesSubscription = _scene->GetApp()->Shared<SharedContext*>()->versionManager.CheckForUpdates();
        _waitingForUpdateCheck = true;
        _errorLabel->visible = false;
    }).Detach();

    headerPanel->AddItem(std::move(versionTitleLabel));
    headerPanel->AddItem(_versionLabel.get());
    headerPanel->AddItem(std::move(checkForUpdatesButton));
    _contentPanel->AddItem(std::move(headerPanel));

    _updatesPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _updatesPanel->parentSize = { 1.0f, 0.0f };
    _updatesPanel->autoHeight = true;
    _updatesPanel->padding = { 10, 10, 10, 10 };

    _contentPanel->AddItem(_updatesPanel.get());

    _BuildPageFromAvailableUpdateData(_scene->GetApp()->Shared<SharedContext*>()->versionManager.GetCachedAvailableUpdates());

    AddItem(_contentPanel.get());
}

void zcom::UpdateSettingsPanel::_OnUpdate()
{
    ScrollPanel::_OnUpdate();

    if (_waitingForUpdateCheck)
    {
        _checkForUpdatesSubscription->HandlePendingEvents([=](std::vector<UpdateData> updates) {
            _waitingForUpdateCheck = false;
            _BuildPageFromAvailableUpdateData(updates);
        });
    }

    if (_waitingForUpdateInitiation)
    {
        _updateInititatedSubscription->HandlePendingEvents([=](std::optional<std::wstring> error) {
            _waitingForUpdateInitiation = false;
            if (!error.has_value())
            {
                _scene->GetApp()->Exit();
            }
            else
            {
                _errorLabel->visible = true;
                _errorLabel->text = error.value();
            }
        });
    }
}

void zcom::UpdateSettingsPanel::_BuildPageFromAvailableUpdateData(std::vector<UpdateData> availableUpdates)
{
    _updatesPanel->ClearItems();

    if (!availableUpdates.empty())
    {
        auto updatesAvailableLabel = Create<Label>(L"Available updates");
        updatesAvailableLabel->autoWidth = true;
        updatesAvailableLabel->autoHeight = true;
        updatesAvailableLabel->fontSize = 16.0f;
        _updatesPanel->AddItem(std::move(updatesAvailableLabel));

        _versionLabel->text = string_to_wstring(OVERLAY_ENGINE_VERSION.ToString());
        _updatesPanel->visible = true;
    }
    else
    {
        _versionLabel->text = string_to_wstring(OVERLAY_ENGINE_VERSION.ToString()) + L" (up to date)";
        _updatesPanel->visible = false;
        return;
    }

    if (!availableUpdates.empty() && availableUpdates.front().minUpdatableVersionTag > OVERLAY_ENGINE_VERSION)
    {
        auto panel = Create<FlexPanel>(FlexDirection::DOWN);
        panel->parentSize = { 1.0f, 0.0f };
        panel->autoHeight = true;
        panel->spacing = 3;
        panel->backgroundColor = Color(0x242424);
        panel->border.cornerRadius = 5.0f;
        panel->SetProperty(FlexMarginBefore(10));
        panel->SetProperty(Shadow());
        
        auto mainRow = Create<FlexPanel>(FlexDirection::RIGHT);
        mainRow->parentSize = { 1.0f, 0.0f };
        mainRow->autoHeight = true;
        mainRow->padding = { 10, 5, 10, 5 };
        mainRow->spacing = 10;
        mainRow->itemAlignment = Alignment::CENTER;

        auto versionAndDateColumn = Create<FlexPanel>(FlexDirection::DOWN);
        versionAndDateColumn->autoHeight = true;
        versionAndDateColumn->SetProperty(FlexGrow());
        auto versionLabel = Create<Label>(string_to_wstring(availableUpdates.front().versionTag.ToString() + " (Latest)"));
        versionLabel->autoWidth = true;
        versionLabel->autoHeight = true;
        versionLabel->fontSize = 18.0f;
        versionLabel->fontWeight = FontWeight::BOLD;
        versionLabel->fontColor = Color(0x707070);
        auto publishedAtLabel = Create<Label>(string_to_wstring(availableUpdates.front().publishedAt.substr(0, 10)));
        publishedAtLabel->autoWidth = true;
        publishedAtLabel->autoHeight = true;
        publishedAtLabel->fontColor = Color(0x606060);
        versionAndDateColumn->AddItem(std::move(versionLabel));
        versionAndDateColumn->AddItem(std::move(publishedAtLabel));

        auto webLinkLabel = Create<Label>(L"View on GitHub");
        webLinkLabel->autoWidth = true;
        webLinkLabel->autoHeight = true;
        webLinkLabel->fontColor = Color(0x639CFF);
        webLinkLabel->fontStyle = FontStyle::ITALIC;
        webLinkLabel->cursorIcon = zwnd::CursorIcon::HAND;
        webLinkLabel->SubscribeOnLeftReleased([=](Component* label, std::optional<Point>) {
            if (label->hoveredArea_)
            {
                ShellExecuteA(NULL, NULL, availableUpdates.front().htmlUrl.c_str(), NULL, NULL, SW_SHOWDEFAULT);
            }
        }).Detach();

        mainRow->AddItem(std::move(versionAndDateColumn));
        mainRow->AddItem(std::move(webLinkLabel));

        panel->AddItem(std::move(mainRow));

        auto descriptionLabel = Create<Label>(L"Updating to this version automatically is only possible from newer versions. Please update to the newest available version first or download the newest version manually by clicking the \"View on GitHub\" link");
        descriptionLabel->parentSize = { 1.0f, 0.0f };
        descriptionLabel->autoHeight = true;
        descriptionLabel->padding = { 10, 10, 10, 5 };
        descriptionLabel->wordWrapping = WordWrapping::WRAP;
        descriptionLabel->fontColor = Color(0xAF9055);
        descriptionLabel->fontStyle = FontStyle::ITALIC;
        descriptionLabel->backgroundColor = Color(0x1A1A1A);
        descriptionLabel->border.cornerRadius = 5.0f;
        descriptionLabel->zIndex = -10;
        descriptionLabel->SetProperty(FlexMarginBefore(-5));

        _updatesPanel->AddItem(std::move(panel));
        _updatesPanel->AddItem(std::move(descriptionLabel));

        size_t unavailableVersionCount = streams::From(availableUpdates)
            .Filter([](const UpdateData& data) { return data.minUpdatableVersionTag > OVERLAY_ENGINE_VERSION; })
            .Count();

        if (unavailableVersionCount > 1)
        {
            auto panel = Create<FlexPanel>(FlexDirection::DOWN);
            panel->parentSize = { 1.0f, 0.0f };
            panel->autoHeight = true;
            panel->spacing = 3;
            panel->backgroundColor = Color(0x242424);
            panel->border.cornerRadius = 5.0f;
            panel->SetProperty(FlexMarginBefore(10));
            panel->SetProperty(Shadow());

            auto label = Create<Label>(std::to_wstring(unavailableVersionCount - 1) + L" more intermediate version" + (unavailableVersionCount > 2 ? L"s" : L""));
            label->parentSize = { 1.0f, 0.0f };
            label->autoHeight = true;
            label->padding = { 10, 5, 10, 5 };
            label->fontColor = Color(0x707070);
            label->fontStyle = FontStyle::ITALIC;
            label->wordWrapping = WordWrapping::WRAP;

            panel->AddItem(std::move(label));

            _updatesPanel->AddItem(std::move(panel));
        }
    }

    for (int i = 0; i < availableUpdates.size(); i++)
    {
        auto& update = availableUpdates[i];
        if (update.minUpdatableVersionTag > OVERLAY_ENGINE_VERSION)
            continue;

        auto panel = Create<FlexPanel>(FlexDirection::DOWN);
        panel->parentSize = { 1.0f, 0.0f };
        panel->autoHeight = true;
        panel->spacing = 3;
        panel->backgroundColor = Color(0x303030);
        panel->border.cornerRadius = 5.0f;
        panel->SetProperty(FlexMarginBefore(10));
        panel->SetProperty(Shadow());

        auto mainRow = Create<FlexPanel>(FlexDirection::RIGHT);
        mainRow->parentSize = { 1.0f, 0.0f };
        mainRow->autoHeight = true;
        mainRow->padding = { 10, 5, 10, 5 };
        mainRow->spacing = 10;
        mainRow->itemAlignment = Alignment::CENTER;

        auto versionAndDateColumn = Create<FlexPanel>(FlexDirection::DOWN);
        versionAndDateColumn->autoHeight = true;
        versionAndDateColumn->SetProperty(FlexGrow());
        auto versionLabel = Create<Label>(string_to_wstring(update.versionTag.ToString() + (i == 0 ? " (Latest)" : "")));
        versionLabel->autoWidth = true;
        versionLabel->autoHeight = true;
        versionLabel->fontSize = 18.0f;
        versionLabel->fontWeight = FontWeight::BOLD;
        auto publishedAtLabel = Create<Label>(string_to_wstring(update.publishedAt.substr(0, 10)));
        publishedAtLabel->autoWidth = true;
        publishedAtLabel->autoHeight = true;
        versionAndDateColumn->AddItem(std::move(versionLabel));
        versionAndDateColumn->AddItem(std::move(publishedAtLabel));

        auto webLinkLabel = Create<Label>(L"View on GitHub");
        webLinkLabel->autoWidth = true;
        webLinkLabel->autoHeight = true;
        webLinkLabel->fontColor = Color(0x639CFF);
        webLinkLabel->fontStyle = FontStyle::ITALIC;
        webLinkLabel->cursorIcon = zwnd::CursorIcon::HAND;
        webLinkLabel->SubscribeOnLeftReleased([=](Component* label, std::optional<Point>) {
            if (label->hoveredArea_)
            {
                ShellExecuteA(NULL, NULL, update.htmlUrl.c_str(), NULL, NULL, SW_SHOWDEFAULT);
            }
        }).Detach();

        auto updateButton = Create<Button>(L"Update");
        NeutralButtonStyle::Apply(updateButton.get());
        updateButton->backgroundColor = Color(0x404040);
        updateButton->size = { 80, 30 };
        updateButton->SubscribeOnActivated([=]() {
            _updateInititatedSubscription = _scene->GetApp()->Shared<SharedContext*>()->versionManager.InitiateUpdateToVersion(update);
            _waitingForUpdateInitiation = true;
            _errorLabel->visible = false;
        }).Detach();

        mainRow->AddItem(std::move(versionAndDateColumn));
        mainRow->AddItem(std::move(webLinkLabel));
        mainRow->AddItem(std::move(updateButton));

        panel->AddItem(std::move(mainRow));

        std::string finalDescription = replace_all(update.description, "\\r\\n", "\r\n");
        auto descriptionLabel = Create<Label>(string_to_wstring(finalDescription));
        descriptionLabel->parentSize = { 1.0f, 0.0f };
        descriptionLabel->autoHeight = true;
        descriptionLabel->padding = { 10, 10, 10, 5 };
        descriptionLabel->wordWrapping = WordWrapping::WRAP;
        descriptionLabel->backgroundColor = Color(0x1A1A1A);
        descriptionLabel->border.cornerRadius = 5.0f;
        descriptionLabel->zIndex = -10;
        descriptionLabel->SetProperty(FlexMarginBefore(-5));

        _updatesPanel->AddItem(std::move(panel));
        _updatesPanel->AddItem(std::move(descriptionLabel));
    }
}
