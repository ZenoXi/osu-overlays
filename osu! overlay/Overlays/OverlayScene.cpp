#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "OverlayScene.h"
#include "OverlayConfig.h"
#include "SharedContext.h"

#include "UICore/Components/Base/Dummy.h"

#include "Shared/Util/Streams.h"
#include "Shared/Components/CursorSensitivityConfig.h"

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

            _rawInputEnabled = streams::From(_enabledOverlays).AnyMatch([](const EnabledOverlay& overlay) { return overlay.overlay->RequiresPrecisePointerData(); });
            if (_rawInputEnabled)
                _app->GetMessageWindow()->Backend().EnablePointerRawInputCapture();
            else
                _app->GetMessageWindow()->Backend().DisablePointerRawInputCapture();
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

    _rawWindowMessageSubscription = _app->GetMessageWindow()->Backend().SubscribeToRawWindowMessages([=](UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_INPUT)
        {
            UINT dwSize = 0;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
            std::vector<BYTE> buffer(dwSize);
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &dwSize, sizeof(RAWINPUTHEADER));
            RAWINPUT* raw = (RAWINPUT*)buffer.data();

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                Rect monitorRect = _mainMonitorRect.load();
                int monitorWidth = monitorRect.right - monitorRect.left;
                int monitorHeight = monitorRect.bottom - monitorRect.top;

                int x = raw->data.mouse.lLastX;
                int y = raw->data.mouse.lLastY;
                bool absolute = (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE);
                if (absolute)
                {
                    _rawInputOffset.x = int(x / 65535.0f * monitorRect.right);
                    _rawInputOffset.y = int(y / 65535.0f * monitorRect.bottom);
                }
                else
                {
                    _rawInputOffset.x += x;
                    _rawInputOffset.y += y;
                }
                if (_rawInputOffset.x < _rawInputOffsetBounds.left)
                    _rawInputOffset.x = _rawInputOffsetBounds.left;
                if (_rawInputOffset.y < _rawInputOffsetBounds.top)
                    _rawInputOffset.y = _rawInputOffsetBounds.top;
                if (_rawInputOffset.x > _rawInputOffsetBounds.right)
                    _rawInputOffset.x = _rawInputOffsetBounds.right;
                if (_rawInputOffset.y > _rawInputOffsetBounds.bottom)
                    _rawInputOffset.y = _rawInputOffsetBounds.bottom;

                GameClient gameClient;
                Rect gameRect;
                Rect gameHitTestRect;
                float gameSensitivity;
                {
                    std::lock_guard<std::mutex> lock(_m_rawInput);
                    gameClient = _guardedGameClient;
                    gameRect = _guardedGameRect;
                    gameHitTestRect = _guardedGameHitTestRect;
                    gameSensitivity = _guardedGameSensitivity;
                }

                POINT p;
                GetCursorPos(&p);
                Point cursorPos = Point(p.x, p.y);
                bool insideGameRect = cursorPos.x >= gameHitTestRect.left && cursorPos.y >= gameHitTestRect.top && cursorPos.x < gameHitTestRect.right && cursorPos.y < gameHitTestRect.bottom;
                if (_cursorInsideGameWindow && !_absoluteMode && gameClient == GameClient::STABLE)
                {
                    Point gameCursorPos = _app->Shared<SharedContext*>()->gameCursorPosition;
                    insideGameRect = gameCursorPos.x >= gameHitTestRect.left && gameCursorPos.y >= gameHitTestRect.top && gameCursorPos.x < gameHitTestRect.right && gameCursorPos.y < gameHitTestRect.bottom;
                }
                bool gameFocused = _gameFocused.load();
                bool resetRawPos = false;
                if (insideGameRect && gameFocused && !_cursorInsideGameWindow)
                {
                    _gameWindowEntryPoint = cursorPos;
                    resetRawPos = true;
                }
                _cursorInsideGameWindow = insideGameRect && gameFocused;

                if (absolute)
                {
                    if (!_absoluteMode)
                    {
                        _absoluteMode = true;
                        resetRawPos = true;
                    }
                }
                else
                {
                    if (_absoluteMode)
                    {
                        _absoluteMode = false;
                        _gameWindowEntryPoint = _app->Shared<SharedContext*>()->gameCursorPosition;
                        resetRawPos = true;
                    }
                }

                if (resetRawPos)
                {
                    if (_absoluteMode)
                    {
                        _rawInputOffsetBounds = gameRect;
                    }
                    else
                    {
                        _rawInputOffset = Point(x, y);
                        _rawInputOffsetBounds.left = (int)std::floor((gameRect.left - _gameWindowEntryPoint.x) / gameSensitivity);
                        _rawInputOffsetBounds.top = (int)std::floor((gameRect.top - _gameWindowEntryPoint.y) / gameSensitivity);
                        _rawInputOffsetBounds.right = (int)std::ceil(((gameRect.right - 1) - _gameWindowEntryPoint.x) / gameSensitivity);
                        _rawInputOffsetBounds.bottom = (int)std::ceil(((gameRect.bottom - 1) - _gameWindowEntryPoint.y) / gameSensitivity);
                    }
                }
                Point rawInputOffset = _rawInputOffset;

                Point gameCursorPos{};
                if (_cursorInsideGameWindow)
                {
                    if (!_absoluteMode)
                    {
                        gameCursorPos = _gameWindowEntryPoint + Point(int(rawInputOffset.x * gameSensitivity), int(rawInputOffset.y * gameSensitivity));
                    }
                    else
                    {
                        int xOffsetFromCenter = { rawInputOffset.x - monitorWidth / 2 };
                        int yOffsetFromCenter = { rawInputOffset.y - monitorHeight / 2 };
                        gameCursorPos = {
                            int(xOffsetFromCenter * gameSensitivity) + monitorWidth / 2,
                            int(yOffsetFromCenter * gameSensitivity) + monitorHeight / 2
                        };
                    }
                }
                else
                {
                    gameCursorPos = cursorPos;
                }
                _app->Shared<SharedContext*>()->gameCursorPosition = gameCursorPos;
            }
        }
    });
    
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
    if (windowName == L"osu!" || windowName.starts_with(L"osu! - ") || windowName.starts_with(L"osu!  - "))
    {
        *((HWND*)lparam) = hwnd;
        return FALSE;
    }

    return TRUE;
}

void zcom::OverlayScene::_Update()
{
    if ((ztime::Main() - _lastGameHwndUpdate) >= _gameHwndUpdateInterval)
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

    if ((ztime::Main() - _lastGameRectUpdate) >= _gameRectUpdateInterval)
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

        int monitorWidth = GetSystemMetrics(SM_CXSCREEN);
        int monitorHeight = GetSystemMetrics(SM_CYSCREEN);
        Rect monitorRect = { 0, 0, monitorWidth, monitorHeight };
        _mainMonitorRect = monitorRect;

        _gameFocused = _gameHwnd ? GetForegroundWindow() == _gameHwnd.value() : false;
    }

    if (_rawInputEnabled)
    {
        bool doPositionCalculations = false;
        if (_gameRect)
        {
            std::wstring gameClientStr = _app->config.GetConfigValue(CursorSensitivityConfig::GAME_CLIENT, Config::ADD_AND_SAVE_IF_MISSING);
            float gameSensitivity = _app->config.GetDoubleConfigValue(CursorSensitivityConfig::SENSITIVITY, Config::ADD_AND_SAVE_IF_MISSING);
            bool gameRawInput = _app->config.GetIntConfigValue(CursorSensitivityConfig::RAW_INPUT_ENABLED, Config::ADD_AND_SAVE_IF_MISSING);
            GameClient gameClient = gameClientStr == L"lazer" ? GameClient::LAZER : GameClient::STABLE;

            if (gameRawInput)
            {
                doPositionCalculations = true;

                Rect monitorRect = _mainMonitorRect.load();
                int monitorWidth = monitorRect.right - monitorRect.left;
                int monitorHeight = monitorRect.bottom - monitorRect.top;

                Rect gameRect = _gameRect.value();
                Rect gameHitTestRect = gameRect;
                if (gameClient == GameClient::STABLE)
                {
                    // A fullscreen stable window reports has a size 1px taller than the monitor height (hence the monitorHeight + 1)
                    if (gameRect.left == 0 && gameRect.top == 0 && gameRect.right == monitorWidth && gameRect.bottom == monitorHeight + 1)
                    {
                        gameRect = monitorRect;
                        // Stable has a 1px thick border around the edge of a borderless fullscreen window, where mouse reverts to Windows sensitivity
                        gameHitTestRect = monitorRect.ShrunkBy(1);
                    }
                }

                std::lock_guard<std::mutex> lock(_m_rawInput);
                _guardedGameClient = gameClient;
                _guardedGameRect = gameRect;
                _guardedGameHitTestRect = gameHitTestRect;
                _guardedGameSensitivity = gameSensitivity;
            }
        }

        if (!doPositionCalculations)
        {
            POINT p;
            GetCursorPos(&p);
            Point cursorPos = Point(p.x, p.y);
            _app->Shared<SharedContext*>()->gameCursorPosition = cursorPos;
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