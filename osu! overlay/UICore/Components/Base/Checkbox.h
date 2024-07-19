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
        bool Checked() const { return _checked; }
        void Checked(bool checked, bool emitChangeEvent = true);
        void SetCheckColor(D2D1_COLOR_F checkColor);
        EventSubscription<void, bool> SubscribeOnStateChanged(const std::function<void(bool)>& handler);

    private:
        EventEmitter<void, bool> _onStateChanged;
        D2D1_COLOR_F _checkColor = {};
        bool _checked = false;

    protected:
        void _OnDraw(Graphics g) override;
        EventTargets _OnLeftPressed(int x, int y) override;
        void _OnSelected(bool reverse) override;
        void _OnDeselected() override;
        bool _OnHotkey(int id) override { return false; }
        bool _OnKeyDown(BYTE vkCode) override;
        bool _OnKeyUp(BYTE vkCode) override { return false; }
        bool _OnChar(wchar_t ch) override { return false; }
    };
}