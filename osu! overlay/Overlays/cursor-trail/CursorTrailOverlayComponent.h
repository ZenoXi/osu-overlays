#pragma once

#include "Overlays/Overlay.h"
#include "CursorTrailConfig.h"

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Label.h"
#include "UICore/Helper/Time.h"

#include "OsuDataProvider/DataProviderView.h"
#include "Shared/Util/Navigation.h"

namespace zcom
{
    class CursorTrailOverlayComponent : public Panel
    {
        DEFINE_COMPONENT(CursorTrailOverlayComponent, Panel)
        DEFAULT_DESTRUCTOR(CursorTrailOverlayComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        void _OnUpdate() override;
        void _OnDraw(Graphics* g) override;

        std::shared_ptr<const Overlay> _overlay;

        std::unique_ptr<DataProviderView> _dataProviderView; struct Point
        {
            Pos2D<float> position{};
            TimePoint time{};
            Pos2D<float> bezierAnchorPosition{};
        };

        struct GradientStop
        {
            Color color;
            float position;
        };

        //POINT _currentMousePos = { 0, 0 };
        //std::mutex _m_input;
        //std::vector<Point> _pendingInput;
        //Clock _inputClock;
        //std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _inputMessageSubscription = nullptr;

        std::vector<Point> _points;
        Pos2D<float> _previousMove = { 0.0f, 0.0f };
        Color _headColor = Color::ARGB(CursorTrailConfig::HEAD_COLOR.defaultValue);
        int _trailWidth = CursorTrailConfig::TRAIL_WIDTH.defaultValue;
        int _headSize = CursorTrailConfig::HEAD_SIZE.defaultValue;
        float _trailEdgeWidth = CursorTrailConfig::TRAIL_EDGE_WIDTH.defaultValue;
        float _headEdgeWidth = CursorTrailConfig::HEAD_EDGE_WIDTH.defaultValue;
        Duration _trailDuration = Duration(CursorTrailConfig::TRAIL_LIFETIME.defaultValue, MILLISECONDS);
        Duration _colorCycleDuration = Duration(CursorTrailConfig::COLOR_CYCLE_DURATION.defaultValue, MILLISECONDS);
        Duration _iconSpinDuration = Duration(CursorTrailConfig::ICON_SPIN_DURATION.defaultValue, MILLISECONDS);
        int _trailResolution = CursorTrailConfig::TRAIL_RESOLUTION.defaultValue;
        std::vector<GradientStop> _palette;

        std::optional<Bitmap> _headIconBitmap = std::nullopt;

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configValueChangedEventSubscription = nullptr;
        void _UpdateParameters();
        void _ParsePaletteString(const std::wstring& str);

        D2D1_COLOR_F _GetColorByTime(TimePoint time);
    };
}
