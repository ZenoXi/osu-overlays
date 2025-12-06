#pragma once

#include "Overlays/Overlay.h"

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Label.h"
#include "UICore/Helper/Time.h"

#include "OsuDataProvider/DataProviderView.h"

namespace zcom
{
    struct FontSizeInfo
    {
        float fontSize = 0;
        float textWidth = 0;
        float textHeight = 0;
        Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
    };

    class ComboCounterOverlayComponent : public Panel
    {
        DEFINE_COMPONENT(ComboCounterOverlayComponent, Panel)
    public:
        ~ComboCounterOverlayComponent();
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        void _OnUpdate() override;
        void _OnDraw(Graphics* g) override;

        std::shared_ptr<const Overlay> _overlay;

        std::unique_ptr<DataProviderView> _dataProviderView;

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

        Color _barColor = Color(0xFFF0AA);
        Color _textColor = Color(0xAAAAAA);
        Color _gridColor = Color(0x444444);

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configValueChangedEventSubscription = nullptr;
        void _UpdateParameters();
    };
}
