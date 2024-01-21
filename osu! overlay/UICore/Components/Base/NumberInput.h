#pragma once

#include "App.h"
#include "Button.h"
#include "TextInput.h"
#include "Window/KeyboardEventHandler.h"

#include "Helper/ResourceManager.h"
#include "Helper/Time.h"
#include "Helper/decimal.h"

#define MAX_DEC_PRECISION 7

typedef dec::decimal<MAX_DEC_PRECISION> NumberInputValue;

namespace zcom
{
    class NumberInput : public TextInput
    {
    public:
        NumberInputValue GetValue() const
        {
            return _value;
        }

        int GetPrecision() const
        {
            return _precision;
        }

        NumberInputValue GetStepSize() const
        {
            return _stepSize;
        }

        NumberInputValue GetMinValue() const
        {
            return _minValue;
        }

        NumberInputValue GetMaxValue() const
        {
            return _maxValue;
        }

        void SetValue(NumberInputValue value)
        {
            if (value == _value)
                return;

            _value = value;
            _valueChangedEvent->InvokeAll(_value);
            _BoundValue();
            _UpdateText();
        }

        void SetPrecision(int precision)
        {
            if (precision < 0)
                precision = 0;
            if (precision > MAX_DEC_PRECISION)
                precision = MAX_DEC_PRECISION;
            if (precision == _precision)
                return;

            _precision = precision;
            _UpdateText();
        }

        void SetStepSize(NumberInputValue stepSize)
        {
            _stepSize = stepSize;
        }

        void SetMinValue(NumberInputValue value)
        {
            _minValue = value;
            if (_maxValue < _minValue)
                _maxValue = _minValue;
            _BoundValue();
        }

        void SetMaxValue(NumberInputValue value)
        {
            _maxValue = value;
            if (_minValue > _maxValue)
                _minValue = _maxValue;
            _BoundValue();
        }

        EventSubscription<void, NumberInputValue> AddOnValueChanged(std::function<void(NumberInputValue)> handler)
        {
            return _valueChangedEvent->Subscribe(handler);
        }


    protected:
        friend class Scene;
        friend class Component;
        NumberInput(Scene* scene) : TextInput(scene) {}
        void Init()
        {
            TextInput::Init();
            SetTextAreaMargins({ 0, 0, 19, 0 });

            auto valueUpButton = Create<Button>(L"");
            valueUpButton->SetBaseWidth(19);
            valueUpButton->SetParentHeightPercent(0.5f);
            valueUpButton->SetAlignment(Alignment::END, Alignment::START);
            valueUpButton->SetPreset(ButtonPreset::NO_EFFECTS);
            valueUpButton->SetButtonImageAll(ResourceManagerOld::GetImage("menu_arrow_up_7x7"));
            valueUpButton->ButtonImage()->SetPlacement(ImagePlacement::BOTTOM_CENTER);
            valueUpButton->ButtonImage()->SetImageOffsetY(-1.0f);
            valueUpButton->ButtonImage()->SetPixelSnap(true);
            valueUpButton->UseImageParamsForAll(valueUpButton->ButtonImage());
            valueUpButton->ButtonImage()->SetTintColor(D2D1::ColorF(0.5f, 0.5f, 0.5f));
            valueUpButton->ButtonHoverImage()->SetTintColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
            valueUpButton->ButtonClickImage()->SetTintColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
            valueUpButton->SetSelectable(false);
            valueUpButton->SetActivation(zcom::ButtonActivation::PRESS);
            valueUpButton->SubscribeOnActivated([&]()
            {
                _UpdateValue();
                SetValue(_value + _stepSize);
            }).Detach();

            auto valueDownButton = Create<Button>(L"");
            valueDownButton->SetBaseWidth(19);
            valueDownButton->SetParentHeightPercent(0.5f);
            valueDownButton->SetAlignment(Alignment::END, Alignment::END);
            valueDownButton->SetPreset(ButtonPreset::NO_EFFECTS);
            valueDownButton->SetButtonImageAll(ResourceManagerOld::GetImage("menu_arrow_down_7x7"));
            valueDownButton->ButtonImage()->SetPlacement(ImagePlacement::TOP_CENTER);
            valueDownButton->ButtonImage()->SetImageOffsetY(1.0f);
            valueDownButton->ButtonImage()->SetPixelSnap(true);
            valueDownButton->UseImageParamsForAll(valueDownButton->ButtonImage());
            valueDownButton->ButtonImage()->SetTintColor(D2D1::ColorF(0.5f, 0.5f, 0.5f));
            valueDownButton->ButtonHoverImage()->SetTintColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
            valueDownButton->ButtonClickImage()->SetTintColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
            valueDownButton->SetSelectable(false);
            valueDownButton->SetActivation(zcom::ButtonActivation::PRESS);
            valueDownButton->SubscribeOnActivated([&]()
            {
                _UpdateValue();
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
                    _UpdateValue();
            }).Detach();
        }
    public:
        ~NumberInput()
        {

        }
        NumberInput(NumberInput&&) = delete;
        NumberInput& operator=(NumberInput&&) = delete;
        NumberInput(const NumberInput&) = delete;
        NumberInput& operator=(const NumberInput&) = delete;

    private:
        NumberInputValue _value;
        int _precision = 0;
        NumberInputValue _stepSize;
        NumberInputValue _minValue;
        NumberInputValue _maxValue;

        bool _internalChange = false;

        EventEmitter<void, NumberInputValue> _valueChangedEvent;

    private:
        void _UpdateValue()
        {
            NumberInputValue number(wstring_to_string(Text()->GetText()));
            SetValue(number);
            _UpdateText();
        }

        void _UpdateText()
        {
            std::ostringstream ss;
            ss << _value;
            std::wstring str = string_to_wstring(ss.str());

            // Cut off unnecessary decimal points
            if (_precision == 0)
                str = str.substr(0, str.length() - (MAX_DEC_PRECISION + 1));
            else
                str = str.substr(0, str.length() - MAX_DEC_PRECISION + _precision);

            _internalChange = true;
            Text()->SetText(str);
            _internalChange = false;
        }

        void _BoundValue()
        {
            if (_value < _minValue)
                SetValue(_minValue);
            else if (_value > _maxValue)
                SetValue(_maxValue);
        }

#pragma region base_class
    protected:
        EventTargets _OnMouseMove(int deltaX, int deltaY)
        {
            return TextInput::_OnMouseMove(deltaX, deltaY);
        }

        void _OnDeselected()
        {
            TextInput::_OnDeselected();
            _UpdateValue();
        }

        EventTargets _OnWheelUp(int x, int y)
        {
            _UpdateValue();
            SetValue(_value + _stepSize);
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnWheelDown(int x, int y)
        {
            _UpdateValue();
            SetValue(_value - _stepSize);
            return EventTargets().Add(this, x, y);
        }

    public:
        const char* GetName() const { return Name(); }
        static const char* Name() { return "number_input"; }
#pragma endregion
    };
}