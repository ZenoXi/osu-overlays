#pragma once

#include "ComponentBase.h"

#include "Helper/EventEmitter.h"
#include "Helper/Time.h"
#include "Window/KeyboardEventHandler.h"

namespace zcom
{
    class Toggle : public Component, public KeyboardEventHandler
    {
        DEFINE_COMPONENT(Toggle, Component)
        DEFAULT_DESTRUCTOR(Toggle)
    protected:
        void Init(bool toggledOn);

    public:
        void SetToggledOn(bool toggledOn, bool emitChangeEvent = true);
        bool IsToggledOn() const { return _isToggledOn; }
        void SetToggledOnAnchorColor(D2D1_COLOR_F color);
        void SetToggledOffAnchorColor(D2D1_COLOR_F color);
        D2D1_COLOR_F GetToggledOnAnchorColor() const { return _toggledOnAnchorColor; }
        D2D1_COLOR_F GetToggledOffAnchorColor() const { return _toggledOffAnchorColor; }
        void SetToggledOnBackgroundColor(D2D1_COLOR_F color);
        void SetToggledOffBackgroundColor(D2D1_COLOR_F color);
        D2D1_COLOR_F GetToggledOnBackgroundColor() const { return _toggledOnBackgroundColor; }
        D2D1_COLOR_F GetToggledOffBackgroundColor() const { return _toggledOffBackgroundColor; }
        void SetMarginToBorder(float margin);
        float GetMarginToBorder() const { return _marginToBorder; }
        void SetAnimationDuration(Duration duration) { _animationDuration = duration; }
        Duration GetAnimationDuration() const { return _animationDuration; }

        EventSubscription<void, bool*> SubscribeOnToggled(std::function<void(bool*)> handler) { return _onToggled->Subscribe(handler); }

    private:
        bool _isToggledOn = false;
        D2D1_COLOR_F _toggledOnAnchorColor = D2D1::ColorF(0xC0C0C0);
        D2D1_COLOR_F _toggledOffAnchorColor = D2D1::ColorF(0x808080);
        D2D1_COLOR_F _toggledOnBackgroundColor = GetBackgroundColor();
        D2D1_COLOR_F _toggledOffBackgroundColor = GetBackgroundColor();
        float _marginToBorder = 5.0f;

        Duration _animationDuration = Duration(50, MILLISECONDS);
        TimePoint _animationStartTime = TimePoint(0);
        bool _animating = false;
        float _animationProgress = 0.0f;

        EventEmitter<void, bool*> _onToggled;

    protected:
        void _OnUpdate() override;
        void _OnDraw(Graphics g) override;
        EventTargets _OnLeftPressed(int x, int y) override;
        void _OnSelected(bool reverse) override;
        void _OnDeselected() override;
        bool _OnKeyDown(BYTE vkCode) override;
    };
}