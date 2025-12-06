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
        HIDE_PANEL_METHODS
    protected:
        void Init();
    
    public:
        void SetBodyComponent(Component* body);
        void SetBodyComponent(std::unique_ptr<Component> body);
        void SetAnchorComponent(Component* anchor);
        void SetAnchorComponent(std::unique_ptr<Component> anchor);
        Component* GetBodyComponent() { return GetItem(0); }
        Component* GetAnchorComponent() { return GetItem(1); }

        Value<int> bodyStartOffset = Value<int>(0, [=](int& currentValue, const int& offset) {
            currentValue = offset;
            _PositionAnchor();
        });
        Value<int> bodyEndOffset = Value<int>(0, [=](int& currentValue, const int& offset) {
            currentValue = offset;
            _PositionAnchor();
        });
        Value<int> anchorOffset = Value<int>(0, [=](int& currentValue, const int& offset) {
            currentValue = offset;
            _PositionAnchor();
        });
        Value<Rect> interactionAreaMargins = Rect{ 0, 0, 0, 0 };

        Value<float> value = Value<float>(0.0f, [=](float& currentValue, const float& value) {
            if (value < 0.0f)
                currentValue = 0.0f;
            else if (value > 1.0f)
                currentValue = 1.0f;
            else
                currentValue = value;
            _PositionAnchor();
        });

        Value<bool> insideInteractionArea_ = false;
        Value<bool> holding_ = false;

        EventSubscription<void, Slider*, float*> SubscribeOnValueChanged(std::function<void(Slider*, float*)> handler) { return _onValueChanged->Subscribe(handler); }

    private:
        std::unique_ptr<Dummy> _bodyPlaceholder = nullptr;
        std::unique_ptr<Dummy> _anchorPlaceholder = nullptr;

        EventEmitter<void, Slider*, float*> _onValueChanged;

        void _HandleMouseMove(int position);
        void _PositionAnchor();

    protected:
        EventContext _OnMouseMove(Point point, Point deltaPos) override;
        void _OnMouseLeave() override;
        EventContext _OnLeftPressed(Point point) override;
        EventContext _OnLeftReleased(std::optional<Point> point) override;
        EventContext _OnWheelUp(Point point) override;
        EventContext _OnWheelDown(Point point) override;
        void _OnResize(Size size) override;

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("value", std::make_any<Value<float>*>(&value), 3, ValueProxy::Number(0), ValueProxy::Number(1), ValueProxy::Number("0.05")));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("body start offset", std::make_any<Value<int>*>(&bodyStartOffset)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("body end offset", std::make_any<Value<int>*>(&bodyEndOffset)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("anchor offset", std::make_any<Value<int>*>(&anchorOffset)));
            values.push_back(Rect::LeftValueProxy("interaction area left margin", std::make_any<Value<Rect>*>(&interactionAreaMargins)));
            values.push_back(Rect::TopValueProxy("interaction area top margin", std::make_any<Value<Rect>*>(&interactionAreaMargins)));
            values.push_back(Rect::RightValueProxy("interaction area right margin", std::make_any<Value<Rect>*>(&interactionAreaMargins)));
            values.push_back(Rect::BottomValueProxy("interaction area bottom margin", std::make_any<Value<Rect>*>(&interactionAreaMargins)));
            values.push_back(ValueProxy::BasicBoolValueProxy("interaction area hovered", std::make_any<Value<bool>*>(&insideInteractionArea_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("holding", std::make_any<Value<bool>*>(&holding_)).Computed());

            auto data = Panel::GetReflectionData();
            data.insert(data.begin(), { "Slider", std::move(values) });
            return data;
        }
    };
}