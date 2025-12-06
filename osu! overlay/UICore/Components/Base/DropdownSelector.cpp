#include "DropdownSelector.h"

zcom::DropdownSelector::~DropdownSelector()
{
    _scene->GetBasePanel()->RemoveItem(_dropdownPanel.get());
    _dropdownPanel->ClearItems();
    _dropdownScrollPanel->ClearItems();
    _dropdownItemsPanel->ClearItems();
}

void zcom::DropdownSelector::Init()
{
    TextInput::Init();

    TextPanel()->size = { -19, 0 };
    TextPanel()->parentSize = { 1.0f, 1.0f };

    auto arrowImage = Create<Image>();
    arrowImage->size = { 22, 0 };
    arrowImage->parentSize = { 0.0f, 1.0f };
    arrowImage->xAlign = Alignment::END;
    arrowImage->imagePlacement = ImagePlacement::CENTER;
    arrowImage->snapToPixels = true;
    arrowImage->tintColor = Color(0x808080);
    std::optional<Bitmap> openArrow = _scene->GetWindow()->resourceManager.GetImage("menu_arrow_down_7x7");
    std::optional<Bitmap> closeArrow = _scene->GetWindow()->resourceManager.GetImage("menu_arrow_up_7x7");
    arrowImage->image.ComputedFrom([openArrow, closeArrow](bool selected) {
        return selected ? closeArrow : openArrow;
    }, selected_);

    AddItem(std::move(arrowImage));

    _dropdownPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _dropdownPanel->size.ComputedFrom([=](Size inputSize) {
        return Size{ inputSize.width, _dropdownPanel->size->height };
    }, size_);
    _dropdownPanel->autoHeight = true;
    _dropdownPanel->position.ComputedFrom([](Point inputWindowPosition, Point basePanelWindowPosition, Size inputSize, Size panelSize, DropdownPlacement placement) {
        if (placement == DropdownPlacement::BELOW)
            return (inputWindowPosition - basePanelWindowPosition) + Point{ 0, inputSize.height - 1 };
        else if (placement == DropdownPlacement::ABOVE)
            return (inputWindowPosition - basePanelWindowPosition) - Point{ 0, panelSize.height - 1 };
        else
            return Point{ 0, 0 };
    }, windowPosition_, _scene->GetBasePanel()->windowPosition_, size_, _dropdownPanel->size_, dropdownPlacement);
    _dropdownPanel->backgroundColor = backgroundColor.Get();
    _dropdownPanel->border.visible = true;
    _dropdownPanel->border.color = border.color.Get();
    _dropdownPanel->zIndex = 1000;
    _dropdownPanel->visible.ComputedFrom([](bool selected) { return selected; }, selected_);

    _dropdownItemsPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _dropdownItemsPanel->parentSize = { 1.0f, 0.0f };
    _dropdownItemsPanel->autoHeight = true;
    _noResultsLabel = Create<Label>(L"No results");
    _noResultsLabel->parentSize = { 1.0f, 0.0f };
    _noResultsLabel->size = { 0, 26 };
    _noResultsLabel->xTextAlign = TextAlignment::CENTER;
    _noResultsLabel->yTextAlign = Alignment::CENTER;
    _noResultsLabel->fontColor = Color(0x4D4D4D);
    _noResultsLabel->fontStyle = FontStyle::ITALIC;
    _noResultsLabel->visible = false;
    _dropdownScrollPanel = Create<ScrollPanel>();
    _dropdownScrollPanel->parentSize = { 1.0f, 0.0f };
    _dropdownScrollPanel->size.ComputedFrom([=](Size itemListSize, Size noResultsLabelSize, int maxHeight) {
        return Size{
            _dropdownScrollPanel->size->width,
            itemListSize.height > maxHeight ? maxHeight : (itemListSize.height < noResultsLabelSize.height ? noResultsLabelSize.height : itemListSize.height)
        };
    }, _dropdownItemsPanel->size_, _noResultsLabel->size_, maxListHeight);
    _dropdownScrollPanel->yScrollbar.scrollable = true;

    _dropdownScrollPanel->AddItem(_dropdownItemsPanel.get());
    _dropdownScrollPanel->AddItem(_noResultsLabel.get());
    _dropdownPanel->AddItem(_dropdownScrollPanel.get());
    _scene->GetBasePanel()->AddItem(_dropdownPanel.get());

    _searchMapper = [](DropdownSelector* component, const std::wstring& searchString, std::vector<DropdownItem> baseItems) {
        if (searchString.empty())
            return baseItems;

        std::vector<DropdownItem> resultList;
        std::wstring searchStringLower = to_lowercase(searchString);
        for (auto& item : baseItems)
        {
            if (to_lowercase(item.text).find(searchStringLower.c_str()) != std::string::npos)
                resultList.push_back(item);
        }
        return resultList;
    };

    SubscribeOnTextChanged([=](std::wstring* newText)
    {
        if (!_internalChange)
        {
            _DoSearch(*newText);
        }
    }).Detach();
}

void zcom::DropdownSelector::_DoSearch(const std::wstring& searchString)
{
    std::vector<DropdownItem> selectableItems;
    if (_searchMapper)
        selectableItems = _searchMapper(this, searchString, _baseDropdownItems);
    else
        selectableItems = _baseDropdownItems;

    _dropdownItemsPanel->DeferLayoutUpdates();
    int i = 0;
    for (; i < selectableItems.size(); i++)
    {
        if (i < _dropdownItemsPanel->ItemCount())
        {
            DropdownItemComponent* dropdownItem = (DropdownItemComponent*)_dropdownItemsPanel->GetItem(i);
            dropdownItem->itemId = selectableItems[i].id;
            dropdownItem->text = selectableItems[i].text;
            dropdownItem->visible = true;
        }
        else
        {
            auto dropdownItem = Create<DropdownItemComponent>();
            dropdownItem->parentSize = { 1.0f, 0.0f };
            dropdownItem->size = { 0, 26 };
            dropdownItem->xTextAlign = TextAlignment::LEADING;
            dropdownItem->yTextAlign = Alignment::CENTER;
            dropdownItem->padding = { 5.0f, 0.0f, 5.0f, 0.0f };
            dropdownItem->backgroundColor.ComputedFrom([](bool hovered) {
                return hovered ? Color(0xFFFFFF, 0.1f) : Color();
            }, dropdownItem->hovered_);

            dropdownItem->itemId = selectableItems[i].id;
            dropdownItem->text = selectableItems[i].text;
            dropdownItem->SubscribeOnLeftPressed([=](Component* item, Point) {
                DropdownItemComponent* dropdownItem = (DropdownItemComponent*)item;
                selectedItemId = dropdownItem->itemId;
                text = dropdownItem->text;
                _initialValue = std::nullopt;
                _valueSelectedEventEmitter->InvokeAll(dropdownItem->itemId);
            }).Detach();

            _dropdownItemsPanel->AddItem(std::move(dropdownItem));
        }
    }
    for (; i < _dropdownItemsPanel->ItemCount(); i++)
    {
        _dropdownItemsPanel->GetItem(i)->visible = false;
    }
    _noResultsLabel->visible = selectableItems.empty();
    _dropdownItemsPanel->ResumeLayoutUpdates();
}

zcom::EventContext zcom::DropdownSelector::_OnLeftPressed(Point point)
{
    TextInput::_OnLeftPressed(point);
    // Override TextInput targets, DropdownSelector should be selected on any click
    return EventContext().Add(this, point);
}

void zcom::DropdownSelector::_OnSelected(bool reverse)
{
    TextInput::_OnSelected(reverse);
    if (selectedItemId->has_value())
        _initialValue = { selectedItemId->value(), text };
    else
        _initialValue = std::nullopt;
    text = L"";
    _DoSearch(L"");
}

void zcom::DropdownSelector::_OnDeselected()
{
    TextInput::_OnDeselected();
    if (_initialValue)
    {
        selectedItemId = _initialValue->id;
        text = _initialValue->text;
    }
}
