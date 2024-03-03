#include "NumberInput.h"
#include "Window/Window.h"

void zcom::NumberInput::Init()
{
    TextInput::Init();
    SetTextAreaMargins({ 0, 0, 19, 0 });

    auto valueUpButton = Create<Button>(L"");
    valueUpButton->SetBaseWidth(19);
    valueUpButton->SetParentHeightPercent(0.5f);
    valueUpButton->SetAlignment(Alignment::END, Alignment::START);
    valueUpButton->SetPreset(ButtonPreset::NO_EFFECTS);
    valueUpButton->SetButtonImageAll(_scene->GetWindow()->resourceManager.GetImage("menu_arrow_up_7x7"));
    valueUpButton->ButtonImage()->SetPlacement(ImagePlacement::BOTTOM_CENTER);
    valueUpButton->ButtonImage()->SetImageOffsetY(-1.0f);
    valueUpButton->ButtonImage()->SetPixelSnap(true);
    valueUpButton->UseImageParamsForAll(valueUpButton->ButtonImage());
    valueUpButton->ButtonImage()->SetTintColor(D2D1::ColorF(0.5f, 0.5f, 0.5f));
    valueUpButton->ButtonHoverImage()->SetTintColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
    valueUpButton->ButtonClickImage()->SetTintColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
    valueUpButton->SetSelectable(false);
    valueUpButton->SetActivation(zcom::ButtonActivation::PRESS);
    valueUpButton->SubscribeOnActivated([&]() {
        SetValue(_value + _stepSize);
    }).Detach();

    auto valueDownButton = Create<Button>(L"");
    valueDownButton->SetBaseWidth(19);
    valueDownButton->SetParentHeightPercent(0.5f);
    valueDownButton->SetAlignment(Alignment::END, Alignment::END);
    valueDownButton->SetPreset(ButtonPreset::NO_EFFECTS);
    valueDownButton->SetButtonImageAll(_scene->GetWindow()->resourceManager.GetImage("menu_arrow_down_7x7"));
    valueDownButton->ButtonImage()->SetPlacement(ImagePlacement::TOP_CENTER);
    valueDownButton->ButtonImage()->SetImageOffsetY(1.0f);
    valueDownButton->ButtonImage()->SetPixelSnap(true);
    valueDownButton->UseImageParamsForAll(valueDownButton->ButtonImage());
    valueDownButton->ButtonImage()->SetTintColor(D2D1::ColorF(0.5f, 0.5f, 0.5f));
    valueDownButton->ButtonHoverImage()->SetTintColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
    valueDownButton->ButtonClickImage()->SetTintColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
    valueDownButton->SetSelectable(false);
    valueDownButton->SetActivation(zcom::ButtonActivation::PRESS);
    valueDownButton->SubscribeOnActivated([&]() {
        SetValue(_value - _stepSize);
    }).Detach();

    AddItem(std::move(valueUpButton));
    AddItem(std::move(valueDownButton));

    _value = 0;
    _minValue = std::numeric_limits<int32_t>::min();
    _maxValue = std::numeric_limits<int32_t>::max();
    _stepSize = 1;
    _UpdateText();

    SetPattern(L"^[-\\.0-9]+$");
    SubscribeOnTextChanged([&](Label* label, std::wstring* newText)
    {
        if (!_internalChange)
            SetValue(NumberInputValue(wstring_to_string(*newText)));
    }).Detach();
}