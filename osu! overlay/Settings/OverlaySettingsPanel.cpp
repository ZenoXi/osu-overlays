#include "App.h"
#include "SharedContext.h"
#include "OverlaySettingsPanel.h"
#include "Overlays/OverlayConfig.h"

#include "UICore/Components/Base/Checkbox.h"
#include "UICore/Components/Base/DropdownSelector.h"
#include "Shared/Components/NumberParameterInput.h"
#include "Shared/Components/SectionHeader.h"

void zcom::OverlaySettingsPanel::Init(std::optional<std::any> extraOptions)
{
    ScrollPanel::Init();

    yScrollbar.scrollable = true;
    yScrollbar.backgroundVisible = true;

    Reinit(extraOptions);

    _contentPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _contentPanel->parentSize = { 1.0f, 0.0f };
    _contentPanel->autoHeight = true;

    std::vector<std::wstring> monitors;
    EnumDisplayMonitors(NULL, NULL, _EnumMonitorsProc, (LPARAM)&monitors);

    auto monitorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    monitorRow->parentSize = { 1.0f, 0.0f };
    monitorRow->autoHeight = true;
    monitorRow->spacing = 10;
    monitorRow->padding = { 15, 0, 15, 10 };
    auto monitorDropdown = Create<DropdownSelector>();
    monitorDropdown->size = { 180, 26 };
    monitorDropdown->backgroundColor = Color(0x101010);
    monitorDropdown->border.cornerRadius = 2.0f;
    monitorDropdown->SubscribeOnValueSelected([=](std::optional<int64_t> value) {
        _scene->GetApp()->config.SetIntValue(OverlayConfig::MONITOR_INDEX.name, value.value());
    }).Detach();
    std::vector<DropdownItem> monitorDropdownItems;
    monitorDropdownItems.push_back({ 0l, L"System primary" });
    for (int i = 0; i < monitors.size(); i++)
        monitorDropdownItems.push_back({ i + 1, std::to_wstring(i + 1) + L"  " + monitors[i] });
    monitorDropdown->SetDropdownItems(monitorDropdownItems);
    int64_t monitorIndex = _scene->GetApp()->config.GetIntConfigValue(OverlayConfig::MONITOR_INDEX, Config::ADD_AND_SAVE_IF_MISSING);
    if (monitorIndex <= 0 || monitorIndex > (int64_t)monitors.size())
    {
        monitorDropdown->selectedItemId = 0;
        monitorDropdown->text = L"System primary";
        _scene->GetApp()->config.SetIntValue(OverlayConfig::MONITOR_INDEX.name, 0);
    }
    else
    {
        monitorDropdown->selectedItemId = monitorIndex;
        monitorDropdown->text = std::to_wstring(monitorIndex) + L"  " + monitors[monitorIndex - 1];
    }
    auto monitorLabel = Create<Label>(L"Monitor");
    monitorLabel->size = { 0, 26 };
    monitorLabel->yTextAlign = Alignment::CENTER;
    monitorLabel->SetProperty(FlexGrow());
    monitorLabel->hoverText = L"Which monitor to use";
    monitorRow->AddItem(std::move(monitorDropdown));
    monitorRow->AddItem(std::move(monitorLabel));

    auto layoutPanel = Create<FlexPanel>(FlexDirection::DOWN);
    layoutPanel->parentSize = { 1.0f, 0.0f };
    layoutPanel->autoHeight = true;

    auto fillMonitorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    fillMonitorRow->parentSize = { 1.0f, 0.0f };
    fillMonitorRow->autoHeight = true;
    fillMonitorRow->spacing = 10;
    fillMonitorRow->padding = { 15, 0, 15, 10 };
    auto fillMonitorCheckbox = Create<Checkbox>();
    fillMonitorCheckbox->size = { 20, 20 };
    fillMonitorCheckbox->backgroundColor = Color(0x101010);
    fillMonitorCheckbox->border.cornerRadius = 2.0f;
    fillMonitorCheckbox->yAlign = Alignment::CENTER;
    fillMonitorCheckbox->checked = _scene->GetApp()->config.GetIntConfigValue(OverlayConfig::FILL_MONITOR, Config::ADD_AND_SAVE_IF_MISSING);
    fillMonitorCheckbox->SubscribeOnStateChanged([=](bool state) {
        _scene->GetApp()->config.SetIntValue(OverlayConfig::FILL_MONITOR.name, state);
    }).Detach();
    auto fillMonitorLabel = Create<Label>(L"Fill monitor");
    fillMonitorLabel->size = { 0, 20 };
    fillMonitorLabel->yTextAlign = Alignment::CENTER;
    fillMonitorLabel->SetProperty(FlexGrow());
    fillMonitorLabel->hoverText = L"If checked, the overlay display area will fill the entire selected monitor area";
    fillMonitorRow->AddItem(std::move(fillMonitorCheckbox));
    fillMonitorRow->AddItem(std::move(fillMonitorLabel));

    auto basicLayoutSection = Create<FlexPanel>(FlexDirection::RIGHT);
    basicLayoutSection->size = { 300, 0 };
    basicLayoutSection->autoHeight = true;
    basicLayoutSection->spacing = 10;
    basicLayoutSection->padding = { 15, 0, 15, 10 };
    {
        auto sizeCol = Create<FlexPanel>(FlexDirection::DOWN);
        sizeCol->autoHeight = true;
        sizeCol->SetProperty(FlexGrow());
        sizeCol->spacing = 10;
        {
            auto widthInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(L"Width", L"Width of the display area, in pixels",
                _scene->GetApp()->config.GetIntConfigValue(OverlayConfig::WIDTH, Config::ADD_AND_SAVE_IF_MISSING),
                1, 10000, 10
            ));
            widthInput->padding = { 0, 0, 0, 0 };
            widthInput->GetInput()->size = { 70, 26 };
            widthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(OverlayConfig::WIDTH.name, value.getAsInteger());
            }).Detach();
            sizeCol->AddItem(std::move(widthInput));
        } {
            auto heightInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(L"Height", L"Height of the display area, in pixels",
                _scene->GetApp()->config.GetIntConfigValue(OverlayConfig::HEIGHT, Config::ADD_AND_SAVE_IF_MISSING),
                1, 10000, 10
            ));
            heightInput->padding = { 0, 0, 0, 0 };
            heightInput->GetInput()->size = { 70, 26 };
            heightInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(OverlayConfig::HEIGHT.name, value.getAsInteger());
            }).Detach();
            sizeCol->AddItem(std::move(heightInput));
        }
        basicLayoutSection->AddItem(std::move(sizeCol));
    } {
        auto offsetCol = Create<FlexPanel>(FlexDirection::DOWN);
        offsetCol->autoHeight = true;
        offsetCol->SetProperty(FlexGrow());
        offsetCol->spacing = 10;
        {
            auto xOffsetInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(L"X offset", L"Horizontal offset of the display area, in pixels",
                _scene->GetApp()->config.GetIntConfigValue(OverlayConfig::X_OFFSET, Config::ADD_AND_SAVE_IF_MISSING),
                -10000, 10000, 1
            ));
            xOffsetInput->padding = { 0, 0, 0, 0 };
            xOffsetInput->GetInput()->size = { 70, 26 };
            xOffsetInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(OverlayConfig::X_OFFSET.name, value.getAsInteger());
            }).Detach();
            offsetCol->AddItem(std::move(xOffsetInput));
        } {
            auto yOffsetInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(L"Y offset", L"Vertical offset of the display area, in pixels",
                _scene->GetApp()->config.GetIntConfigValue(OverlayConfig::Y_OFFSET, Config::ADD_AND_SAVE_IF_MISSING),
                -10000, 10000, 1
            ));
            yOffsetInput->padding = { 0, 0, 0, 0 };
            yOffsetInput->GetInput()->size = { 70, 26 };
            yOffsetInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(OverlayConfig::Y_OFFSET.name, value.getAsInteger());
            }).Detach();
            offsetCol->AddItem(std::move(yOffsetInput));
        }
        basicLayoutSection->AddItem(std::move(offsetCol));
    }

    auto fitGameWindowRow = Create<FlexPanel>(FlexDirection::RIGHT);
    fitGameWindowRow->parentSize = { 1.0f, 0.0f };
    fitGameWindowRow->autoHeight = true;
    fitGameWindowRow->spacing = 10;
    fitGameWindowRow->padding = { 15, 0, 15, 10 };
    auto fitGameWindowCheckbox = Create<Checkbox>();
    fitGameWindowCheckbox->size = { 20, 20 };
    fitGameWindowCheckbox->backgroundColor = Color(0x101010);
    fitGameWindowCheckbox->border.cornerRadius = 2.0f;
    fitGameWindowCheckbox->yAlign = Alignment::CENTER;
    fitGameWindowCheckbox->checked = _scene->GetApp()->config.GetIntConfigValue(OverlayConfig::FIT_TO_GAME_WINDOW, Config::ADD_AND_SAVE_IF_MISSING);
    fitGameWindowCheckbox->SubscribeOnStateChanged([=](bool state) {
        _scene->GetApp()->config.SetIntValue(OverlayConfig::FIT_TO_GAME_WINDOW.name, state);
    }).Detach();
    auto fitGameWindowLabel = Create<Label>(L"Fit to osu! window");
    fitGameWindowLabel->size = { 0, 20 };
    fitGameWindowLabel->yTextAlign = Alignment::CENTER;
    fitGameWindowLabel->SetProperty(FlexGrow());
    fitGameWindowLabel->hoverText = L"If checked, the display area will automatically fit to the osu! window if it is open (overrides all other layout settings)";
    fitGameWindowRow->AddItem(std::move(fitGameWindowCheckbox));
    fitGameWindowRow->AddItem(std::move(fitGameWindowLabel));

    layoutPanel->AddItem(Create<SectionHeader>(L"Display area layout"));
    layoutPanel->AddItem(std::move(monitorRow));
    layoutPanel->AddItem(std::move(fillMonitorRow));
    layoutPanel->AddItem(std::move(basicLayoutSection));
    layoutPanel->AddItem(std::move(fitGameWindowRow));
    _contentPanel->AddItem(std::move(layoutPanel));

    AddItem(_contentPanel.get());
}

void zcom::OverlaySettingsPanel::Reinit(std::optional<std::any> extraOptions)
{

}

BOOL zcom::OverlaySettingsPanel::_EnumMonitorsProc(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM lparam)
{
    MONITORINFOEX info{};
    info.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(monitor, &info);
    auto monitors = (std::vector<std::wstring>*)lparam;
    monitors->push_back(std::wstring(info.szDevice));
    return TRUE;
}
