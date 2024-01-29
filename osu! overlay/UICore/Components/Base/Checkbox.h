#pragma once

#include "ComponentBase.h"

#include "Helper/EventEmitter.h"
#include "Window/KeyboardEventHandler.h"

namespace zcom
{
    class Checkbox : public Component, public KeyboardEventHandler
    {
    public:
        bool Checked() const { return _checked; }
        void Checked(bool checked);
        void SetCheckColor(D2D1_COLOR_F checkColor);
        EventSubscription<void, bool> SubscribeOnStateChanged(const std::function<void(bool)>& handler);

    protected:
        friend class Scene;
        friend class Component;
        Checkbox(Scene* scene) : Component(scene) {}
        void Init(bool checked = false);
    public:
        ~Checkbox() {}
        Checkbox(Checkbox&&) = delete;
        Checkbox& operator=(Checkbox&&) = delete;
        Checkbox(const Checkbox&) = delete;
        Checkbox& operator=(const Checkbox&) = delete;

    private:
        EventEmitter<void, bool> _onStateChanged;
        D2D1_COLOR_F _checkColor = {};
        bool _checked = false;

#pragma region base_class
    protected:
        void _OnDraw(Graphics g) override;
        EventTargets _OnLeftPressed(int x, int y) override;
        void _OnSelected(bool reverse) override;
        void _OnDeselected() override;
        bool _OnHotkey(int id) override { return false; }
        bool _OnKeyDown(BYTE vkCode) override;
        bool _OnKeyUp(BYTE vkCode) override { return false; }
        bool _OnChar(wchar_t ch) override { return false; }

    public:
        const char* GetName() const override { return Name(); }
        static const char* Name() { return "checkbox"; }
#pragma endregion
    };
}