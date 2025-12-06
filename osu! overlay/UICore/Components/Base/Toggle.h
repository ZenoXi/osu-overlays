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
        Value<bool> toggledOn = Value<bool>(false, [=](bool& currentValue, const bool& toggledOn) {
            currentValue = toggledOn;
            animating_ = true;
            animationStartTime_ = ztime::Main();
            InvokeRedraw();
        });
        Value<Color> toggledOnAnchorColor = Value<Color>(Color(0xC0C0C0), [=](Color& currentValue, const Color& color) {
            currentValue = color;
            if (toggledOn)
                InvokeRedraw();
        });
        Value<Color> toggledOffAnchorColor = Value<Color>(Color(0x808080), [=](Color& currentValue, const Color& color) {
            currentValue = color;
            if (!toggledOn)
                InvokeRedraw();
        });
        Value<Color> toggledOnBackgroundColor = Value<Color>(backgroundColor.Get(), [=](Color& currentValue, const Color& color) {
            currentValue = color;
            if (toggledOn)
                backgroundColor = color;
        });
        Value<Color> toggledOffBackgroundColor = Value<Color>(backgroundColor.Get(), [=](Color& currentValue, const Color& color) {
            currentValue = color;
            if (!toggledOn)
                backgroundColor = color;
        });
        Value<float> marginToBorder = Value<float>(5.0f, [=](float& currentValue, const float& margin) {
            currentValue = margin;
            InvokeRedraw();
        });
        Value<Duration> animationDuration = Duration(50, MILLISECONDS);

        Value<bool> animating_ = false;
        Value<TimePoint> animationStartTime_ = TimePoint(0);
        Value<float> animationProgress_ = 0.0f;

        EventSubscription<void, bool*> SubscribeOnToggled(std::function<void(bool*)> handler) { return _onToggled->Subscribe(handler); }

    private:
        EventEmitter<void, bool*> _onToggled;

    protected:
        void _OnUpdate() override;
        void _OnDraw(Graphics* g) override;
        EventContext _OnLeftPressed(Point point) override;
        void _OnSelected(bool reverse) override;
        void _OnDeselected() override;
        bool _OnKeyDown(BYTE vkCode) override;

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicBoolValueProxy("toggled on", std::make_any<Value<bool>*>(&toggledOn)));
            values.push_back(ValueProxy::BasicColorValueProxy("toggled on anchor color", std::make_any<Value<Color>*>(&toggledOnAnchorColor)));
            values.push_back(ValueProxy::BasicColorValueProxy("toggled off anchor color", std::make_any<Value<Color>*>(&toggledOffAnchorColor)));
            values.push_back(ValueProxy::BasicColorValueProxy("toggled on background color", std::make_any<Value<Color>*>(&toggledOnBackgroundColor)));
            values.push_back(ValueProxy::BasicColorValueProxy("toggled off background color", std::make_any<Value<Color>*>(&toggledOffBackgroundColor)));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("margin to border", std::make_any<Value<float>*>(&marginToBorder), 3));
            values.push_back(DurationValueProxy("animation duration (ms)", std::make_any<Value<Duration>*>(&animationDuration), MILLISECONDS));
            values.push_back(ValueProxy::BasicBoolValueProxy("animating", std::make_any<Value<bool>*>(&animating_)).Computed());
            values.push_back(TimePointValueProxy("animation start time (ms)", std::make_any<Value<TimePoint>*>(&animationStartTime_), MILLISECONDS).Computed());
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("animation progress", std::make_any<Value<float>*>(&animationProgress_), 3));

            auto data = Component::GetReflectionData();
            data.insert(data.begin(), { "Toggle", std::move(values) });
            return data;
        }
    };
}