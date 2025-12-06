#pragma once

#include "App.h"
#include "Button.h"
#include "TextInput.h"
#include "Window/KeyboardEventHandler.h"

#include "Helper/ResourceManager.h"
#include "Helper/Time.h"
#include "Helper/decimal.h"

#include <iostream>
#include <limits>

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
        Value<NumberInputValue> value = Value<NumberInputValue>([=](NumberInputValue& currentValue, const NumberInputValue& value) {
            currentValue = value;
            _BoundValue();
            _UpdateText();
        });
        Value<int> precision = Value<int>(0, [=](int& currentValue, const int& precision) {

            if (precision < 0)
                currentValue = 0;
            else if (precision > MAX_DEC_PRECISION)
                currentValue = MAX_DEC_PRECISION;
            else
                currentValue = precision;

            _UpdateText();
        });
        Value<NumberInputValue> stepSize = NumberInputValue(1);
        Value<NumberInputValue> minValue = Value<NumberInputValue>(NumberInputValue(std::numeric_limits<int32_t>::min()), [=](NumberInputValue& currentValue, const NumberInputValue& value) {
            currentValue = value;
            if (value > maxValue)
                maxValue = value;
            _BoundValue();
        });
        Value<NumberInputValue> maxValue = Value<NumberInputValue>(NumberInputValue(std::numeric_limits<int32_t>::max()), [=](NumberInputValue& currentValue, const NumberInputValue& value) {
            currentValue = value;
            if (value < minValue)
                minValue = value;
            _BoundValue();
        });
        Value<float> arrowImageGap = 1.0f;

        void StepUp()
        {
            value = value.Get() + stepSize.Get();
        }

        void StepDown()
        {
            value = value.Get() - stepSize.Get();
        }

        [[nodiscard]]
        EventSubscription<void, NumberInputValue> SubscribeOnValueChanged(std::function<void(NumberInputValue)> handler)
        {
            return _valueChangedEvent->Subscribe(handler);
        }

    private:
        bool _internalChange = false;

        EventEmitter<void, NumberInputValue> _valueChangedEvent;


        void _UpdateValue()
        {
            value = NumberInputValue(wstring_to_string(text));
        }

        void _UpdateText()
        {
            std::ostringstream ss;
            ss << value.Get();
            std::string s = ss.str();
            std::wstring str = std::wstring(s.begin(), s.end());

            // Cut off unnecessary decimal points
            if (precision == 0)
                str = str.substr(0, str.length() - (MAX_DEC_PRECISION + 1));
            else
                str = str.substr(0, str.length() - MAX_DEC_PRECISION + precision);

            _internalChange = true;
            text = str;
            _internalChange = false;
        }

        void _BoundValue()
        {
            if (value.Get() < minValue.Get())
                value = minValue.Get();
            else if (value.Get() > maxValue.Get())
                value = maxValue.Get();
        }

    protected:
        void _OnDeselected() override
        {
            TextInput::_OnDeselected();
            _UpdateValue();
        }

        EventContext _OnWheelUp(Point point) override
        {
            StepUp();
            _valueChangedEvent->InvokeAll(value);
            return EventContext().Add(this, point);
        }

        EventContext _OnWheelDown(Point point) override
        {
            StepDown();
            _valueChangedEvent->InvokeAll(value);
            return EventContext().Add(this, point);
        }

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicNumberValueProxy("value", std::make_any<Value<ValueProxy::Number>*>(&value), 3));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("precision", std::make_any<Value<int>*>(&precision), ValueProxy::Number(0), ValueProxy::Number(MAX_DEC_PRECISION)));
            values.push_back(ValueProxy::BasicNumberValueProxy("step size", std::make_any<Value<ValueProxy::Number>*>(&stepSize), MAX_DEC_PRECISION));
            values.push_back(ValueProxy::BasicNumberValueProxy("min value", std::make_any<Value<ValueProxy::Number>*>(&minValue), MAX_DEC_PRECISION));
            values.push_back(ValueProxy::BasicNumberValueProxy("max value", std::make_any<Value<ValueProxy::Number>*>(&maxValue), MAX_DEC_PRECISION));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("arrow image gap", std::make_any<Value<float>*>(&arrowImageGap), 3));

            auto data = TextInput::GetReflectionData();
            data.insert(data.begin(), { "Number input", std::move(values) });
            return data;
        }
    };
}