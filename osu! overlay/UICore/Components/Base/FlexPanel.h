#pragma once

#include "Panel.h"

#include <optional>

namespace zcom
{
    class FlexGrow : public Property
    {
    public:
        static std::string _NAME_() { return "flex_grow"; }
        FlexGrow(float ratio = 1.0f) : ratio(ratio) {}
        float ratio;
    };

    class FlexShrink : public Property
    {
    public:
        static std::string _NAME_() { return "flex_shrink"; }
        FlexShrink(float ratio = 1.0f) : ratio(ratio) {}
        float ratio;
    };

    class FlexMaxSize : public Property
    {
    public:
        static std::string _NAME_() { return "flex_max_size"; }
        FlexMaxSize(int value = std::numeric_limits<int>::max()) : value(value) {}
        int value;
    };

    class FlexMinSize : public Property
    {
    public:
        static std::string _NAME_() { return "flex_min_size"; }
        FlexMinSize(int value = 0) : value(value) {}
        int value;
    };

    class FlexAlign : public Property
    {
    public:
        static std::string _NAME_() { return "flex_align"; }
        FlexAlign(Alignment value = Alignment::START) : value(value) {}
        Alignment value;
    };

    class FlexMarginBefore : public Property
    {
    public:
        static std::string _NAME_() { return "flex_margin_before"; }
        FlexMarginBefore(int value = 0) : value(value) {}
        int value;
    };

    class FlexMarginAfter : public Property
    {
    public:
        static std::string _NAME_() { return "flex_margin_after"; }
        FlexMarginAfter(int value = 0) : value(value) {}
        int value;
    };

    class FlexIgnore : public Property
    {
    public:
        static std::string _NAME_() { return "flex_ignore"; }
        FlexIgnore() {}
    };

    enum class FlexDirection
    {
        DOWN,
        UP,
        RIGHT,
        LEFT
    };
    constexpr std::vector<std::pair<int64_t, std::wstring>> FlexDirectionValueProxySelectionValues()
    {
        return {
            { (int64_t)FlexDirection::DOWN, L"Down" },
            { (int64_t)FlexDirection::UP, L"Up" },
            { (int64_t)FlexDirection::RIGHT, L"Right" },
            { (int64_t)FlexDirection::LEFT, L"Left" }
        };
    }

    class FlexPanel : public Panel
    {
        DEFINE_COMPONENT(FlexPanel, Panel)
        DEFAULT_DESTRUCTOR(FlexPanel)
    protected:
        void Init(FlexDirection direction)
        {
            Panel::Init();
            this->direction = direction;
        }

    public:
        Value<int> spacing = Value<int>(0, [=](int& currentValue, const int& spacing) {
            currentValue = spacing;
            _RecalculateLayout();
        });
        Value<FlexDirection> direction = Value<FlexDirection>(FlexDirection::DOWN, [=](FlexDirection& currentValue, const FlexDirection& direction) {
            currentValue = direction;
            _RecalculateLayout();
        });
        Value<std::optional<Alignment>> itemAlignment = Value<std::optional<Alignment>>(std::nullopt, [=](std::optional<Alignment>& currentValue, const std::optional<Alignment>& alignment) {
            currentValue = alignment;
            _RecalculateLayout();
        });
        Value<bool> autoWidth = Value<bool>(false, [=](bool& currentValue, const bool& value) {
            currentValue = value;
            _RecalculateLayout();
        });
        Value<bool> autoHeight = Value<bool>(false, [=](bool& currentValue, const bool& value) {
            currentValue = value;
            _RecalculateLayout();
        });


    protected:
        void _ComputeItemLayout() override;

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;

            values.push_back(ValueProxy::BasicIntValueProxy<int>("spacing", std::make_any<Value<int>*>(&spacing)));
            values.push_back(ValueProxy::BasicEnumValueProxy<FlexDirection>("direction", std::make_any<Value<FlexDirection>*>(&direction), FlexDirectionValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicOptionalEnumValueProxy<Alignment>("item alignment", std::make_any<Value<std::optional<Alignment>>*>(&itemAlignment), AlignmentValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicBoolValueProxy("auto width", std::make_any<Value<bool>*>(&autoWidth)));
            values.push_back(ValueProxy::BasicBoolValueProxy("auto height", std::make_any<Value<bool>*>(&autoHeight)));

            auto data = Panel::GetReflectionData();
            data.insert(data.begin(), { "Flex panel", std::move(values) });
            return data;
        }
    };
}
