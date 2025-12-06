#include "NumberInput.h"
#include "Window/Window.h"

void zcom::NumberInput::Init()
{
    TextInput::Init();

    TextPanel()->size = { -19, 0 };
    TextPanel()->parentSize = { 1.0f, 1.0f };

    auto valueUpButton = Create<Button>(L"", ButtonPreset::NO_EFFECTS);
    valueUpButton->size = { 19, 0 };
    valueUpButton->parentSize = { 0.0f, 0.5f };
    valueUpButton->xAlign = Alignment::END;
    valueUpButton->yAlign = Alignment::START;
    valueUpButton->Image()->image = _scene->GetWindow()->resourceManager.GetImage("menu_arrow_up_7x7");
    valueUpButton->Image()->imagePlacement = ImagePlacement::BOTTOM_CENTER;
    valueUpButton->Image()->yOffset.ComputedFrom([](float offset) { return -offset; }, arrowImageGap);
    valueUpButton->Image()->snapToPixels = true;
    valueUpButton->ValueFromButtonState<Color>(valueUpButton->Image()->tintColor,
        Color(0x808080),
        Color(0x0070DD),
        Color(0x0070DD)
    );
    valueUpButton->selectable = false;
    valueUpButton->activation = ButtonActivation::PRESS;
    valueUpButton->SubscribeOnActivated([&]() {
        StepUp();
        _valueChangedEvent->InvokeAll(value);
    }).Detach();
    
    auto valueDownButton = Create<Button>(L"", ButtonPreset::NO_EFFECTS);
    valueDownButton->size = { 19, 0 };
    valueDownButton->parentSize = { 0.0f, 0.5f };
    valueDownButton->xAlign = Alignment::END;
    valueDownButton->yAlign = Alignment::END;
    valueDownButton->Image()->image = _scene->GetWindow()->resourceManager.GetImage("menu_arrow_down_7x7");
    valueDownButton->Image()->imagePlacement = ImagePlacement::TOP_CENTER;
    valueDownButton->Image()->yOffset.ComputedFrom([](float offset) { return offset; }, arrowImageGap);
    valueDownButton->Image()->snapToPixels = true;
    valueDownButton->ValueFromButtonState<Color>(valueDownButton->Image()->tintColor,
        Color(0x808080),
        Color(0x0070DD),
        Color(0x0070DD)
    );
    valueDownButton->selectable = false;
    valueDownButton->activation = ButtonActivation::PRESS;
    valueDownButton->SubscribeOnActivated([&]() {
        StepDown();
        _valueChangedEvent->InvokeAll(value);
    }).Detach();

    AddItem(std::move(valueUpButton));
    AddItem(std::move(valueDownButton));

    _UpdateText();

    // This pattern matches any decimal number with some leniences to allow better typing experience,
    // such as allowing a minus sign by itself to permit users to start typing negative numbers, or
    // allowing a string of left zeroes, since that can be formed by deleting a dot from a number
    // like '0.003'.
    // Such leniency is ok, because at all times the parsed number ('value') is valid, and its string
    // representation will be shown after deselecting the input field.
    pattern = L"^-?(0|[1-9]\\d*)?(\\.)?(\\d+)?$";
    SubscribeOnTextChanged([=](std::wstring* newText)
    {
        if (!_internalChange)
        {
            value = NumberInputValue(wstring_to_string(*newText));
            _valueChangedEvent->InvokeAll(value);
        }
        // It is possible to update newText here to align with the new value string representation,
        // but it is a bad idea, because it messes too much with the text while it is being typed.
        // 
        // For example, typing a dot in the middle of '100000' immediatelly converts it to '100' if
        // 0 precision is set, which is unintuitive.
        // Therefore, the update is performed on deselect only, since that leaves the text input in
        // a clean state when the user is done typing.
    }).Detach();
    SubscribeOnDeselected([=](Component* item) {
        _UpdateText();
    }).Detach();
}