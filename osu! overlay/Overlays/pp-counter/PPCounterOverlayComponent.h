#pragma once

#include "Overlays/Overlay.h"

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Label.h"
#include "UICore/Helper/Time.h"

#include "OsuDataProvider/DataProviderView.h"

namespace zcom
{
    class PPCounterOverlayComponent : public Panel
    {
        DEFINE_COMPONENT(PPCounterOverlayComponent, Panel)
        DEFAULT_DESTRUCTOR(PPCounterOverlayComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        void _OnUpdate() override;

        std::shared_ptr<const Overlay> _overlay;

        std::unique_ptr<FlexPanel> _dataPanel;
        std::unique_ptr<Label> _ppLabel;
        std::unique_ptr<Label> _100CountLabel;
        std::unique_ptr<Label> _50CountLabel;
        std::unique_ptr<Label> _missCountLabel;

        Value<Color> _backgroundColor;
        Value<Color> _backgroundAccentColor;
        bool _showDifficultyGraph;
        Color _difficultyGraphForegroundColor;
        Color _difficultyGraphBackgroundColor;

        std::unique_ptr<DataProviderView> _dataProviderView;

        int _startPPValue = 0;
        int _targetPPValue = 0;
        int _currentPPValue = 0;
        TimePoint _ppChangeStart = TimePoint();
        Duration _ppChangeDuration = Duration(500, MILLISECONDS);
        bool _animatingPP = false;

        float _mapRate = 1.0f;
        std::vector<float> _strains;
        float _highestStrain = 0.0f;
        std::vector<int64_t> _xAxis;
        int64_t _currentGameTime = 0;
        int64_t _lastObjectTime = 0;
        TimePoint _lastGraphUpdate = TimePoint();
        Duration _graphUpdateInterval = Duration(100, MILLISECONDS);

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configValueChangedEventSubscription = nullptr;

        void _UpdateAppearanceFromConfig();
        void _DrawStrainGraph(Component* item, Graphics* g);
    };
}