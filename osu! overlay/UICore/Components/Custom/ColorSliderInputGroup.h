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
        void Init(std::wstring label, uint8_t initialValue = 0)
        {
            FlexPanel::Init(FlexDirection::RIGHT);
            spacing = 10;
            itemAlignment = Alignment::CENTER;
            _label = Create<Label>(label);
            _label->size = { 50, 26 };
            _label->yTextAlign = Alignment::CENTER;
            _label->xTextAlign = TextAlignment::TRAILING;
            _slider = Create<Slider>();
            _slider->value = (float)initialValue / 255.0f;
            _slider->size = { 0, 26 };
            _slider->bodyStartOffset = 2;
            _slider->bodyEndOffset = 2;
            _slider->interactionAreaMargins = { 0, 4, 0, 4 };
            _slider->anchorOffset = -2;
            _slider->eatScrollEvents = true;
            _slider->SetProperty(FlexGrow());
            auto sliderBody = Create<Dummy>();
            sliderBody->parentSize = { 1.0f, 0.0f };
            sliderBody->size = { -4, 14 };
            sliderBody->position = { 2, 0 };
            sliderBody->yAlign = Alignment::CENTER;
            auto sliderAnchor = Create<Dummy>();
            sliderAnchor->size = { 5, 20 };
            sliderAnchor->yAlign = Alignment::CENTER;
            auto bitmapHovered = _scene->GetWindow()->resourceManager.GetImage("slider_anchor_hovered");
            auto bitmapRegular = _scene->GetWindow()->resourceManager.GetImage("slider_anchor");
            sliderAnchor->backgroundImage.ComputedFrom([bitmapHovered, bitmapRegular](bool hovered) {
                if (hovered)
                    return bitmapHovered;
                else
                    return bitmapRegular;
            }, _slider->insideInteractionArea_);
            _input = Create<NumberInput>();
            _input->size = { 60, 26 };
            _input->value = NumberInputValue((int)initialValue);
            _input->minValue = NumberInputValue(0);
            _input->maxValue = NumberInputValue(255);
            _input->stepSize = NumberInputValue(5);
            _input->backgroundColor = Color(0x101010);
            _input->border.cornerRadius = 2.0f;
            _input->SubscribeOnValueChanged([=](NumberInputValue value) {
                this->value = (uint8_t)value.getAsInteger();
            }).Detach();
            _slider->SubscribeOnValueChanged([=](Slider*, float* newValue) {
                this->value = (uint8_t)((*newValue) * 255);
            }).Detach();
            // Use post action events, since changing slider value before scroll event
            // TODO: investigate unfinished comment ^
            _slider->SubscribePostWheelUp([=](Component*, std::vector<EventContext::Params>, Point) {
                int newValue = (int)this->value + 5;
                this->value = uint8_t(newValue > 255 ? 255 : newValue);
            }).Detach();
            _slider->SubscribePostWheelDown([=](Component*, std::vector<EventContext::Params>, Point) {
                int newValue = (int)this->value - 5;
                this->value = uint8_t(newValue < 0 ? 0 : newValue);
            }).Detach();
            _slider->SetBodyComponent(std::move(sliderBody));
            _slider->SetAnchorComponent(std::move(sliderAnchor));
            AddItem(_label.get());
            AddItem(_slider.get());
            AddItem(_input.get());

            value = initialValue;
        }

    public:
        Label* GetLabel() { return _label.get(); }
        Slider* GetSlider() { return _slider.get(); }
        NumberInput* GetInput() { return _input.get(); }

        Value<uint8_t> value = Value<uint8_t>(0, [=](uint8_t& currentValue, const uint8_t& newValue) {
            currentValue = newValue;
            _slider->value = newValue / 255.0f;
            _input->value = NumberInputValue(newValue);
        });

    private:
        std::unique_ptr<Label> _label = nullptr;
        std::unique_ptr<Slider> _slider = nullptr;
        std::unique_ptr<NumberInput> _input = nullptr;
    };
}