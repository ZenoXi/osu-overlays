#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "OverlayScene.h"
#include "OverlayConfig.h"

#include "UICore/Components/Base/Dummy.h"

#include "Shared/Util/Streams.h"

void zcom::OverlayScene::Init(SceneOptionsBase* options)
{
    OverlaySceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const OverlaySceneOptions*>(options);

    _overlayEventSubscription = opt.overlayEventEmitter->SubscribeAsync([=](OverlayEvent event) {
        _basePanel->ExecuteSynchronously([=]() {
            if (event.type == OverlayEvent::OVERLAY_ENABLED)
            {
                _enabledOverlays.push_back({ event.affectedOverlay, event.affectedOverlay->CreateOverlayComponent(event.affectedOverlay, _basePanel) });
                _basePanel->AddItem(_enabledOverlays.back().overlayComponent.get());
            }
            else if (event.type == OverlayEvent::OVERLAY_DISABLED)
            {
                for (auto it = _enabledOverlays.begin(); it != _enabledOverlays.end(); it++)
                {
                    if (it->overlay->Id() == event.affectedOverlay->Id())
                    {
                        _basePanel->RemoveItem(it->overlayComponent.get());
                        _enabledOverlays.erase(it);
                        break;
                    }
                }
            }

            bool vsyncDisabled = streams::From(_enabledOverlays).AnyMatch([](const EnabledOverlay& overlay) { return overlay.overlay->RequiresUncappedFramerate(); });
            if (vsyncDisabled)
                _window->Backend().Graphics()->DisableVsync();
            else
                _window->Backend().Graphics()->EnableVsync();
        });
    });

    _configValueChangedEventSubscription = _app->config.SubscribeOnConfigValueChanged();
    _configValueChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        bool layoutChanged = changes.has_value() && (
                changes->first == OverlayConfig::MONITOR_INDEX.name ||
                changes->first == OverlayConfig::FILL_MONITOR.name ||
                changes->first == OverlayConfig::WIDTH.name ||
                changes->first == OverlayConfig::HEIGHT.name ||
                changes->first == OverlayConfig::X_OFFSET.name ||
                changes->first == OverlayConfig::Y_OFFSET.name ||
                changes->first == OverlayConfig::FIT_TO_GAME_WINDOW.name
            );

        if (layoutChanged)
        {
            _basePanel->ExecuteSynchronously([=]() {
                _UpdateLayoutFromConfig();
            });
        }
    });
    _UpdateLayoutFromConfig();

    struct DisplayChangeMessage { static const char* ID() { return "display_change"; } };
    // OverlayScene is initialized only once when the overlay window is created, so the message registration can safely be done here, without worrying about duplication
    _window->Backend().RegisterMessage(WM_DISPLAYCHANGE, [](WPARAM, LPARAM) { return zwnd::WindowMessage{ DisplayChangeMessage::ID() }; });
    _windowMessageSubscription = _window->SubscribeToWindowMessages([=](zwnd::WindowMessage msg) {
        if (msg.id == DisplayChangeMessage::ID())
        {
            _basePanel->ExecuteSynchronously([=]() {
                _monitors.clear();
                EnumDisplayMonitors(NULL, NULL, _EnumMonitorsProc, (LPARAM)&_monitors);
                _layoutChanged = true;
            });
        }
        return true;
    });
    EnumDisplayMonitors(NULL, NULL, _EnumMonitorsProc, (LPARAM)&_monitors);

    auto notificationPanel = Create<NotificationDisplayComponent>();
    notificationPanel->size = { 360, 0 };
    notificationPanel->parentSize = { 0.0f, 1.0f };
    notificationPanel->xAlign = Alignment::END;
    notificationPanel->zIndex = 1000;
    _basePanel->AddItem(std::move(notificationPanel));

    _basePanel->SubscribePostUpdate([=]() { _Update(); }).Detach();
}

void zcom::OverlayScene::_UpdateLayoutFromConfig()
{
    _layout.monitorIndex = (int)_app->config.GetIntConfigValue(OverlayConfig::MONITOR_INDEX, Config::ADD_AND_SAVE_IF_MISSING);
    _layout.fillMonitor = (bool)_app->config.GetIntConfigValue(OverlayConfig::FILL_MONITOR, Config::ADD_AND_SAVE_IF_MISSING);
    _layout.width = (int)_app->config.GetIntConfigValue(OverlayConfig::WIDTH, Config::ADD_AND_SAVE_IF_MISSING);
    _layout.height = (int)_app->config.GetIntConfigValue(OverlayConfig::HEIGHT, Config::ADD_AND_SAVE_IF_MISSING);
    _layout.xOffset = (int)_app->config.GetIntConfigValue(OverlayConfig::X_OFFSET, Config::ADD_AND_SAVE_IF_MISSING);
    _layout.yOffset = (int)_app->config.GetIntConfigValue(OverlayConfig::Y_OFFSET, Config::ADD_AND_SAVE_IF_MISSING);
    _layout.fitToGameWindow = (bool)_app->config.GetIntConfigValue(OverlayConfig::FIT_TO_GAME_WINDOW, Config::ADD_AND_SAVE_IF_MISSING);
    _layoutChanged = true;
}

BOOL zcom::OverlayScene::_EnumMonitorsProc(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM lparam)
{
    MONITORINFO info{};
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &info);
    auto monitors = (std::vector<_MonitorDesc>*)lparam;
    monitors->push_back({ Rect{ rect->left, rect->top, rect->right, rect->bottom }, bool(info.dwFlags & MONITORINFOF_PRIMARY) });
    return TRUE;
}

std::optional<zcom::Rect> zcom::OverlayScene::_GetClientRect(HWND hwnd)
{
    RECT windowRect;
    if (!GetWindowRect(hwnd, &windowRect))
        return std::nullopt;
    RECT clientRect;
    if (!GetClientRect(hwnd, &clientRect))
        return std::nullopt;
    POINT point = { windowRect.left, windowRect.top };
    if (!ScreenToClient(hwnd, &point))
        return std::nullopt;

    Rect newRect = Rect{
        windowRect.left - point.x,
        windowRect.top - point.y,
        windowRect.left - point.x + clientRect.right,
        windowRect.top - point.y + clientRect.bottom
    };

    return newRect;
}

BOOL zcom::OverlayScene::_EnumWindowsProc(HWND hwnd, LPARAM lparam)
{
    wchar_t buffer[256];
    GetWindowText(hwnd, buffer, 256);

    std::wstring windowName(buffer);
    if (windowName == L"osu!" || windowName.starts_with(L"osu!  - "))
    {
        *((HWND*)lparam) = hwnd;
        return FALSE;
    }

    return TRUE;
}

void zcom::OverlayScene::_Update()
{
    if (_layout.fitToGameWindow && (ztime::Main() - _lastGameHwndUpdate) >= _gameHwndUpdateInterval)
    {
        _lastGameHwndUpdate = ztime::Main();

        if (!_gameHwnd)
        {
            HWND gameHwnd = NULL;
            EnumWindows(_EnumWindowsProc, (LPARAM)&gameHwnd);
            if (gameHwnd != NULL)
                _gameHwnd = gameHwnd;
        }
    }

    if (_layout.fitToGameWindow && (ztime::Main() - _lastGameRectUpdate) >= _gameRectUpdateInterval)
    {
        _lastGameRectUpdate = ztime::Main();

        if (_gameHwnd)
        {
            std::optional<Rect> newRect = _GetClientRect(_gameHwnd.value());
            if (newRect)
            {
                if (!_gameRect || _gameRect != newRect)
                {
                    _gameRect = newRect;
                    _layoutChanged = true;
                }
            }
            else
            {
                _gameHwnd = std::nullopt;
                if (_gameRect)
                {
                    _gameRect = std::nullopt;
                    _layoutChanged = true;
                }
            }
        }
        else if (_gameRect)
        {
            _gameRect = std::nullopt;
            _layoutChanged = true;
        }
    }


    if (_layoutChanged)
    {
        _layoutChanged = false;

        if (_layout.monitorIndex < 0 || _layout.monitorIndex > _monitors.size())
        {
            _layout.monitorIndex = 0;
            _app->config.SetIntValue(OverlayConfig::MONITOR_INDEX.name, 0);
        }

        Rect monitorRect{};
        for (int i = 0; i < _monitors.size(); i++)
        {
            if ((_layout.monitorIndex == 0 && _monitors[i].primary) || _layout.monitorIndex == i + 1)
            {
                monitorRect = _monitors[i].rect;
                break;
            }
        }

        Rect overlayRect = monitorRect;
        if (_layout.fitToGameWindow && _gameRect)
        {
            overlayRect = _gameRect.value();
        }
        else if (!_layout.fillMonitor)
        {
            overlayRect = Rect{
                monitorRect.left + _layout.xOffset,
                monitorRect.top + _layout.yOffset,
                monitorRect.left + _layout.xOffset + _layout.width,
                monitorRect.top + _layout.yOffset + _layout.height
            };
        }

        _window->Backend().SetWindowRectangle(RectToRECT(overlayRect));
        std::cout << "overlay layout updated\n";
    }
}
