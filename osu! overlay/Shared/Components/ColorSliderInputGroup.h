#pragma once

#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Slider.h"
#include "Components/Base/NumberInput.h"

namespace zcom
{
    class ColorSliderInputGroup : public FlexPanel
    {
        DEFINE_COMPONENT(ColorSliderInputGroup, FlexPanel)
        DEFAULT_DESTRUCTOR(ColorSliderInputGroup)
    protected:
        void Init(std::wstring label)
        {
            FlexPanel::Init(FlexDirection::RIGHT);
            SetSpacing(10);
            SetItemAlignment(Alignment::CENTER);
            _label = Create<Label>(label);
            _label->SetBaseSize(50, 26);
            _label->SetVerticalTextAlignment(Alignment::CENTER);
            _label->SetHorizontalTextAlignment(TextAlignment::TRAILING);
            _slider = Create<Slider>();
            _slider->SetValue(0.0f);
            _slider->SetBaseHeight(26);
            _slider->SetSliderBodyOffset(2, 2);
            _slider->SetInteractionAreaMargins({ 0, 4, 0, 4 });
            _slider->SetAnchorOffset(-2);
            _slider->SetProperty(FlexGrow());
            _slider->SetEatScrollEvents(true);
            auto opacitySliderBody = Create<Dummy>();
            opacitySliderBody->SetParentWidthPercent(1.0f);
            opacitySliderBody->SetBaseSize(-4, 14);
            opacitySliderBody->SetHorizontalOffsetPixels(2);
            opacitySliderBody->SetVerticalAlignment(Alignment::CENTER);
            //opacitySliderBody->SubscribePostDraw([=](Component* item, Graphics g) {
            //    D2D1_COLOR_F cellColor1 = D2D1::ColorF(0x404040);
            //    D2D1_COLOR_F cellColor2 = D2D1::ColorF(0x303030);
            //    _DrawCheckeredPattern(item, cellColor1, cellColor2, g);
            //    D2D1_COLOR_F startColor = D2D1::ColorF(_currentColor.r / 255.0f, _currentColor.g / 255.0f, _currentColor.b / 255.0f, 0.0f);
            //    D2D1_COLOR_F endColor = D2D1::ColorF(_currentColor.r / 255.0f, _currentColor.g / 255.0f, _currentColor.b / 255.0f, 1.0f);
            //    _DrawGradient(item, startColor, endColor, g);
            //}).Detach();
            auto opacitySliderAnchor = Create<Dummy>();
            opacitySliderAnchor->SetBaseSize(5, 20);
            opacitySliderAnchor->SetVerticalAlignment(Alignment::CENTER);
            opacitySliderAnchor->SetBackgroundImage(_scene->GetWindow()->resourceManager.GetImage("slider_anchor"));
            _input = Create<NumberInput>();
            _input->SetBaseSize(60, 26);
            _input->SetValue(NumberInputValue(0));
            _input->SetMinValue(NumberInputValue(0));
            _input->SetMaxValue(NumberInputValue(255));
            _input->SetStepSize(NumberInputValue(3));
            _input->SetBackgroundColor(D2D1::ColorF(0x101010));
            _input->SetCornerRounding(2.0f);
            _input->SubscribeOnValueChanged([=](NumberInputValue value) {
                _slider->SetValue(value.getAsInteger() / 255.0f, false);
            }).Detach();
            _slider->SubscribeOnValueChanged([=](Slider*, float* newValue) {
                _input->SetValue(NumberInputValue((*newValue) * 255), true);
            }).Detach();
            _slider->SubscribeOnEnterInteractionArea([=](Slider* slider) {
                slider->GetAnchorComponent()->SetBackgroundImage(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_hovered"));
            }).Detach();
            _slider->SubscribeOnLeaveInteractionArea([=](Slider* slider) {
                slider->GetAnchorComponent()->SetBackgroundImage(_scene->GetWindow()->resourceManager.GetImage("slider_anchor"));
            }).Detach();
            // Use post action events, since changing slider value before scroll event
            _slider->SubscribePostWheelUp([=](Component*, std::vector<EventTargets::Params>, int, int) {
                _input->StepUp();
            }).Detach();
            _slider->SubscribePostWheelDown([=](Component*, std::vector<EventTargets::Params>, int, int) {
                _input->StepDown();
            }).Detach();
            _slider->SetBodyComponent(std::move(opacitySliderBody));
            _slider->SetAnchorComponent(std::move(opacitySliderAnchor));
            AddItem(_label.get());
            AddItem(_slider.get());
            AddItem(_input.get());
        }

    public:
        Label* GetLabel() { return _label.get(); }
        Slider* GetSlider() { return _slider.get(); }
        NumberInput* GetInput() { return _input.get(); }

    private:
        std::unique_ptr<Label> _label = nullptr;
        std::unique_ptr<Slider> _slider = nullptr;
        std::unique_ptr<NumberInput> _input = nullptr;
    };
}