#pragma once

#include "ComponentBase.h"
#include "Window/KeyboardEventHandler.h"

#include <unordered_map>

namespace zcom
{

    // Completely barebones component, only contains base component functionality
    class KeySelector : public Component, public KeyboardEventHandler
    {
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

#pragma region base_class
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

    public:
        const char* GetName() const override { return Name(); }
        static const char* Name() { return "key_selector"; }
#pragma endregion

    private:
        EventEmitter<void, BYTE> _onKeySelected;
        BYTE _currentKey = 0;

    protected:
        friend class Scene;
        friend class Component;
        KeySelector(Scene* scene) : Component(scene) {}
        void Init(BYTE initialValue = 0)
        {
            _currentKey = initialValue;
            SetSelectable(true);
            SetBorderVisibility(true);
            SetBorderColor(D2D1::ColorF(0.3f, 0.3f, 0.3f));
        }
    public:
        ~KeySelector() {}
        KeySelector(KeySelector&&) = delete;
        KeySelector& operator=(KeySelector&&) = delete;
        KeySelector(const KeySelector&) = delete;
        KeySelector& operator=(const KeySelector&) = delete;
    };
}