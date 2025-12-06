#pragma once

#include "Scenes/Scene.h"

#include "Components/Custom/ColorSliderInputGroup.h"
#include "UICore/Model/Color.h"
#include "Helper/Time.h"

#include <optional>

namespace zcom
{
    struct ColorSelectorSceneOptions : public SceneOptionsBase
    {
        std::optional<ConfigValue<int>> configValue = std::nullopt;
        std::optional<Color> initialColor = std::nullopt;
        std::optional<EventEmitter<void, Color>> colorSelectorValueChangedEventEmitter;
        std::optional<EventEmitter<void, Color>> colorSelectorSceneValueChangedEventEmitter;
    };

    class ColorSelectorScene : public Scene
    {
        DEFINE_SCENE(ColorSelectorScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
        void Uninit() override;

    private:
        std::unique_ptr<ColorSliderInputGroup> _redInput = nullptr;
        std::unique_ptr<ColorSliderInputGroup> _greenInput = nullptr;
        std::unique_ptr<ColorSliderInputGroup> _blueInput = nullptr;
        std::unique_ptr<ColorSliderInputGroup> _opacityInput = nullptr;

        Color _initialColor;
        Color _currentColor;

        TimePoint _lastColorChange = TimePoint(0);
        std::optional<ConfigValue<int>> _configValue = std::nullopt;
        bool _optionsSaved = true;

        std::optional<EventEmitter<void, Color>> _colorSelectorSceneValueChangedEventEmitter;
        std::unique_ptr<AsyncEventSubscription<void, Color>> _colorSelectorValueChangedSubscription;

        void _OnColorChanged();
        void _DrawGradient(Component* item, Color startColor, Color endColor, Graphics* g);
        void _DrawCheckeredPattern(Component* item, Color color1, Color color2, Graphics* g);

        void _Update();
    };
}