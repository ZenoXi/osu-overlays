#include "App.h"
#include "SharedContext.h"
#include "RTLeaderboardSetupComponent.h"
#include "RTLeaderboardConfig.h"
#include "LeaderboardCountries.h"

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Label.h"
#include "UICore/Components/Base/Button.h"
#include "UICore/Components/Base/Dummy.h"
#include "UICore/Components/Base/Checkbox.h"
#include "UICore/Components/Base/DropdownSelector.h"
#include "UICore/Components/Base/ColorSelector.h"

#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Components/NumberParameterInput.h"
#include "Shared/Styles/Styles.h"

void zcom::RTLeaderboardSetupComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    ScrollPanel::Init();

    size = { 360, 0 };
    parentSize = { 0.0f, 1.0f };
    backgroundColor = Color(0x202020);
    yScrollbar.scrollable = true;
    yScrollbar.backgroundVisible = true;

    _overlay = overlay;
    _overlayView.emplace(overlay->Id(), this);
    _overlayView->enabled_.Subscribe([=](bool enabled) {
        if (!enabled)
            _leaderboardOptionsChanged = false;
    }).Detach();

    auto flexPanel = Create<FlexPanel>(FlexDirection::DOWN);
    flexPanel->parentSize = { 1.0f, 0.0f };
    flexPanel->autoHeight = true;

    auto generalPanel = Create<FlexPanel>(FlexDirection::DOWN);
    generalPanel->parentSize = { 1.0f, 0.0f };
    generalPanel->autoHeight = true;

    auto titleRow = Create<FlexPanel>(FlexDirection::RIGHT);
    titleRow->parentSize = { 1.0f, 0.0f };
    titleRow->autoHeight = true;
    titleRow->padding = { 15, 15, 15, 15 };
    auto generalLabel = Create<Label>(L"Real time leaderboard");
    generalLabel->size = { 0, 30 };
    generalLabel->yTextAlign = Alignment::CENTER;
    generalLabel->fontSize = 20.0f;
    generalLabel->SetProperty(FlexGrow());
    auto enableButton = Create<Button>(L"Enable");
    enableButton->size = { 90, 30 };
    enableButton->SetComputedStyle("style", [](Component* item, bool overlayOpen) {
        Button* button = (Button*)item;
        if (!overlayOpen)
        {
            button->Label()->text = L"Enable";
            EnableButtonStyle::Apply(button);
        }
        else
        {
            button->Label()->text = L"Disable";
            DisableButtonStyle::Apply(button);
        }
    }, _overlayView->enabled_);
    enableButton->activation = ButtonActivation::RELEASE;
    enableButton->SubscribeOnActivated([=]() {
        if (!_overlayView->enabled_)
            _scene->GetApp()->Shared<SharedContext*>()->overlayManager.EnableOverlay(_overlay);
        else
            _scene->GetApp()->Shared<SharedContext*>()->overlayManager.DisableOverlay(_overlay->Id());
    }).Detach();
    titleRow->AddItem(std::move(generalLabel));
    titleRow->AddItem(std::move(enableButton));

    generalPanel->AddItem(std::move(titleRow));
    flexPanel->AddItem(std::move(generalPanel));

    auto overlayLayoutSetupPanel = Create<OverlayLayoutSetup>(_scene->GetApp()->config.GetConfigValue(RTLeaderboardConfig::LAYOUT_STRING));
    overlayLayoutSetupPanel->parentSize = { 1.0f, 0.0f };
    overlayLayoutSetupPanel->padding = { 0, 0, 0, 10 };
    overlayLayoutSetupPanel->autoHeight = true;
    overlayLayoutSetupPanel->SubscribeOnOverlayLayoutStringChanged([=](std::wstring layoutString) {
        _scene->GetApp()->config.SetValue(RTLeaderboardConfig::LAYOUT_STRING.name, layoutString);
    }).Detach();
    flexPanel->AddItem(std::move(overlayLayoutSetupPanel));

    auto leaderboardOptionsPanel = Create<FlexPanel>(FlexDirection::DOWN);
    leaderboardOptionsPanel->parentSize = { 1.0f, 0.0f };
    leaderboardOptionsPanel->autoHeight = true;

    auto settingsChangedWarningLabel = Create<Label>(L"New settings will apply after restarting the real-time leaderboard");
    settingsChangedWarningLabel->parentSize = { 1.0f, 0.0f };
    settingsChangedWarningLabel->autoHeight = true;
    settingsChangedWarningLabel->padding = { 15, 0, 15, 10 };
    settingsChangedWarningLabel->wordWrapping = WordWrapping::WRAP;
    settingsChangedWarningLabel->fontColor = Color(0xA48000);
    settingsChangedWarningLabel->fontStyle = FontStyle::ITALIC;
    settingsChangedWarningLabel->visible.ComputedFrom([](bool settingsChanged) { return settingsChanged; }, _leaderboardOptionsChanged);

    auto countryRow = Create<FlexPanel>(FlexDirection::RIGHT);
    countryRow->parentSize = { 1.0f, 0.0f };
    countryRow->autoHeight = true;
    countryRow->spacing = 10;
    countryRow->padding = { 15, 0, 15, 2 };
    auto useSpecificCountryCheckbox = Create<Checkbox>();
    useSpecificCountryCheckbox->size = { 20, 20 };
    useSpecificCountryCheckbox->backgroundColor = Color(0x101010);
    useSpecificCountryCheckbox->border.cornerRadius = 2.0f;
    useSpecificCountryCheckbox->yAlign = Alignment::CENTER;
    useSpecificCountryCheckbox->checked = _scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::USE_SPECIFIC_COUNTRY, Config::ADD_IF_MISSING);
    useSpecificCountryCheckbox->SubscribeOnStateChanged([=](bool state) {
        _scene->GetApp()->config.SetIntValue(RTLeaderboardConfig::USE_SPECIFIC_COUNTRY.name, state);
        if (_overlayView->enabled_)
            _leaderboardOptionsChanged = true;
    }).Detach();
    auto useSpecificCountryLabel = Create<Label>(L"Use country leaderboard");
    useSpecificCountryLabel->size = { 0, 26 };
    useSpecificCountryLabel->yTextAlign = Alignment::CENTER;
    useSpecificCountryLabel->SetProperty(FlexGrow());
    useSpecificCountryLabel->hoverText = L"When checked, will display the leaderboard of the selected country instead of a global leaderboard. Can be used if the player isn't in the top 10000 to have at least some functionality";
    auto countryDropdown = Create<DropdownSelector>();
    countryDropdown->size = { -30, 26 };
    countryDropdown->parentSize = { 1.0f, 0.0f };
    countryDropdown->SetProperty(FlexMarginAfter(10));
    countryDropdown->xAlign = Alignment::CENTER;
    countryDropdown->backgroundColor = Color(0x101010);
    countryDropdown->GetDropdownPanel()->backgroundColor = Color(0x101010);
    countryDropdown->disabled.ComputedFrom([](bool useSpecificCountry) { return !useSpecificCountry; }, useSpecificCountryCheckbox->checked);
    countryDropdown->hoverText = L"Which country leaderboard to use\nCountry names are listed as they appear in the osu! website";
    std::vector<DropdownItem> countryItems;
    for (int i = 0; i < LeaderboardCountries::COUNTRIES.size(); i++)
        countryItems.push_back({ i, LeaderboardCountries::COUNTRIES[i].name });
    countryDropdown->SetDropdownItems(countryItems);
    std::wstring currentCountryCode = _scene->GetApp()->config.GetConfigValue(RTLeaderboardConfig::COUNTRY_CODE, Config::ADD_IF_MISSING);
    std::optional<size_t> countryIndex = LeaderboardCountries::FindIndexByCode(currentCountryCode);
    if (countryIndex)
    {
        countryDropdown->selectedItemId = (int64_t)countryIndex.value();
        countryDropdown->text = LeaderboardCountries::COUNTRIES[countryIndex.value()].name;
    }
    countryDropdown->SubscribeOnValueSelected([=](std::optional<int64_t> id) {
        if (id && id.value() >= 0 && id.value() < (int64_t)LeaderboardCountries::COUNTRIES.size())
        {
            _scene->GetApp()->config.SetValue(RTLeaderboardConfig::COUNTRY_CODE.name, LeaderboardCountries::COUNTRIES[id.value()].code);
            if (_overlayView->enabled_)
                _leaderboardOptionsChanged = true;
        }
    }).Detach();
    countryRow->AddItem(std::move(useSpecificCountryCheckbox));
    countryRow->AddItem(std::move(useSpecificCountryLabel));

    auto useOtherUserRow = Create<FlexPanel>(FlexDirection::RIGHT);
    useOtherUserRow->parentSize = { 1.0f, 0.0f };
    useOtherUserRow->autoHeight = true;
    useOtherUserRow->spacing = 10;
    useOtherUserRow->padding = { 15, 0, 15, 2 };
    auto useOtherUserCheckbox = Create<Checkbox>();
    useOtherUserCheckbox->size = { 20, 20 };
    useOtherUserCheckbox->backgroundColor = Color(0x101010);
    useOtherUserCheckbox->border.cornerRadius = 2.0f;
    useOtherUserCheckbox->yAlign = Alignment::CENTER;
    useOtherUserCheckbox->checked = _scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::USE_OTHER_USER, Config::ADD_IF_MISSING);
    useOtherUserCheckbox->SubscribeOnStateChanged([=](bool state) {
        _scene->GetApp()->config.SetIntValue(RTLeaderboardConfig::USE_OTHER_USER.name, state);
        if (_overlayView->enabled_)
            _leaderboardOptionsChanged = true;
    }).Detach();
    auto useOtherUserLabel = Create<Label>(L"Use other user");
    useOtherUserLabel->size = { 0, 26 };
    useOtherUserLabel->yTextAlign = Alignment::CENTER;
    useOtherUserLabel->SetProperty(FlexGrow());
    useOtherUserLabel->hoverText = L"When checked, the leaderboard will appear as if playing as a user with the specified id";
    auto userIdRow = Create<FlexPanel>(FlexDirection::RIGHT);
    userIdRow->parentSize = { 1.0f, 0.0f };
    userIdRow->autoHeight = true;
    userIdRow->spacing = 10;
    userIdRow->padding = { 15, 0, 15, 10 };
    auto userIdInput = Create<TextInput>();
    userIdInput->size = { 80, 26 };
    userIdInput->xAlign = Alignment::CENTER;
    userIdInput->backgroundColor = Color(0x101010);
    userIdInput->pattern = L"^[\\d]*$";
    userIdInput->disabled.ComputedFrom([](bool useOtherUser) { return !useOtherUser; }, useOtherUserCheckbox->checked);
    userIdInput->text = _scene->GetApp()->config.GetConfigValue(RTLeaderboardConfig::OTHER_USER_ID, Config::ADD_IF_MISSING);
    userIdInput->SubscribeOnTextChanged([=](std::wstring* text) {
        _scene->GetApp()->config.SetValue(RTLeaderboardConfig::OTHER_USER_ID.name, *text);
        if (_overlayView->enabled_)
            _leaderboardOptionsChanged = true;
    }).Detach();
    auto userIdLabel = Create<Label>(L"User id");
    userIdLabel->size = { 0, 26 };
    userIdLabel->yTextAlign = Alignment::CENTER;
    userIdLabel->SetProperty(FlexGrow());
    userIdLabel->disabled.ComputedFrom([](bool useOtherUser) { return !useOtherUser; }, useOtherUserCheckbox->checked);
    userIdRow->AddItem(std::move(userIdInput));
    userIdRow->AddItem(std::move(userIdLabel));
    useOtherUserRow->AddItem(std::move(useOtherUserCheckbox));
    useOtherUserRow->AddItem(std::move(useOtherUserLabel));

    leaderboardOptionsPanel->AddItem(std::move(Create<SectionHeader>(L"Leaderboard options")));
    leaderboardOptionsPanel->AddItem(std::move(settingsChangedWarningLabel));
    leaderboardOptionsPanel->AddItem(std::move(countryRow));
    leaderboardOptionsPanel->AddItem(std::move(countryDropdown));
    leaderboardOptionsPanel->AddItem(std::move(useOtherUserRow));
    leaderboardOptionsPanel->AddItem(std::move(userIdRow));
    flexPanel->AddItem(std::move(leaderboardOptionsPanel));

    auto appearanceOptionsPanel = Create<FlexPanel>(FlexDirection::DOWN);
    appearanceOptionsPanel->parentSize = { 1.0f, 0.0f };
    appearanceOptionsPanel->autoHeight = true;

    auto uiScaleInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"UI scale", L"The amount by which the visual size of the leaderboard items is multipled. A scale of 2.0 means double the size",
        _scene->GetApp()->config.GetDoubleConfigValue(RTLeaderboardConfig::UI_SCALE, Config::ADD_IF_MISSING),
        0.1f, 100.0f, 0.1f
    ));
    uiScaleInput->GetInput()->size.Assign(Width(70));
    uiScaleInput->GetInput()->precision = 2;
    uiScaleInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(RTLeaderboardConfig::UI_SCALE.name, value.getAsDouble());
    }).Detach();

    auto playerBackgroundColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    playerBackgroundColorRow->parentSize = { 1.0f, 0.0f };
    playerBackgroundColorRow->autoHeight = true;
    playerBackgroundColorRow->spacing = 10;
    playerBackgroundColorRow->padding = { 15, 0, 15, 10 };
    auto playerBackgroundColorInput = Create<ColorSelector>();
    playerBackgroundColorInput->UseConfigValue(RTLeaderboardConfig::PLAYER_BACKGROUND_COLOR);
    playerBackgroundColorInput->size = { 60, 26 };
    playerBackgroundColorInput->border.cornerRadius = 2.0f;
    auto playerBackgroundColorLabel = Create<Label>(L"Player background color");
    playerBackgroundColorLabel->size = { 0, 26 };
    playerBackgroundColorLabel->yTextAlign = Alignment::CENTER;
    playerBackgroundColorLabel->SetProperty(FlexGrow());
    playerBackgroundColorRow->AddItem(std::move(playerBackgroundColorInput));
    playerBackgroundColorRow->AddItem(std::move(playerBackgroundColorLabel));

    auto nonPlayerBackgroundColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    nonPlayerBackgroundColorRow->parentSize = { 1.0f, 0.0f };
    nonPlayerBackgroundColorRow->autoHeight = true;
    nonPlayerBackgroundColorRow->spacing = 10;
    nonPlayerBackgroundColorRow->padding = { 15, 0, 15, 10 };
    auto nonPlayerBackgroundColorInput = Create<ColorSelector>();
    nonPlayerBackgroundColorInput->UseConfigValue(RTLeaderboardConfig::NON_PLAYER_BACKGROUND_COLOR);
    nonPlayerBackgroundColorInput->size = { 60, 26 };
    nonPlayerBackgroundColorInput->border.cornerRadius = 2.0f;
    auto nonPlayerBackgroundColorLabel = Create<Label>(L"Non-player background color");
    nonPlayerBackgroundColorLabel->size = { 0, 26 };
    nonPlayerBackgroundColorLabel->yTextAlign = Alignment::CENTER;
    nonPlayerBackgroundColorLabel->SetProperty(FlexGrow());
    nonPlayerBackgroundColorRow->AddItem(std::move(nonPlayerBackgroundColorInput));
    nonPlayerBackgroundColorRow->AddItem(std::move(nonPlayerBackgroundColorLabel));

    auto usernameBackgroundColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    usernameBackgroundColorRow->parentSize = { 1.0f, 0.0f };
    usernameBackgroundColorRow->autoHeight = true;
    usernameBackgroundColorRow->spacing = 10;
    usernameBackgroundColorRow->padding = { 15, 0, 15, 10 };
    auto usernameBackgroundColorInput = Create<ColorSelector>();
    usernameBackgroundColorInput->UseConfigValue(RTLeaderboardConfig::USERNAME_TEXT_COLOR);
    usernameBackgroundColorInput->size = { 60, 26 };
    usernameBackgroundColorInput->border.cornerRadius = 2.0f;
    auto usernameBackgroundColorLabel = Create<Label>(L"Username text color");
    usernameBackgroundColorLabel->size = { 0, 26 };
    usernameBackgroundColorLabel->yTextAlign = Alignment::CENTER;
    usernameBackgroundColorLabel->SetProperty(FlexGrow());
    usernameBackgroundColorRow->AddItem(std::move(usernameBackgroundColorInput));
    usernameBackgroundColorRow->AddItem(std::move(usernameBackgroundColorLabel));

    auto ppBackgroundColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    ppBackgroundColorRow->parentSize = { 1.0f, 0.0f };
    ppBackgroundColorRow->autoHeight = true;
    ppBackgroundColorRow->spacing = 10;
    ppBackgroundColorRow->padding = { 15, 0, 15, 10 };
    auto ppBackgroundColorInput = Create<ColorSelector>();
    ppBackgroundColorInput->UseConfigValue(RTLeaderboardConfig::PP_TEXT_COLOR);
    ppBackgroundColorInput->size = { 60, 26 };
    ppBackgroundColorInput->border.cornerRadius = 2.0f;
    auto ppBackgroundColorLabel = Create<Label>(L"PP text color");
    ppBackgroundColorLabel->size = { 0, 26 };
    ppBackgroundColorLabel->yTextAlign = Alignment::CENTER;
    ppBackgroundColorLabel->SetProperty(FlexGrow());
    ppBackgroundColorRow->AddItem(std::move(ppBackgroundColorInput));
    ppBackgroundColorRow->AddItem(std::move(ppBackgroundColorLabel));

    auto rankBackgroundColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    rankBackgroundColorRow->parentSize = { 1.0f, 0.0f };
    rankBackgroundColorRow->autoHeight = true;
    rankBackgroundColorRow->spacing = 10;
    rankBackgroundColorRow->padding = { 15, 0, 15, 10 };
    auto rankBackgroundColorInput = Create<ColorSelector>();
    rankBackgroundColorInput->UseConfigValue(RTLeaderboardConfig::RANK_TEXT_COLOR);
    rankBackgroundColorInput->size = { 60, 26 };
    rankBackgroundColorInput->border.cornerRadius = 2.0f;
    auto rankBackgroundColorLabel = Create<Label>(L"Rank text color");
    rankBackgroundColorLabel->size = { 0, 26 };
    rankBackgroundColorLabel->yTextAlign = Alignment::CENTER;
    rankBackgroundColorLabel->SetProperty(FlexGrow());
    rankBackgroundColorRow->AddItem(std::move(rankBackgroundColorInput));
    rankBackgroundColorRow->AddItem(std::move(rankBackgroundColorLabel));

    appearanceOptionsPanel->AddItem(std::move(Create<SectionHeader>(L"Appearance options")));
    appearanceOptionsPanel->AddItem(std::move(uiScaleInput));
    appearanceOptionsPanel->AddItem(std::move(playerBackgroundColorRow));
    appearanceOptionsPanel->AddItem(std::move(nonPlayerBackgroundColorRow));
    appearanceOptionsPanel->AddItem(std::move(usernameBackgroundColorRow));
    appearanceOptionsPanel->AddItem(std::move(ppBackgroundColorRow));
    appearanceOptionsPanel->AddItem(std::move(rankBackgroundColorRow));
    flexPanel->AddItem(std::move(appearanceOptionsPanel));

    AddItem(std::move(flexPanel));
}
