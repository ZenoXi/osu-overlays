#pragma once

#include "ComponentBase.h"
#include "../../Helper/ConfigValue.h"
#include "../../Helper/WindowView.h"

namespace zcom
{
    class ColorSelector : public Component
    {
        DEFINE_COMPONENT(ColorSelector, Component)
    public:
        ~ColorSelector();
    protected:
        void Init();

    public:
        void UseConfigValue(ConfigValue<int> configValue);
        void ResetConfigValue() { _configValue = std::nullopt; }
        Value<Color> color = Value<Color>(Color(0x808080), [=](Color& currentValue, const Color& newValue) {
            currentValue = newValue;
            if (!_settingInternally)
                _colorSelectorValueChangedEventEmitter->InvokeAll(newValue);
            InvokeRedraw();
        });
        Value<std::wstring> colorSelectorPopupName = std::wstring(L"Color selector");

        [[nodiscard]]
        EventSubscription<void, Color> SubscribeOnColorChanged(const std::function<void(Color)>& handler)
        {
            return _colorChangedEventEmitter->Subscribe(handler);
        }

    private:
        std::optional<ConfigValue<int>> _configValue;
        std::wstring _windowClassName;
        std::optional<WindowView> _windowView;

        bool _settingInternally = false;

        EventEmitter<void, Color> _colorChangedEventEmitter;
        EventEmitter<void, Color> _colorSelectorValueChangedEventEmitter = EventEmitter<void, Color>(EventEmitterThreadMode::MULTITHREADED);
        std::unique_ptr<AsyncEventSubscription<void, Color>> _colorSelectorSceneValueChangedSubscription;

        void _OpenColorSelector();
        ID2D1ImageBrush* _CreateCheckeredPatternBrush(Graphics* g, D2D1_COLOR_F cellColor);

    protected:
        void _OnDraw(Graphics* g) override;
        EventContext _OnLeftReleased(std::optional<Point> point) override;

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicColorValueProxy("color", std::make_any<Value<Color>*>(&color)));
            values.push_back(ValueProxy::BasicTextValueProxy("popup name", std::make_any<Value<std::wstring>*>(&colorSelectorPopupName)));

            auto data = Component::GetReflectionData();
            data.insert(data.begin(), { "Color selector", std::move(values) });
            return data;
        }
    };
}