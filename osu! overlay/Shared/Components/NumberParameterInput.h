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
            parentSize = { 1.0f, 0.0f };
            autoHeight = true;
            spacing = 10;
            padding = { 15, 0, 15, 10 };

            _input = Create<NumberInput>();
            _input->size = { 60, 26 };
            _input->value = NumberInputValue(params.initialValue);
            _input->minValue = NumberInputValue(params.minValue);
            _input->maxValue = NumberInputValue(params.maxValue);
            _input->stepSize = NumberInputValue(params.stepSize);
            _input->backgroundColor = Color(0x101010);
            _input->border.cornerRadius = 2.0f;

            _label = Create<Label>(params.inputTitle);
            _label->size = { 0, 26 };
            _label->autoWidth = true;
            _label->yTextAlign = Alignment::CENTER;
            _label->hoverText = params.inputDescription;

            AddItem(_input.get());
            AddItem(_label.get());
            if (params.requiresMemory)
            {
                _icon = Create<Image>(_scene->GetWindow()->resourceManager.GetImage("osu_memory"));
                _icon->size = { 26, 26 };
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