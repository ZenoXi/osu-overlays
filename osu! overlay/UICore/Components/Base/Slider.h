#pragma once

#include "Panel.h"
#include "Dummy.h"

#include "Helper/EventEmitter.h"

namespace zcom
{
    class Slider : public Panel
    {
        DEFINE_COMPONENT(Slider, Panel)
        DEFAULT_DESTRUCTOR(Slider)
    protected:
        void Init();
    
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

        void SetValue(float value, bool emitChangeEvent = true);
        float GetValue() const { return _currentValue; }

        EventSubscription<void, Slider*> SubscribeOnEnterInteractionArea(std::function<void(Slider*)> handler) { return _onEnterInteractionArea->Subscribe(handler); }
        EventSubscription<void, Slider*> SubscribeOnLeaveInteractionArea(std::function<void(Slider*)> handler) { return _onLeaveInteractionArea->Subscribe(handler); }
        EventSubscription<void, Slider*> SubscribeOnSliderPressed(std::function<void(Slider*)> handler) { return _onSliderPressed->Subscribe(handler); }
        EventSubscription<void, Slider*> SubscribeOnSliderReleased(std::function<void(Slider*)> handler) { return _onSliderReleased->Subscribe(handler); }
        EventSubscription<void, Slider*, float*> SubscribeOnValueChanged(std::function<void(Slider*, float*)> handler) { return _onValueChanged->Subscribe(handler); }

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

        EventEmitter<void, Slider*> _onEnterInteractionArea;
        EventEmitter<void, Slider*> _onLeaveInteractionArea;
        EventEmitter<void, Slider*> _onSliderPressed;
        EventEmitter<void, Slider*> _onSliderReleased;
        EventEmitter<void, Slider*, float*> _onValueChanged;

        void _HandleMouseMove(int position);
        void _PositionAnchor();
        void _SetInsideInteractionArea(bool value);

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
        EventTargets _OnWheelUp(int x, int y) override;
        EventTargets _OnWheelDown(int x, int y) override;
        void _OnResize(int width, int height) override;
    };
}