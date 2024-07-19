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
            _currentKey = initialValue;
            SetSelectable(true);
            SetBorderVisibility(true);
            SetBorderColor(D2D1::ColorF(0.3f, 0.3f, 0.3f));
        }

    public:
        static const std::unordered_map<BYTE, std::wstring> KeyCodeNameMap;

        BYTE GetCurrentKey() const
        {
            return _currentKey;
        }

        void SetCurrentKey(BYTE key)
        {
            _currentKey = key;
        }

        EventSubscription<void, BYTE> SubscribeOnKeySelected(const std::function<void(BYTE)>& func)
        {
            return _onKeySelected->Subscribe(func);
        }

    private:
        EventEmitter<void, BYTE> _onKeySelected;
        BYTE _currentKey = 0;

    protected:
        bool _OnKeyDown(BYTE vkCode) override
        {
            _currentKey = vkCode;
            _onKeySelected->InvokeAll(vkCode);
            OnDeselected();
            return true;
        }

        void _OnSelected(bool reverse) override;

        void _OnDeselected() override;
    };
}