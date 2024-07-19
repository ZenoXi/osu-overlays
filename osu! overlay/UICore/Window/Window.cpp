#include "App.h"
#include "Window.h"

#include "Scenes/ContextMenuScene.h"
#include "Scenes/TooltipScene.h"

zwnd::Window::Window(
    App* app,
    WindowType type,
    std::optional<WindowId> parentWindowId,
    WindowProperties props,
    HINSTANCE hinst,
    std::function<void(Window* window)> initFunction,
    std::function<void(Window* window)> onClosed
)
    : _id(WindowId::Generate()),
    _type(type),
    _parentId(parentWindowId),
    _onClosed(onClosed),
    _windowMessageEvent(EventEmitterThreadMode::MULTITHREADED)
{
    _app = app;
    _props = props;
    _hinst = hinst;

    // Start message thread
    _messageThread = std::thread(&Window::_MessageThread, this);

    // Wait for window to be created
    while (!_windowCreated.load());

    // Init resources and scenes
    initFunction(this);

    _scenesInited.store(true);

    // Start UI thread
    _uiThread = std::thread(&Window::_UIThread, this);

    // Asynchronously create tooltip window
    if (!props.disableFastTooltips)
    {
        // TODO: Wrap into shared_ptr, until project updates to C++23 and std::move_only_function becomes available
        auto subscriptionWrapper = std::make_shared<std::unique_ptr<AsyncEventSubscription<void, zcom::TooltipParams>>>(std::move(_tooltipEventEmitter->SubscribeAsync()));

        _app->CreateToolWindowAsync(
            _id,
            zwnd::WindowProperties()
                .WindowClassName(L"wndClassTooltip")
                .MinSize(1, 1)
                .InitialSize(10, 10)
                .InitialDisplay(zwnd::WindowDisplayType::HIDDEN)
                .DisableWindowAnimations()
                .DisableWindowActivation()
                .DisableMouseInteraction()
                .DisableFastTooltips(),
            [subscriptionWrapper](zwnd::Window* wnd) mutable
            {
                wnd->LoadNonClientAreaScene<zcom::DefaultNonClientAreaScene>(nullptr);

                zcom::TooltipSceneOptions opt;
                opt.showRequestSubscriptionWrapper = subscriptionWrapper;
                //opt.showRequestSubscription = std::move(sub);
                //opt.showRequestEmitter = _tooltipEventEmitter;
                wnd->LoadStartingScene<zcom::TooltipScene>(&opt);
            }
        );
    }
}

zwnd::Window::Window(HINSTANCE hinst)
    : _id(WindowId::Generate()),
    _windowMessageEvent(EventEmitterThreadMode::MULTITHREADED)
{
    _isMessageOnly = true;
    _hinst = hinst;

    // Start message thread
    _messageThread = std::thread(&Window::_MessageOnlyThread, this);

    // Wait for window to be created
    while (!_windowCreated.load());
}

zwnd::Window::~Window()
{
    // _closed should be always true here, since only the App object can destroy Window instances, and it always calls Window::Close before that
    if (_messageThread.joinable())
        _messageThread.join();
}

void zwnd::Window::Fullscreen(bool fullscreen)
{
    if (fullscreen == _fullscreenTargetValue)
        return;

    _fullscreenTargetValue = fullscreen;
    _fullscreenChanged = true;
    _window->SetFullscreen(_fullscreenTargetValue);
}

void zwnd::Window::OpenContextMenu(zcom::MenuTemplate::Menu menuTemplate, RECT sourceItemRect)
{
    RECT windowRect = _window->GetWindowRectangle();
    sourceItemRect.left += windowRect.left;
    sourceItemRect.top += windowRect.top;
    sourceItemRect.right += windowRect.left;
    sourceItemRect.bottom += windowRect.top;

    std::optional<zwnd::WindowId> menuId = _app->CreateToolWindow(
        _id,
        zwnd::WindowProperties()
            .WindowClassName(L"wndClassMenu")
            .InitialSize(10, 10)
            .InitialDisplay(zwnd::WindowDisplayType::HIDDEN)
            .DisableWindowAnimations()
            .DisableWindowActivation(),
        [sourceItemRect, menuTemplate](zwnd::Window* wnd)
        {
            wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
            wnd->resourceManager.InitAllImages();
            wnd->LoadNonClientAreaScene<zcom::DefaultNonClientAreaScene>(nullptr);

            zcom::ContextMenuSceneOptions opt;
            opt.params.parentRect = sourceItemRect;
            opt.params.menuTemplate = menuTemplate;
            wnd->LoadStartingScene<zcom::ContextMenuScene>(&opt);
        }
    );
}

void zwnd::Window::ShowTooltip(zcom::TooltipParams params)
{
    RECT windowRect = _window->GetWindowRectangle();
    params.xPos += windowRect.left;
    params.yPos += windowRect.top;
    if (params.mouseMovementBounds)
    {
        params.mouseMovementBounds.value().left += windowRect.left;
        params.mouseMovementBounds.value().right += windowRect.left;
        params.mouseMovementBounds.value().top += windowRect.top;
        params.mouseMovementBounds.value().bottom += windowRect.top;
    }
    _tooltipEventEmitter->InvokeAll(params);
}

void zwnd::Window::_UninitScene(std::string name)
{
    int index = _GetSceneIndex(name);
    if (index == -1)
        return;

    zcom::Scene* scene = _activeScenes[index].get();
    scene->Uninit();
    _nonClientAreaScene->ProcessDeletedScene(scene);

    _activeScenes.erase(_activeScenes.begin() + index);
    _UpdateSceneZIndices();
}

std::vector<zcom::Scene*> zwnd::Window::Scenes()
{
    std::vector<zcom::Scene*> scenes(_activeScenes.size());
    for (int i = 0; i < _activeScenes.size(); i++)
        scenes[i] = _activeScenes[i].get();
    return scenes;
}

zcom::DefaultTitleBarScene* zwnd::Window::GetTitleBarScene()
{
    return _titleBarScene.get();
}

zcom::DefaultNonClientAreaScene* zwnd::Window::GetNonClientAreaScene()
{
    return _nonClientAreaScene.get();
}

zcom::Scene* zwnd::Window::_GetScene(std::string name)
{
    for (int i = 0; i < _activeScenes.size(); i++)
        if (_activeScenes[i]->GetName() == name)
            return _activeScenes[i].get();
    return nullptr;
}

int zwnd::Window::_GetSceneIndex(std::string name)
{
    for (int i = 0; i < _activeScenes.size(); i++)
        if (_activeScenes[i]->GetName() == name)
            return i;
    return -1;
}

void zwnd::Window::_HandleMessage(WindowMessage msg)
{
    if (msg.id == WindowSizeMessage::ID())
    {
        WindowSizeMessage message{};
        message.Decode(msg);
        _windowSizeMessage = message;
    }
    else if (msg.id == WindowMoveMessage::ID())
    {

    }
    else if (msg.id == WindowCloseMessage::ID())
    {
        Close();
    }
    else if (msg.id == MouseMoveMessage::ID())
    {
        MouseMoveMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        if (!_nonClientAreaScene->GetBasePanel()->GetMouseInside())
            _nonClientAreaScene->GetBasePanel()->OnMouseEnter();
        if (!_nonClientAreaScene->GetBasePanel()->GetMouseInsideArea())
            _nonClientAreaScene->GetBasePanel()->OnMouseEnterArea();
        _nonClientAreaScene->GetBasePanel()->OnMouseMove(x, y);
    }
    else if (msg.id == MouseEnterMessage::ID())
    {
        // At the moment scene mouse move handler deals with mouse entry
    }
    else if (msg.id == MouseLeaveMessage::ID())
    {
        _nonClientAreaScene->GetBasePanel()->OnMouseLeave();
    }
    else if (msg.id == MouseLeftPressedMessage::ID())
    {
        MouseLeftPressedMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        zcom::EventTargets targets = _nonClientAreaScene->GetBasePanel()->OnLeftPressed(x, y);

        // Update selected item
        zcom::Component* mainTarget = targets.MainTarget();
        for (auto component : _nonClientAreaScene->GetBasePanel()->GetAllChildren())
            if (component != mainTarget && component->Selected())
                component->OnDeselected();
        if (mainTarget != nullptr && !mainTarget->Selected() && mainTarget->GetSelectable())
            mainTarget->OnSelected();
    }
    else if (msg.id == MouseRightPressedMessage::ID())
    {
        MouseRightPressedMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _nonClientAreaScene->GetBasePanel()->OnRightPressed(x, y);
    }
    else if (msg.id == MouseLeftReleasedMessage::ID())
    {
        MouseLeftReleasedMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _nonClientAreaScene->GetBasePanel()->OnLeftReleased(x, y);
    }
    else if (msg.id == MouseRightReleasedMessage::ID())
    {
        MouseRightReleasedMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _nonClientAreaScene->GetBasePanel()->OnRightReleased(x, y);
    }
    else if (msg.id == MouseWheelUpMessage::ID())
    {
        MouseWheelUpMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _nonClientAreaScene->GetBasePanel()->OnWheelUp(x, y);
    }
    else if (msg.id == MouseWheelDownMessage::ID())
    {
        MouseWheelDownMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _nonClientAreaScene->GetBasePanel()->OnWheelDown(x, y);
    }
    else if (msg.id == KeyDownMessage::ID())
    {
        KeyDownMessage message{};
        message.Decode(msg);
        bool handled = keyboardManager.OnKeyDown(message.keyCode);
        if (!handled)
        {
            // Advance selected element
            bool reverse = keyboardManager.KeyState(VK_SHIFT);
            zcom::Component* itemToSelect = _nonClientAreaScene->GetBasePanel()->IterateTab(reverse);
            for (auto component : _nonClientAreaScene->GetBasePanel()->GetAllChildren())
                if (component != itemToSelect && component->Selected())
                    component->OnDeselected();
            if (itemToSelect != nullptr)
                itemToSelect->OnSelected(reverse);
        }
    }
    else if (msg.id == KeyUpMessage::ID())
    {
        KeyUpMessage message{};
        message.Decode(msg);
        keyboardManager.OnKeyUp(message.keyCode);
    }
    else if (msg.id == CharMessage::ID())
    {
        CharMessage message{};
        message.Decode(msg);
        keyboardManager.OnChar(message.character);
    }

    _windowMessageEvent->InvokeAll(msg);
}

void zwnd::Window::_MessageThread()
{
    // Create window
    HWND parentWindow = NULL;
    if (_parentId.has_value())
    {
        // When this is called, _app holds the lock in it's window creation function
        Handle<Window> hwnd = _app->GetWindowNoLock(_parentId.value());
        if (hwnd.Valid())
        {
            // TODO: Should add window locking, since the backend pointer can be released while in use

            auto view = hwnd->Backend();
            if (view.Valid())
            {
                parentWindow = view.WindowHandle();
            }
        }
    }
    _window = std::make_unique<WindowBackend>(_hinst, _props, parentWindow);

    // Pass the device context to the resource manager
    resourceManager.CoInit();
    resourceManager.SetDeviceContext(_window->gfx.GetGraphics().target);

    // Wait for scene and resource init
    _windowCreated.store(true);
    while (!_scenesInited.load());

    // Add handlers
    //_window->AddKeyboardHandler(&keyboardManager);

    // Main window loop
    _window->ProcessMessages();
    _closed.store(true);
    _uiThread.join();

    resourceManager.ReleaseResources();
    resourceManager.CoUninit();

    _window.reset();
    _onClosed(this);
}

void zwnd::Window::_MessageOnlyThread()
{
    _window = std::make_unique<WindowBackend>(_hinst);
    _windowCreated.store(true);

    while (_window->ProcessSingleMessage())
    {
        _window->ProcessQueueMessages([&](WindowMessage msg) {
            _windowMessageEvent->InvokeAll(msg);
        });
    }
    _closed.store(true);

    _window.reset();
}

void zwnd::Window::_UIThread()
{
    int framecounter = 0;
    Clock frameTimer = Clock(0);

    // Create frame number debug text rendering resources
    IDWriteFactory* dwriteFactory = nullptr;
    IDWriteTextFormat* dwriteTextFormat = nullptr;
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&dwriteFactory)
    );
    if (dwriteFactory)
    {
        dwriteFactory->CreateTextFormat(
            L"Calibri",
            NULL,
            DWRITE_FONT_WEIGHT_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            20.0f,
            L"en-us",
            &dwriteTextFormat
        );
        if (!dwriteTextFormat)
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }

    while (true)
    {
        _window->LockSize();

        // TODO: these probably should be window/thread specific
        ztime::clock[CLOCK_GAME].Update();
        ztime::clock[CLOCK_MAIN].Update();

        _windowSizeMessage = std::nullopt;
        _window->ProcessQueueMessages([&](WindowMessage msg) { Window::_HandleMessage(msg); });

        // Check for resize
        if (_windowSizeMessage.has_value())
        {
            int newWidth = _windowSizeMessage->width;
            int newHeight = _windowSizeMessage->height;
            ResizeFlags resizeFlags;
            resizeFlags.windowMaximized = _windowSizeMessage->maximized;
            resizeFlags.windowMinimized = _windowSizeMessage->minimized;
            resizeFlags.windowRestored = _windowSizeMessage->restored;

            // Handle fullscreen change
            if (_fullscreenChanged)
            {
                if (_fullscreenTargetValue)
                    resizeFlags.windowFullscreened = true;
                else if (_window->Maximized())
                    resizeFlags.windowMaximized = true;
                else if (_window->Minimized())
                    resizeFlags.windowMinimized = true;
                else
                    resizeFlags.windowRestored = true;

                _fullscreen = _fullscreenTargetValue;
                _fullscreenChanged = false;
            }

            _nonClientAreaScene->ProcessWindowResize(newWidth, newHeight, resizeFlags);

            WindowSizeExMessage sizeExMsg;
            sizeExMsg.width = newWidth;
            sizeExMsg.height = newHeight;
            sizeExMsg.flags = resizeFlags;
            _windowMessageEvent->InvokeAll(sizeExMsg.Encode());
        }

        // Render frame
        _window->gfx.BeginFrame();

        // Pass title bar item and non-client area scene properties to underlying window thread
        _PassParamsToHitTest();

        // Update and render the UI
        _nonClientAreaScene->GetBasePanel()->Update();
        bool redraw = _nonClientAreaScene->GetBasePanel()->Redraw();
        if (redraw)
        {
            //SimpleTimer drawTimer;

            //if (_parentId.has_value())
            //    std::cout << "Redrawn (" << framecounter++ << ")\n";
            Graphics g = _window->gfx.GetGraphics();
            g.target->BeginDraw();
            g.target->Clear();

            _nonClientAreaScene->GetBasePanel()->Draw(g);
            g.target->DrawBitmap(_nonClientAreaScene->GetBasePanel()->ContentImage());

            if (dwriteFactory && dwriteTextFormat)
            {
                // Display frame number while 'Ctrl + S + F' is held
                if ((GetKeyState(VK_CONTROL) & 0x8000) &&
                    (GetKeyState('S') & 0x8000) &&
                    (GetKeyState('F') & 0x8000))
                {
                    IDWriteTextLayout* dwriteTextLayout = nullptr;

                    std::wstringstream ss;
                    ss << framecounter++;
                    dwriteFactory->CreateTextLayout(
                        ss.str().c_str(),
                        (UINT32)ss.str().length(),
                        dwriteTextFormat,
                        100,
                        30,
                        &dwriteTextLayout
                    );
                    if (dwriteTextLayout)
                    {
                        ID2D1SolidColorBrush* brush;
                        g.target->CreateSolidColorBrush(D2D1::ColorF(0.4f, 0.8f, 0.0f, 0.9f), &brush);
                        if (brush)
                        {
                            g.target->DrawTextLayout(D2D1::Point2F(5.0f, 5.0f), dwriteTextLayout, brush);
                            brush->Release();
                        }
                        else
                        {
                            // TODO: Logging
                        }
                        dwriteTextLayout->Release();
                    }
                    else
                    {
                        // TODO: Logging
                    }
                }
            }

            // Update layered window
            _window->UpdateLayeredWindow();

            g.target->EndDraw();
            //std::cout << drawTimer.MicrosElapsed() << "us\n";
        }

        _window->UnlockSize();

        // Uninit scenes
        while (!_scenesToUninitialize.empty())
        {
            _UninitScene(_scenesToUninitialize.front());
            _scenesToUninitialize.pop_front();
        }

        _window->gfx.EndFrame(redraw);

        if (_closed.load())
            break;
    }

    // Uninit all scenes
    for (auto& scene : _activeScenes)
    {
        scene->Uninit();
        _nonClientAreaScene->ProcessDeletedScene(scene.get());
    }
    _activeScenes.clear();
    if (_titleBarScene)
    {
        ((zcom::Scene*)_titleBarScene.get())->Uninit();
        _nonClientAreaScene->ProcessDeletedTitleBarScene(_titleBarScene.get());
        _titleBarScene.reset();
    }
    ((zcom::Scene*)_nonClientAreaScene.get())->Uninit();
    _nonClientAreaScene.reset();

    // Release text rendering resources
    if (dwriteTextFormat)
        dwriteTextFormat->Release();
    if (dwriteFactory)
        dwriteFactory->Release();
}

void zwnd::Window::_PassParamsToHitTest()
{
    if (!_fullscreen)
    {
        if (_type == WindowType::TOOL)
            _window->SetResizingBorderMargins({0, 0, 0, 0});
        else
            _window->SetResizingBorderMargins(_nonClientAreaScene->GetResizingBorderWidths());

        RECT clientAreaMargins = _nonClientAreaScene->GetClientAreaMargins();
        _window->SetClientAreaMargins(clientAreaMargins);

        if (_TitleBarAvailable())
        {
            _window->SetCaptionHeight(_titleBarScene->CaptionHeight());

            RECT windowMenuButtonRect = _titleBarScene->WindowMenuButtonRect();
            // Transform rect to window coordinates
            windowMenuButtonRect.left += clientAreaMargins.left;
            windowMenuButtonRect.right += clientAreaMargins.left;
            windowMenuButtonRect.top += clientAreaMargins.top;
            windowMenuButtonRect.bottom += clientAreaMargins.top;
            _window->SetWinMenuButtonRect(windowMenuButtonRect);

            std::vector<RECT> excludedRects = _titleBarScene->ExcludedCaptionRects();
            // Transform rects to window coordinates
            for (auto& rect : excludedRects)
            {
                rect.left += clientAreaMargins.left;
                rect.right += clientAreaMargins.left;
                rect.top += clientAreaMargins.top;
                rect.bottom += clientAreaMargins.top;
            }
            _window->SetExcludedCaptionRects(excludedRects);
        }
    }
    else
    {
        RECT nullRect = { 0, 0, 0, 0 };
        _window->SetResizingBorderMargins(nullRect);
        _window->SetClientAreaMargins(nullRect);
        _window->SetCaptionHeight(0);
        _window->SetWinMenuButtonRect(nullRect);
        _window->SetExcludedCaptionRects({});
    }
}

void zwnd::Window::_UpdateSceneZIndices()
{
    for (int i = 0; i < _activeScenes.size(); i++)
        _activeScenes[i]->GetBasePanel()->SetZIndex(i);
}

bool zwnd::Window::_TitleBarAvailable()
{
    return _titleBarScene && _type != WindowType::TOOL;
}