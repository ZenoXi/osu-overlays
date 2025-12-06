#pragma once

#include "ComponentBase.h"

#include "Helper/EventEmitter.h"
#include "Window/KeyboardEventHandler.h"

namespace zcom
{
    class Checkbox : public Component, public KeyboardEventHandler
    {
        DEFINE_COMPONENT(Checkbox, Component)
        DEFAULT_DESTRUCTOR(Checkbox)
    protected:
        void Init(bool checked = false);

    public:
        Value<bool> checked = Value<bool>(false, [=](bool& currentValue, const bool& checked) {
            currentValue = checked;
            InvokeRedraw();
        });
        Value<Color> checkColor = Value<Color>(Color(0x999999), [=](Color& currentValue, const Color& newValue) {
            currentValue = newValue;
            if (checked)
                InvokeRedraw();
        });
        EventSubscription<void, bool> SubscribeOnStateChanged(const std::function<void(bool)>& handler);

    private:
        EventEmitter<void, bool> _onStateChanged;

    protected:
        void _OnDraw(Graphics* g) override;
        EventContext _OnLeftPressed(Point point) override;
        void _OnSelected(bool reverse) override;
        void _OnDeselected() override;
        bool _OnHotkey(int id) override { return false; }
        bool _OnKeyDown(BYTE vkCode) override;
        bool _OnKeyUp(BYTE vkCode) override { return false; }
        bool _OnChar(wchar_t ch) override { return false; }

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicBoolValueProxy("checked", std::make_any<Value<bool>*>(&checked)));
            values.push_back(ValueProxy::BasicColorValueProxy("check color", std::make_any<Value<Color>*>(&checkColor)));

            auto data = Component::GetReflectionData();
            data.insert(data.begin(), { "Checkbox", std::move(values) });
            return data;
        }
    };
}