#pragma once

#include "Scenes/Scene.h"

#include "Components/Base/Label.h"

#include "OsuDataProvider/DataProvider.h"
#include "OsuDataProvider/GameState.h"

#include <vector>
#include <array>
#include <unordered_map>
#include <wrl.h>

namespace zcom
{
    struct ComboBarSceneOptions : public SceneOptionsBase
    {

    };

    struct FontSizeInfo
    {
        float fontSize = 0;
        float textWidth = 0;
        float textHeight = 0;
        Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
    };

    class ComboBarScene : public Scene
    {
        DEFINE_SCENE(ComboBarScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
        void Uninit() override;

    private:
        std::unique_ptr<osu::DataProvider> _dataProvider;
        bool _interactionModeActive = false;

        std::vector<int> _maxCombos;
        int _comboAtPreviousCheck = 0;
        bool _inGameState = false;
        int _scoreAtPreviousCheck = 0;

        IDWriteFactory* _dwriteFactory = nullptr;
        IDWriteFontCollection* _dwriteFontCollection = nullptr;
        IDWriteTextFormat* _dwriteGridTextFormat = nullptr;

        std::wstring _fontName;
        // Maps number of digits to the font size information, necessary to fit the text
        std::array<FontSizeInfo, 10> _fontSizeMap;
        void _BuildFontSizeMap(const float maxFontSize, const float layoutWidth);

        D2D1_COLOR_F _barColor = D2D1::ColorF(0xFFF0AA);
        D2D1_COLOR_F _textColor = D2D1::ColorF(0xAAAAAA);
        D2D1_COLOR_F _gridColor = D2D1::ColorF(0x444444);

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configValueChangedEventSubscription = nullptr;
        void _UpdateParameters();

        void _Update();
        void _Draw(Component* panel, Graphics g);
    };
}