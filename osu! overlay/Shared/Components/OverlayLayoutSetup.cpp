#include "App.h"
#include "SharedContext.h"
#include "OverlayLayoutSetup.h"

#include "UICore/Components/Base/NumberInput.h"
#include "UICore/Components/Base/Dummy.h"
#include "Shared/Components/NumberParameterInput.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Styles/Styles.h"

void zcom::OverlayLayoutSetup::Init(const std::wstring& layoutString)
{
    FlexPanel::Init(FlexDirection::DOWN);

    _ParseLayoutString(layoutString);

    auto fillOverlayRow = Create<FlexPanel>(FlexDirection::RIGHT);
    fillOverlayRow->parentSize = { 1.0f, 0.0f };
    fillOverlayRow->autoHeight = true;
    fillOverlayRow->spacing = 10;
    fillOverlayRow->padding = { 15, 0, 15, 10 };
    auto fillOverlayCheckbox = Create<Checkbox>();
    fillOverlayCheckbox->size = { 20, 20 };
    fillOverlayCheckbox->backgroundColor = Color(0x101010);
    fillOverlayCheckbox->border.cornerRadius = 2.0f;
    fillOverlayCheckbox->yAlign = Alignment::CENTER;
    fillOverlayCheckbox->checked = _fillDisplayArea;
    fillOverlayCheckbox->SubscribeOnStateChanged([=](bool state) {
        _fillDisplayArea = state;
        _UpdateLayoutString();
    }).Detach();
    auto fillOverlayLabel = Create<Label>(L"Fill entire overlay");
    fillOverlayLabel->size = { 0, 20 };
    fillOverlayLabel->yTextAlign = Alignment::CENTER;
    fillOverlayLabel->SetProperty(FlexGrow());
    fillOverlayLabel->hoverText = L"When checked, this overlay will use the entire overlay work area";
    fillOverlayRow->AddItem(std::move(fillOverlayCheckbox));
    fillOverlayRow->AddItem(std::move(fillOverlayLabel));

    auto basicLayoutSection = Create<FlexPanel>(FlexDirection::RIGHT);
    basicLayoutSection->parentSize = { 1.0f, 0.0f };
    basicLayoutSection->autoHeight = true;
    basicLayoutSection->spacing = 10;
    basicLayoutSection->padding = { 15, 0, 15, 10 };
    {
        auto sizeCol = Create<FlexPanel>(FlexDirection::DOWN);
        sizeCol->autoHeight = true;
        sizeCol->SetProperty(FlexGrow());
        sizeCol->spacing = 10;
        {
            auto widthInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(L"Width", L"Width of the overlay, in pixels", _width, 1, 10000, 10));
            widthInput->padding = { 0, 0, 0, 0 };
            widthInput->GetInput()->size = { 70, 26 };
            widthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _width = (int)value.getAsInteger();
                _UpdateLayoutString();
            }).Detach();
            sizeCol->AddItem(std::move(widthInput));
        } {
            auto heightInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(L"Height", L"Height of the overlay, in pixels", _height, 1, 10000, 10));
            heightInput->padding = { 0, 0, 0, 0 };
            heightInput->GetInput()->size = { 70, 26 };
            heightInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _height = (int)value.getAsInteger();
                _UpdateLayoutString();
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
            auto xOffsetInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(L"X offset", L"Horizontal offset of the overlay, in pixels", _xOffset, -10000, 10000, 1));
            xOffsetInput->padding = { 0, 0, 0, 0 };
            xOffsetInput->GetInput()->size = { 70, 26 };
            xOffsetInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _xOffset = (int)value.getAsInteger();
                _UpdateLayoutString();
            }).Detach();
            offsetCol->AddItem(std::move(xOffsetInput));
        } {
            auto yOffsetInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(L"Y offset", L"Vertical offset of the overlay, in pixels", _yOffset, -10000, 10000, 1));
            yOffsetInput->padding = { 0, 0, 0, 0 };
            yOffsetInput->GetInput()->size = { 70, 26 };
            yOffsetInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _yOffset = (int)value.getAsInteger();
                _UpdateLayoutString();
            }).Detach();
            offsetCol->AddItem(std::move(yOffsetInput));
        }
        basicLayoutSection->AddItem(std::move(offsetCol));
    }

    auto advancedLayoutSection = Create<FlexPanel>(FlexDirection::DOWN);
    advancedLayoutSection->parentSize = { 1.0f, 0.0f };
    advancedLayoutSection->autoHeight = true;
    advancedLayoutSection->padding = { 0, 10, 0, 10 };
    advancedLayoutSection->visible.ComputedFrom([](bool showAdvanced) { return showAdvanced; }, _showAdvancedPositionSettings);

    auto overlayAnchorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    overlayAnchorRow->parentSize = { 1.0f, 0.0f };
    overlayAnchorRow->autoHeight = true;
    overlayAnchorRow->spacing = 10;
    overlayAnchorRow->padding = { 15, 0, 15, 10 };
    auto overlayAnchorLabel = Create<Label>(L"Anchor overlay");
    overlayAnchorLabel->size = { 90, 26 };
    overlayAnchorLabel->yTextAlign = Alignment::CENTER;
    auto overlayAnchorDropdown = Create<DropdownSelector>();
    overlayAnchorDropdown->size = { 100, 26 };
    overlayAnchorDropdown->backgroundColor = Color(0x101010);
    overlayAnchorDropdown->SetDropdownItems(_CreateAnchorPositionDropdownItems());
    overlayAnchorDropdown->selectedItemId = (int64_t)_overlayAnchor;
    overlayAnchorDropdown->text = _AnchorPositionToString(_overlayAnchor);
    overlayAnchorDropdown->SubscribeOnValueSelected([=](std::optional<int64_t> value) {
        if (value)
        {
            _overlayAnchor = (AnchorPosition)value.value();
            _UpdateLayoutString();
        }
    }).Detach();
    auto overlayAnchorLabel2 = Create<Label>(L"point");
    overlayAnchorLabel2->size = { 0, 26 };
    overlayAnchorLabel2->autoWidth = true;
    overlayAnchorLabel2->yTextAlign = Alignment::CENTER;
    overlayAnchorRow->AddItem(std::move(overlayAnchorLabel));
    overlayAnchorRow->AddItem(std::move(overlayAnchorDropdown));
    overlayAnchorRow->AddItem(std::move(overlayAnchorLabel2));
    
    auto displayAreaAnchorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    displayAreaAnchorRow->parentSize = { 1.0f, 0.0f };
    displayAreaAnchorRow->autoHeight = true;
    displayAreaAnchorRow->spacing = 10;
    displayAreaAnchorRow->padding = { 15, 0, 15, 10 };
    auto displayAreaAnchorLabel = Create<Label>(L"to display area");
    displayAreaAnchorLabel->size = { 90, 26 };
    displayAreaAnchorLabel->yTextAlign = Alignment::CENTER;
    auto displayAreaAnchorDropdown = Create<DropdownSelector>();
    displayAreaAnchorDropdown->size = { 100, 26 };
    displayAreaAnchorDropdown->backgroundColor = Color(0x101010);
    displayAreaAnchorDropdown->SetDropdownItems(_CreateAnchorPositionDropdownItems());
    displayAreaAnchorDropdown->selectedItemId = (int64_t)_displayAreaAnchor;
    displayAreaAnchorDropdown->text = _AnchorPositionToString(_displayAreaAnchor);
    displayAreaAnchorDropdown->SubscribeOnValueSelected([=](std::optional<int64_t> value) {
        if (value)
        {
            _displayAreaAnchor = (AnchorPosition)value.value();
            _UpdateLayoutString();
        }
    }).Detach();
    auto displayAreaAnchorLabel2 = Create<Label>(L"point");
    displayAreaAnchorLabel2->size = { 0, 26 };
    displayAreaAnchorLabel2->autoWidth = true;
    displayAreaAnchorLabel2->yTextAlign = Alignment::CENTER;
    displayAreaAnchorRow->AddItem(std::move(displayAreaAnchorLabel));
    displayAreaAnchorRow->AddItem(std::move(displayAreaAnchorDropdown));
    displayAreaAnchorRow->AddItem(std::move(displayAreaAnchorLabel2));

    advancedLayoutSection->AddItem(std::move(overlayAnchorRow));
    advancedLayoutSection->AddItem(std::move(displayAreaAnchorRow));

    auto advancedButton = Create<Button>(L"Advanced");
    NeutralButtonStyle::Apply(advancedButton.get());
    advancedButton->size = { 90, 26 };
    advancedButton->position = { 15, 0 };
    advancedButton->SetProperty(FlexMarginAfter(-26)); // Place next buttom at same vertical position
    advancedButton->Label()->text.ComputedFrom([](bool showAdvanced) { return showAdvanced ? L"Hide" : L"Advanced"; }, _showAdvancedPositionSettings);
    advancedButton->SubscribeOnActivated([=]() {
        _showAdvancedPositionSettings = !_showAdvancedPositionSettings;
    }).Detach();

    auto configureDisplayAreaButton = Create<Button>(L"Configure display area");
    NeutralButtonStyle::Apply(configureDisplayAreaButton.get());
    configureDisplayAreaButton->size = { 160, 26 };
    configureDisplayAreaButton->position = { 115, 0 };
    configureDisplayAreaButton->SubscribeOnActivated([=]() {
        _scene->GetApp()->Shared<SharedContext*>()->settingsWindow.OpenSettings(SettingsTab::OVERLAY);
    }).Detach();

    AddItem(Create<SectionHeader>(L"Layout"));
    AddItem(std::move(fillOverlayRow));
    AddItem(std::move(basicLayoutSection));
    AddItem(std::move(advancedLayoutSection));
    AddItem(std::move(advancedButton));
    AddItem(std::move(configureDisplayAreaButton));
}

void zcom::OverlayLayoutSetup::_ParseLayoutString(const std::wstring& str)
{
    std::array<std::wstring, 7> parts;
    split_wstr<7>(str, parts, L'|');

    _fillDisplayArea = parts[0] == L"1";
    _width = std::stoi(parts[1]);
    _height = std::stoi(parts[2]);
    _xOffset = std::stoi(parts[3]);
    _yOffset = std::stoi(parts[4]);
    int overlayAnchorValue = std::stoi(parts[5]);
    if (overlayAnchorValue >= 0 && overlayAnchorValue <= 8)
        _overlayAnchor = (AnchorPosition)overlayAnchorValue;
    int displayAreaAnchorValue = std::stoi(parts[6]);
    if (displayAreaAnchorValue >= 0 && displayAreaAnchorValue <= 8)
        _displayAreaAnchor = (AnchorPosition)displayAreaAnchorValue;
}

void zcom::OverlayLayoutSetup::_UpdateLayoutString()
{
    std::wostringstream ss(L"");
    ss << _fillDisplayArea << '|' << _width << '|' << _height << '|' << _xOffset << '|' << _yOffset << '|' << (int)_overlayAnchor << '|' << (int)_displayAreaAnchor;
    _overlayLayoutStringChangedEvent->InvokeAll(ss.str());
}

std::vector<zcom::DropdownItem> zcom::OverlayLayoutSetup::_CreateAnchorPositionDropdownItems() const
{
    std::vector<DropdownItem> items;
    items.push_back({ (int64_t)AnchorPosition::TOP_LEFT, L"Top left" });
    items.push_back({ (int64_t)AnchorPosition::TOP_CENTER, L"Top center" });
    items.push_back({ (int64_t)AnchorPosition::TOP_RIGHT, L"Top right" });
    items.push_back({ (int64_t)AnchorPosition::CENTER_LEFT, L"Center left" });
    items.push_back({ (int64_t)AnchorPosition::CENTER, L"Center" });
    items.push_back({ (int64_t)AnchorPosition::CENTER_RIGHT, L"Center right" });
    items.push_back({ (int64_t)AnchorPosition::BOTTOM_LEFT, L"Bottom left" });
    items.push_back({ (int64_t)AnchorPosition::BOTTOM_CENTER, L"Bottom center" });
    items.push_back({ (int64_t)AnchorPosition::BOTTOM_RIGHT, L"Bottom right" });
    return items;
}

std::wstring zcom::OverlayLayoutSetup::_AnchorPositionToString(AnchorPosition position) const
{
    auto items = _CreateAnchorPositionDropdownItems();
    for (auto& item : items)
        if (item.id == (int64_t)position)
            return item.text;
    return L"";
}

void zcom::ApplyLayoutStringToComponent(const std::wstring& layoutString, Component* component)
{
    std::array<std::wstring, 7> parts;
    split_wstr<7>(layoutString, parts, L'|');
    
    bool fillDisplayArea = parts[0] == L"1";
    if (fillDisplayArea)
    {
        component->size = { 0, 0 };
        component->position = { 0, 0 };
        component->parentSize = { 1.0f, 1.0f };
        component->xAlign = Alignment::START;
        component->yAlign = Alignment::START;
    }
    else
    {
        int w = std::stoi(parts[1]);
        int h = std::stoi(parts[2]);
        int x = std::stoi(parts[3]);
        int y = std::stoi(parts[4]);

        component->size = { w, h };
        component->parentSize = { 0.0f, 0.0f };

        AnchorPosition overlayAnchor = AnchorPosition::TOP_LEFT;
        AnchorPosition displayAreaAnchor = AnchorPosition::TOP_LEFT;

        int overlayAnchorValue = std::stoi(parts[5]);
        if (overlayAnchorValue >= 0 && overlayAnchorValue <= 8)
            overlayAnchor = (AnchorPosition)overlayAnchorValue;
        int displayAreaAnchorValue = std::stoi(parts[6]);
        if (displayAreaAnchorValue >= 0 && displayAreaAnchorValue <= 8)
            displayAreaAnchor = (AnchorPosition)displayAreaAnchorValue;

        int baseXOffset = 0;
        int baseYOffset = 0;

        switch (displayAreaAnchor)
        {
        case AnchorPosition::TOP_LEFT:      { component->xAlign = Alignment::START;  component->yAlign = Alignment::START;  baseXOffset = 0;     baseYOffset = 0;     break; }
        case AnchorPosition::TOP_CENTER:    { component->xAlign = Alignment::CENTER; component->yAlign = Alignment::START;  baseXOffset = w / 2; baseYOffset = 0;     break; }
        case AnchorPosition::TOP_RIGHT:     { component->xAlign = Alignment::END;    component->yAlign = Alignment::START;  baseXOffset = w;     baseYOffset = 0;     break; }
        case AnchorPosition::CENTER_LEFT:   { component->xAlign = Alignment::START;  component->yAlign = Alignment::CENTER; baseXOffset = 0;     baseYOffset = h / 2; break; }
        case AnchorPosition::CENTER:        { component->xAlign = Alignment::CENTER; component->yAlign = Alignment::CENTER; baseXOffset = w / 2; baseYOffset = h / 2; break; }
        case AnchorPosition::CENTER_RIGHT:  { component->xAlign = Alignment::END;    component->yAlign = Alignment::CENTER; baseXOffset = w;     baseYOffset = h / 2; break; }
        case AnchorPosition::BOTTOM_LEFT:   { component->xAlign = Alignment::START;  component->yAlign = Alignment::END;    baseXOffset = 0;     baseYOffset = h;     break; }
        case AnchorPosition::BOTTOM_CENTER: { component->xAlign = Alignment::CENTER; component->yAlign = Alignment::END;    baseXOffset = w / 2; baseYOffset = h;     break; }
        case AnchorPosition::BOTTOM_RIGHT:  { component->xAlign = Alignment::END;    component->yAlign = Alignment::END;    baseXOffset = w;     baseYOffset = h;     break; }
        }
        
        switch (overlayAnchor)
        {
        case AnchorPosition::TOP_LEFT:      { component->position = { baseXOffset + x,         baseYOffset + y };         break; }
        case AnchorPosition::TOP_CENTER:    { component->position = { baseXOffset + x - w / 2, baseYOffset + y };         break; }
        case AnchorPosition::TOP_RIGHT:     { component->position = { baseXOffset + x - w,     baseYOffset + y };         break; }
        case AnchorPosition::CENTER_LEFT:   { component->position = { baseXOffset + x,         baseYOffset + y - h / 2 }; break; }
        case AnchorPosition::CENTER:        { component->position = { baseXOffset + x - w / 2, baseYOffset + y - h / 2 }; break; }
        case AnchorPosition::CENTER_RIGHT:  { component->position = { baseXOffset + x - w,     baseYOffset + y - h / 2 }; break; }
        case AnchorPosition::BOTTOM_LEFT:   { component->position = { baseXOffset + x,         baseYOffset + y - h };     break; }
        case AnchorPosition::BOTTOM_CENTER: { component->position = { baseXOffset + x - w / 2, baseYOffset + y - h };     break; }
        case AnchorPosition::BOTTOM_RIGHT:  { component->position = { baseXOffset + x - w,     baseYOffset + y - h };     break; }
        }
    }
}
