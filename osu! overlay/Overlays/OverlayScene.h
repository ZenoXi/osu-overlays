#pragma once

#include "Overlay.h"
#include "OverlayEvent.h"
#include "UICore/Scenes/Scene.h"
#include "notification/NotificationDisplayComponent.h"

#include <atomic>

namespace zcom
{
    struct OverlaySceneOptions : public SceneOptionsBase
    {
        EventEmitter<void, OverlayEvent> overlayEventEmitter;
    };

    class OverlayScene : public Scene
    {
        DEFINE_SCENE(OverlayScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;

    private:
        std::unique_ptr<AsyncEventSubscription<void, OverlayEvent>> _overlayEventSubscription;
        struct EnabledOverlay
        {
            std::shared_ptr<const Overlay> overlay;
            std::unique_ptr<Component> overlayComponent;
        };
        std::vector<EnabledOverlay> _enabledOverlays;

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configValueChangedEventSubscription = nullptr;
        struct _OverlayLayout
        {
            int monitorIndex;
            bool fillMonitor;
            int width;
            int height;
            int xOffset;
            int yOffset;
            bool fitToGameWindow;
        };
        _OverlayLayout _layout{};
        void _UpdateLayoutFromConfig();

        std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _windowMessageSubscription = nullptr;
        struct _MonitorDesc
        {
            Rect rect;
            bool primary;
        };
        std::vector<_MonitorDesc> _monitors;
        static BOOL _EnumMonitorsProc(HMONITOR, HDC, LPRECT, LPARAM);

        std::optional<HWND> _gameHwnd = std::nullopt;
        TimePoint _lastGameHwndUpdate = TimePoint(0);
        Duration _gameHwndUpdateInterval = Duration(1, SECONDS);
        std::optional<Rect> _GetClientRect(HWND hwnd);
        static BOOL _EnumWindowsProc(HWND, LPARAM);

        std::optional<Rect> _gameRect = std::nullopt;
        TimePoint _lastGameRectUpdate = TimePoint(0);
        Duration _gameRectUpdateInterval = Duration(1, SECONDS);

        bool _layoutChanged = false;

        void _Update();
    };
}