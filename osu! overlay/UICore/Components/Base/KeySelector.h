#pragma once

#include "ComponentBase.h"
#include "Window/KeyboardEventHandler.h"

#include <unordered_map>

namespace zcom
{

    // Completely barebones component, only contains base component functionality
    class KeySelector : public Component, public KeyboardEventHandler
    {
        DEFINE_COMPONENT(KeySelector, Component)
        DEFAULT_DESTRUCTOR(KeySelector)
    protected:
        void Init(BYTE initialValue = 0)
        {
            currentKey = initialValue;
            selectable = true;
            border.visible = true;
            border.color = Color(0x4D4D4D);
        }

    public:
        static const std::unordered_map<BYTE, std::wstring> KeyCodeNameMap;

        Value<BYTE> currentKey = 0;

        EventSubscription<void, BYTE> SubscribeOnKeySelected(const std::function<void(BYTE)>& func)
        {
            return _onKeySelected->Subscribe(func);
        }

    private:
        EventEmitter<void, BYTE> _onKeySelected;

    protected:
        bool _OnKeyDown(BYTE vkCode) override
        {
            currentKey = vkCode;
            _onKeySelected->InvokeAll(vkCode);
            OnDeselected();
            return true;
        }

        void _OnSelected(bool reverse) override;

        void _OnDeselected() override;

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicIntValueProxy<BYTE>("current key", std::make_any<Value<BYTE>*>(&currentKey), ValueProxy::Number(0), ValueProxy::Number(255)));

            auto data = Component::GetReflectionData();
            data.insert(data.begin(), { "Key selector", std::move(values) });
            return data;
        }
    };
}