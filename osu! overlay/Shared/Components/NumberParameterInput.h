#pragma once

#include "Components/Base/FlexPanel.h"
#include "Components/Base/NumberInput.h"
#include "Components/Base/Label.h"
#include "Components/Base/Image.h"

namespace zcom
{
    template<typename T>
    struct NumberParameterInputParams
    {
        std::wstring inputTitle;
        std::wstring inputDescription;
        T initialValue;
        T minValue;
        T maxValue;
        T stepSize;
        bool requiresMemory;

        NumberParameterInputParams(
            std::wstring inputTitle,
            std::wstring inputDescription,
            T initialValue,
            T minValue,
            T maxValue,
            T stepSize,
            bool requiresMemory = false)
        {
            this->inputTitle = inputTitle;
            this->inputDescription = inputDescription;
            this->initialValue = initialValue;
            this->minValue = minValue;
            this->maxValue = maxValue;
            this->stepSize = stepSize;
            this->requiresMemory = requiresMemory;
        }
    };

    template<typename T>
    class NumberParameterInput : public FlexPanel
    {
        DEFINE_COMPONENT(NumberParameterInput, FlexPanel)
        DEFAULT_DESTRUCTOR(NumberParameterInput)
    protected:
        void Init(NumberParameterInputParams<T> params)
        {
            FlexPanel::Init(FlexDirection::RIGHT);
            FillContainerWidth();
            SetSpacing(10);
            SetPadding({ 15, 0, 15, 10 });

            _input = Create<NumberInput>();
            _input->SetBaseSize(60, 26);
            _input->SetValue(NumberInputValue(params.initialValue));
            _input->SetMinValue(NumberInputValue(params.minValue));
            _input->SetMaxValue(NumberInputValue(params.maxValue));
            _input->SetStepSize(NumberInputValue(params.stepSize));
            _input->SetBackgroundColor(D2D1::ColorF(0x101010));
            _input->SetCornerRounding(2.0f);

            _label = Create<Label>(params.inputTitle);
            _label->SetBaseHeight(26);
            _label->AutomaticWidth();
            _label->SetVerticalTextAlignment(Alignment::CENTER);
            _label->SetHoverText(params.inputDescription);

            AddItem(_input.get());
            AddItem(_label.get());
            if (params.requiresMemory)
            {
                _icon = Create<Image>(_scene->GetWindow()->resourceManager.GetImage("osu_memory"));
                _icon->SetBaseSize(26, 26);
                _label->SetProperty(FlexGrow());
                AddItem(_icon.get());
            }
        }

    public:
        NumberInput* GetInput() { return _input.get(); }
        Label* GetLabel() { return _label.get(); }
        Image* GetIcon() { return _icon.get(); }

    private:
        std::unique_ptr<NumberInput> _input = nullptr;
        std::unique_ptr<Label> _label = nullptr;
        std::unique_ptr<Image> _icon = nullptr;
    };
}