#pragma once

#include "Scenes/Scene.h"

#include "Components/Base/Slider.h"
#include "Shared/Util/Color.h"
#include "Helper/Time.h"

namespace zcom
{
    struct ColorSelectorSceneOptions : public SceneOptionsBase
    {
        ConfigValue<int> configValue = ConfigValue<int>(L"", 0xFFFFFFFF);
    };

    class ColorSelectorScene : public Scene
    {
        DEFINE_SCENE(ColorSelectorScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
        void Uninit() override;

    private:
        std::unique_ptr<Slider> _redSlider = nullptr;
        std::unique_ptr<Slider> _greenSlider = nullptr;
        std::unique_ptr<Slider> _blueSlider = nullptr;
        std::unique_ptr<Slider> _opacitySlider = nullptr;

        zutil::Color _initialColor;
        zutil::Color _currentColor;

        TimePoint _lastColorChange = TimePoint(0);
        ConfigValue<int> _configValue = ConfigValue<int>(L"", 0xFFFFFFFF);
        bool _optionsSaved = true;

        void _OnColorChanged();
        void _DrawGradient(Component* item, D2D1_COLOR_F startColor, D2D1_COLOR_F endColor, Graphics g);
        void _DrawCheckeredPattern(Component* item, D2D1_COLOR_F color1, D2D1_COLOR_F color2, Graphics g);

        void _Update();
    };
}