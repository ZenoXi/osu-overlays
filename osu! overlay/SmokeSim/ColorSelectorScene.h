#pragma once

#include "Scenes/Scene.h"

#include "Components/Base/Slider.h"
#include "Shared/Util/Color.h"
#include "Helper/Time.h"

namespace zcom
{
    struct ColorSelectorSceneOptions : public SceneOptionsBase
    {
        std::wstring optionName;
    };

    class ColorSelectorScene : public Scene
    {
        std::unique_ptr<Slider> _redSlider = nullptr;
        std::unique_ptr<Slider> _greenSlider = nullptr;
        std::unique_ptr<Slider> _blueSlider = nullptr;
        std::unique_ptr<Slider> _opacitySlider = nullptr;

        zutil::Color _initialColor;
        zutil::Color _currentColor;

        TimePoint _lastColorChange = TimePoint(0);
        std::wstring _optionName = L"";
        bool _optionsSaved = true;

        void _OnColorChanged();
        void _SaveOptions();
        void _DrawGradient(Component* item, D2D1_COLOR_F startColor, D2D1_COLOR_F endColor, Graphics g);
        void _DrawCheckeredPattern(Component* item, D2D1_COLOR_F color1, D2D1_COLOR_F color2, Graphics g);

    public:
        ColorSelectorScene(App* app, zwnd::Window* window);

        const char* GetName() const { return "ColorSelectorScene"; }
        static const char* StaticName() { return "ColorSelectorScene"; }

    private:
        void _Init(SceneOptionsBase* options);
        void _Uninit();
        void _Focus();
        void _Unfocus();
        void _Update();
        void _Resize(int width, int height, ResizeInfo info);
    };
}