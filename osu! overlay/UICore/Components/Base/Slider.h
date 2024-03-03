#pragma once

#include "Panel.h"
#include "Dummy.h"

#include "Helper/EventEmitter.h"

namespace zcom
{
    class Slider : public Panel
    {
    public:
        void SetBodyComponent(Component* body);
        void SetBodyComponent(std::unique_ptr<Component> body);
        void SetAnchorComponent(Component* anchor);
        void SetAnchorComponent(std::unique_ptr<Component> anchor);
        Component* GetBodyComponent() { return GetItem(0); }
        Component* GetAnchorComponent() { return GetItem(1); }
        void SetSliderBodyStartOffset(int offset);
        void SetSliderBodyEndOffset(int offset);
        void SetSliderBodyOffset(int start, int end);
        void SetAnchorOffset(int offset);
        void SetInteractionAreaMargins(RECT margins);

        void SetValue(float value);
        float GetValue() const { return _currentValue; }

        EventSubscription<void> SubscribeOnEnterInteractionArea(std::function<void()> handler) { return _onEnterInteractionArea->Subscribe(handler); }
        EventSubscription<void> SubscribeOnLeaveInteractionArea(std::function<void()> handler) { return _onLeaveInteractionArea->Subscribe(handler); }
        EventSubscription<void> SubscribeOnSliderPressed(std::function<void()> handler) { return _onSliderPressed->Subscribe(handler); }
        EventSubscription<void> SubscribeOnSliderReleased(std::function<void()> handler) { return _onSliderReleased->Subscribe(handler); }
        EventSubscription<void, float*> SubscribeOnValueChanged(std::function<void(float*)> handler) { return _onValueChanged->Subscribe(handler); }

    private:
        std::unique_ptr<Dummy> _bodyPlaceholder = nullptr;
        std::unique_ptr<Dummy> _anchorPlaceholder = nullptr;

        int _startOffset = 0;
        int _endOffset = 0;
        int _anchorOffset = 0;
        RECT _interactionAreaMargins = { 0, 0, 0, 0 };

        bool _insideInteractionArea = false;
        bool _holding = false;

        float _currentValue = 0.0f;

        EventEmitter<void> _onEnterInteractionArea;
        EventEmitter<void> _onLeaveInteractionArea;
        EventEmitter<void> _onSliderPressed;
        EventEmitter<void> _onSliderReleased;
        EventEmitter<void, float*> _onValueChanged;

        void _HandleMouseMove(int position);
        void _PositionAnchor();
        void _SetInsideInteractionArea(bool value);

#pragma region base_class
        // Hide public panel methods
    private:
        using Panel::AddItem;
        using Panel::InsertItem;
        using Panel::RemoveItem;
        using Panel::ItemCount;
        using Panel::GetItem;
        using Panel::ClearItems;
        using Panel::FindChildRelativeOffset;

    protected:
        EventTargets _OnMouseMove(int x, int y, int deltaX, int deltaY) override;
        void _OnMouseLeave() override;
        EventTargets _OnLeftPressed(int x, int y) override;
        EventTargets _OnLeftReleased(int x, int y) override;
        void _OnResize(int width, int height) override;

    public:
        const char* GetName() const override { return Name(); }
        static const char* Name() { return "slider"; }
#pragma endregion

    protected:
        friend class Scene;
        friend class Component;
        Slider(Scene* scene) : Panel(scene) {}
        void Init();
    public:
        ~Slider() {}
        Slider(Slider&&) = delete;
        Slider& operator=(Slider&&) = delete;
        Slider(const Slider&) = delete;
        Slider& operator=(const Slider&) = delete;
    };
}