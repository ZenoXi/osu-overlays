#include "App.h"
#include "SharedContext.h"
#include "Window/Window.h"
#include "CursorSensitivitySetup.h"
#include "CursorSensitivityConfig.h"

#include "UICore/Components/Base/NumberInput.h"
#include "UICore/Components/Base/Image.h"
#include "Shared/Components/NumberParameterInput.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Styles/Styles.h"

std::wstring IndexToClientStr(int64_t index)
{
    return index == 1 ? L"lazer" : L"stable";
}

int64_t ClientStrToIndex(const std::wstring& str)
{
    return str == L"lazer" ? 1 : 0;
}

void zcom::CursorSensitivitySetup::Init()
{
    FlexPanel::Init(FlexDirection::DOWN);

    auto infoLabel = Create<Label>(L"! The values here need to match the in-game settings to have accurate results !");
    infoLabel->parentSize = { 1.0f, 0.0f };
    infoLabel->autoHeight = true;
    infoLabel->padding = { 15, 0, 15, 10 };
    infoLabel->wordWrapping = WordWrapping::WRAP;
    infoLabel->fontStyle = FontStyle::ITALIC;

    auto gameClientRow = Create<FlexPanel>(FlexDirection::RIGHT);
    gameClientRow->parentSize = { 1.0f, 0.0f };
    gameClientRow->autoHeight = true;
    gameClientRow->spacing = 10;
    gameClientRow->padding = { 15, 0, 15, 10 };
    gameClientRow->itemAlignment = Alignment::CENTER;
    auto gameClientDropdown = Create<DropdownSelector>();
    gameClientDropdown->size = { 70, 26 };
    gameClientDropdown->backgroundColor = Color(0x101010);
    gameClientDropdown->border.cornerRadius = 2.0f;
    gameClientDropdown->SubscribeOnValueSelected([=](std::optional<int64_t> value) {
        _scene->GetApp()->config.SetValue(CursorSensitivityConfig::GAME_CLIENT.name, (value && value.value() == 1) ? L"lazer" : L"stable");
    }).Detach();
    std::vector<DropdownItem> gameClientDropdownItems;
    gameClientDropdownItems.push_back({ 0l, L"stable" });
    gameClientDropdownItems.push_back({ 1l, L"lazer" });
    gameClientDropdown->SetDropdownItems(gameClientDropdownItems);
    std::wstring client = _scene->GetApp()->config.GetConfigValue(CursorSensitivityConfig::GAME_CLIENT, Config::ADD_AND_SAVE_IF_MISSING);
    int64_t clientIndex = ClientStrToIndex(client);
    gameClientDropdown->selectedItemId = clientIndex;
    gameClientDropdown->text = IndexToClientStr(clientIndex);
    _scene->GetApp()->config.SetValue(CursorSensitivityConfig::GAME_CLIENT.name, IndexToClientStr(clientIndex));
    auto gameClientLabel = Create<Label>(L"Game client");
    gameClientLabel->size = { 0, 20 };
    gameClientLabel->yTextAlign = Alignment::CENTER;
    gameClientLabel->SetProperty(FlexGrow());
    gameClientLabel->hoverText = L"Select which client you're using. There are subtle differences in how stable and lazer handle sensitivity other than 1.0 and selecting the incorrect one can make the overlay drift from actual cursor";
    gameClientRow->AddItem(std::move(gameClientDropdown));
    gameClientRow->AddItem(std::move(gameClientLabel));

    auto rawInputRow = Create<FlexPanel>(FlexDirection::RIGHT);
    rawInputRow->parentSize = { 1.0f, 0.0f };
    rawInputRow->autoHeight = true;
    rawInputRow->spacing = 10;
    rawInputRow->padding = { 15, 0, 15, 10 };
    rawInputRow->itemAlignment = Alignment::CENTER;
    auto rawInputCheckbox = Create<Checkbox>();
    rawInputCheckbox->size = { 20, 20 };
    rawInputCheckbox->backgroundColor = Color(0x101010);
    rawInputCheckbox->border.cornerRadius = 2.0f;
    rawInputCheckbox->yAlign = Alignment::CENTER;
    rawInputCheckbox->checked = _scene->GetApp()->config.GetIntConfigValue(CursorSensitivityConfig::RAW_INPUT_ENABLED, Config::ADD_AND_SAVE_IF_MISSING);
    rawInputCheckbox->SubscribeOnStateChanged([=](bool state) {
        _scene->GetApp()->config.SetIntValue(CursorSensitivityConfig::RAW_INPUT_ENABLED.name, state);
    }).Detach();
    auto rawInputLabel = Create<Label>(L"Raw input enabled");
    rawInputLabel->size = { 0, 20 };
    rawInputLabel->yTextAlign = Alignment::CENTER;
    rawInputLabel->SetProperty(FlexGrow());
    rawInputLabel->hoverText = L"Whether the checkbox 'raw input' in osu!stable or 'high precision mouse' in osu!lazer is checked";
    Checkbox* checkbox = rawInputCheckbox.get();
    rawInputRow->AddItem(std::move(rawInputCheckbox));
    rawInputRow->AddItem(std::move(rawInputLabel));

    auto rawInputWarningLabel = Create<Label>(L"With raw input enabled, the overlay cursor can often de-sync when moving in/out of the osu! window while NOT playing. Alt-Tab out and back in to osu! to fix (might require a few tries).");
    rawInputWarningLabel->parentSize = { 1.0f, 0.0f };
    rawInputWarningLabel->autoHeight = true;
    rawInputWarningLabel->padding = { 15, 0, 15, 10 };
    rawInputWarningLabel->wordWrapping = WordWrapping::WRAP;
    rawInputWarningLabel->fontColor = Color(0xA48000);
    rawInputWarningLabel->fontStyle = FontStyle::ITALIC;
    rawInputWarningLabel->visible.ComputedFrom([=](bool rawInputChecked) { return rawInputChecked; }, checkbox->checked);

    double sensitivity = _scene->GetApp()->config.GetDoubleConfigValue(CursorSensitivityConfig::SENSITIVITY, Config::ADD_AND_SAVE_IF_MISSING);
    auto sensitivityInput = Create<NumberParameterInput<double>>(NumberParameterInputParams(L"Sensitivity", L"The sensitivity option value in game", sensitivity, 0.01, 10.0, 0.01));
    sensitivityInput->GetInput()->stepSize = NumberInputValue("0.01");
    sensitivityInput->GetInput()->precision = 2;
    sensitivityInput->GetInput()->size = { 70, 26 };
    sensitivityInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(CursorSensitivityConfig::SENSITIVITY.name, value.getAsDouble());
    }).Detach();
    sensitivityInput->GetInput()->disabled.ComputedFrom([=](bool rawInputChecked) { return !rawInputChecked; }, checkbox->checked);
    sensitivityInput->GetLabel()->disabled.ComputedFrom([=](bool rawInputChecked) { return !rawInputChecked; }, checkbox->checked);

    AddItem(Create<SectionHeader>(L"[BETA] Cursor sensitivity"));
    AddItem(std::move(infoLabel));
    AddItem(std::move(gameClientRow));
    AddItem(std::move(rawInputWarningLabel));
    AddItem(std::move(rawInputRow));
    AddItem(std::move(sensitivityInput));
}