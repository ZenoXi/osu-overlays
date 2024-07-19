#pragma once

#include "Scenes/Scene.h"
#include "Shared/Util/Navigation.h"
#include "Shared/Util/Color.h"
#include "Helper/Time.h"
#include "CursorTrailConfig.h"

#include <vector>

namespace zcom
{
    struct CursorTrailSceneOptions : public SceneOptionsBase
    {

    };

    class CursorTrailScene : public Scene
    {
        DEFINE_SCENE(CursorTrailScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;

    private:
        struct Point
        {
            Pos2D<float> position{};
            TimePoint time{};
            Pos2D<float> bezierAnchorPosition{};
        };

        struct GradientStop
        {
            zutil::Color color;
            float position;
        };

        //POINT _currentMousePos = { 0, 0 };
        //std::mutex _m_input;
        //std::vector<Point> _pendingInput;
        //Clock _inputClock;
        //std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _inputMessageSubscription = nullptr;

        std::vector<Point> _points;
        Pos2D<float> _previousMove = { 0.0f, 0.0f };
        zutil::Color _headColor = zutil::Color(CursorTrailConfig::HEAD_COLOR.defaultValue);
        int _trailWidth = CursorTrailConfig::TRAIL_WIDTH.defaultValue;
        int _headSize = CursorTrailConfig::HEAD_SIZE.defaultValue;
        float _trailEdgeWidth = CursorTrailConfig::TRAIL_EDGE_WIDTH.defaultValue;
        float _headEdgeWidth = CursorTrailConfig::HEAD_EDGE_WIDTH.defaultValue;
        Duration _trailDuration = Duration(CursorTrailConfig::TRAIL_LIFETIME.defaultValue, MILLISECONDS);
        Duration _colorCycleDuration = Duration(CursorTrailConfig::COLOR_CYCLE_DURATION.defaultValue, MILLISECONDS);
        Duration _iconSpinDuration = Duration(CursorTrailConfig::ICON_SPIN_DURATION.defaultValue, MILLISECONDS);
        int _trailResolution = CursorTrailConfig::TRAIL_RESOLUTION.defaultValue;
        std::vector<GradientStop> _palette;

        ID2D1Bitmap* _headIconBitmap = nullptr;

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configValueChangedEventSubscription = nullptr;
        void _UpdateParameters();
        void _ParsePaletteString(const std::wstring& str);

        void _Update();
        void _Draw(Component* panel, Graphics g);
        D2D1_COLOR_F _GetColorByTime(TimePoint time);
    };
}