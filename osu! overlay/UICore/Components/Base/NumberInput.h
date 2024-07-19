#pragma once

#include "App.h"
#include "Button.h"
#include "TextInput.h"
#include "Window/KeyboardEventHandler.h"

#include "Helper/ResourceManager.h"
#include "Helper/Time.h"
#include "Helper/decimal.h"

#include <iostream>

#define MAX_DEC_PRECISION 7

typedef dec::decimal<MAX_DEC_PRECISION> NumberInputValue;

namespace zcom
{
    class NumberInput : public TextInput
    {
        DEFINE_COMPONENT(NumberInput, TextInput)
        DEFAULT_DESTRUCTOR(NumberInput)
    protected:
        void Init();

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

        void SetValue(NumberInputValue value, bool emitChangeEvent = true)
        {
            if (value == _value)
                return;

            _value = value;
            if (emitChangeEvent)
                _valueChangedEvent->InvokeAll(_value);
            _BoundValue();
            _UpdateText();
        }

        void StepUp()
        {
            SetValue(GetValue() + GetStepSize());
        }

        void StepDown()
        {
            SetValue(GetValue() - GetStepSize());
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

        EventSubscription<void, NumberInputValue> SubscribeOnValueChanged(std::function<void(NumberInputValue)> handler)
        {
            return _valueChangedEvent->Subscribe(handler);
        }

    private:
        NumberInputValue _value;
        int _precision = 0;
        NumberInputValue _stepSize;
        NumberInputValue _minValue;
        NumberInputValue _maxValue;

        bool _internalChange = false;

        EventEmitter<void, NumberInputValue> _valueChangedEvent;


        void _UpdateValue()
        {
            SetValue(NumberInputValue(wstring_to_string(Text()->GetText())));
        }

        void _UpdateText()
        {
            std::ostringstream ss;
            ss << _value;
            std::string s = ss.str();
            std::wstring str = std::wstring(s.begin(), s.end());

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

    protected:
        void _OnDeselected() override
        {
            TextInput::_OnDeselected();
            _UpdateValue();
        }

        EventTargets _OnWheelUp(int x, int y) override
        {
            _UpdateValue();
            SetValue(_value + _stepSize);
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnWheelDown(int x, int y) override
        {
            _UpdateValue();
            SetValue(_value - _stepSize);
            return EventTargets().Add(this, x, y);
        }
    };
}