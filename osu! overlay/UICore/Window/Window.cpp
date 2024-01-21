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
    _closed.store(true);
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
    _activeScenes.erase(_activeScenes.begin() + index);
    _sceneChanged = true;
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
        _closed.store(true);
    }
    else if (msg.id == MouseMoveMessage::ID())
    {
        MouseMoveMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _BuildMasterPanel()->OnMouseMove(x, y);
    }
    else if (msg.id == MouseEnterMessage::ID())
    {
        // At the moment scene mouse move handler deals with mouse entry
    }
    else if (msg.id == MouseLeaveMessage::ID())
    {
        if (_TitleBarAvailable())
            _titleBarScene->GetCanvas()->OnMouseLeave();
        _nonClientAreaScene->GetCanvas()->OnMouseLeave();
        for (auto& scene : _activeScenes)
            scene->GetCanvas()->OnMouseLeave();
    }
    else if (msg.id == MouseLeftPressedMessage::ID())
    {
        MouseLeftPressedMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _BuildMasterPanel()->OnLeftPressed(x, y);
    }
    else if (msg.id == MouseRightPressedMessage::ID())
    {
        MouseRightPressedMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _BuildMasterPanel()->OnRightPressed(x, y);
    }
    else if (msg.id == MouseLeftReleasedMessage::ID())
    {
        MouseLeftReleasedMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _BuildMasterPanel()->OnLeftReleased(x, y);
    }
    else if (msg.id == MouseRightReleasedMessage::ID())
    {
        MouseRightReleasedMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _BuildMasterPanel()->OnRightReleased(x, y);
    }
    else if (msg.id == MouseWheelUpMessage::ID())
    {
        MouseWheelUpMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _BuildMasterPanel()->OnWheelUp(x, y);
    }
    else if (msg.id == MouseWheelDownMessage::ID())
    {
        MouseWheelDownMessage message{};
        message.Decode(msg);
        int x = message.x;
        int y = message.y;

        _BuildMasterPanel()->OnWheelDown(x, y);
    }
    else if (msg.id == KeyDownMessage::ID())
    {
        KeyDownMessage message{};
        message.Decode(msg);
        keyboardManager.OnKeyDown(message.keyCode);
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
    _window->AddKeyboardHandler(&keyboardManager);

    // Main window loop
    Clock msgTimer = Clock(0);
    while (true)
    {
        // Check for window close
        if (_closed.load())
            break;

        // Messages
        bool msgProcessed = _window->ProcessMessages();

        _window->HandleFullscreenChange();
        _window->HandleCursorVisibility();

        // Limit cpu usage
        if (!msgProcessed)
        {
            // If no messages are received for 50ms or more, sleep to limit cpu usage.
            // This way we allow for full* mouse poll rate utilization when necessary.
            //
            // * the very first mouse move after a break will have a very small delay
            // which may be noticeable in certain situations (FPS games)
            msgTimer.Update();
            if (msgTimer.Now().GetTime(MILLISECONDS) >= 50)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
        {
            msgTimer.Reset();
        }
    }
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

    Clock msgTimer = Clock(0);
    while (true)
    {
        // Check for window close
        if (_closed.load())
            break;

        // Messages
        bool msgProcessed = _window->ProcessMessages();
        _window->ProcessQueueMessages([&](WindowMessage msg) {
            _windowMessageEvent->InvokeAll(msg);
        });

        // Limit cpu usage
        if (!msgProcessed)
        {
            // If no messages are received for 50ms or more, sleep to limit cpu usage.
            // This way we allow for full* mouse poll rate utilization when necessary.
            //
            // * the very first mouse move after a break will have a very small delay
            // which may be noticeable in certain situations (FPS games)
            msgTimer.Update();
            if (msgTimer.Now().GetTime(MILLISECONDS) >= 50)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
        {
            msgTimer.Reset();
        }
    }

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

    while (true)
    {
        _window->LockSize();

        _windowSizeMessage = std::nullopt;
        _window->ProcessQueueMessages([&](WindowMessage msg) { Window::_HandleMessage(msg); });

        ztime::clock[CLOCK_GAME].Update();
        ztime::clock[CLOCK_MAIN].Update();

        // Check for resize
        if (_windowSizeMessage.has_value())
        {
            int newWidth = _windowSizeMessage->width;
            int newHeight = _windowSizeMessage->height;
            zcom::ResizeInfo resizeInfo;
            resizeInfo.windowMaximized = _windowSizeMessage->maximized;
            resizeInfo.windowMinimized = _windowSizeMessage->minimized;
            resizeInfo.windowRestored = _windowSizeMessage->restored;
            RECT clientAreaMargins = _nonClientAreaScene->GetClientAreaMargins();
            int titleBarHeight = _TitleBarAvailable() ? _titleBarScene->TitleBarSceneHeight() : 0;

            // Handle fullscreen change
            if (_fullscreenChanged)
            {
                if (_fullscreenTargetValue)
                    resizeInfo.windowFullscreened = true;
                else if (_window->Maximized())
                    resizeInfo.windowMaximized = true;
                else if (_window->Minimized())
                    resizeInfo.windowMinimized = true;
                else
                    resizeInfo.windowRestored = true;

                _fullscreen = _fullscreenTargetValue;
                _fullscreenChanged = false;
            }

            // Resize regular scenes
            for (auto& scene : _activeScenes)
            {
                if (!_fullscreen)
                {
                    scene->Resize(
                        newWidth - clientAreaMargins.left - clientAreaMargins.right,
                        newHeight - clientAreaMargins.top - clientAreaMargins.bottom - titleBarHeight,
                        resizeInfo
                    );
                    scene->GetCanvas()->BasePanel()->SetWindowPosition(clientAreaMargins.left, clientAreaMargins.top + titleBarHeight);
                }
                else
                {
                    scene->Resize(newWidth, newHeight, resizeInfo);
                    scene->GetCanvas()->BasePanel()->SetWindowPosition(0, 0);
                }
            }
            // Resize title bar scene
            if (_TitleBarAvailable())
            {
                ((zcom::Scene*)_titleBarScene.get())->Resize(
                    newWidth - clientAreaMargins.left - clientAreaMargins.right,
                    _titleBarScene->TitleBarSceneHeight(),
                    resizeInfo
                );
                ((zcom::Scene*)_titleBarScene.get())->GetCanvas()->BasePanel()->SetWindowPosition(clientAreaMargins.left, clientAreaMargins.top);
            }
            // Resize non-client area scene
            ((zcom::Scene*)_nonClientAreaScene.get())->Resize(newWidth, newHeight, resizeInfo);
        }

        // Render frame
        _window->gfx.BeginFrame();

        // Pass title bar item and non-client area scene properties to underlying window thread
        _PassParamsToHitTest();

        bool redraw = false;
        if (_sceneChanged)
        {
            _sceneChanged = false;
            redraw = true;

            // TODO: Resend mouse move message (prefferably have a function in 'WindowBackend')
        }

        // Use a copy because the scene update functions can change the '_activeScenes' ordering
        auto activeScenes = Scenes();

        { // Updating scenes
            for (auto& scene : activeScenes)
                scene->Update();
            if (_TitleBarAvailable())
                ((zcom::Scene*)_titleBarScene.get())->Update();
            ((zcom::Scene*)_nonClientAreaScene.get())->Update();
        }

        { // Redraw checking
            for (auto& scene : activeScenes)
            {
                if (scene->Redraw())
                {
                    redraw = true;
                    break;
                }
            }
            if (!_fullscreen && _TitleBarAvailable() && ((zcom::Scene*)_titleBarScene.get())->Redraw())
                redraw = true;
            if (!_fullscreen && ((zcom::Scene*)_nonClientAreaScene.get())->Redraw())
                redraw = true;
        }

        //std::cout << "Updated " << ++framecounter << '\n';
        //redraw = true;
        if (redraw)
        {
            //if (_parentId.has_value())
            //    std::cout << "Redrawn (" << framecounter++ << ")\n";
            Graphics g = _window->gfx.GetGraphics();
            g.target->BeginDraw();
            g.target->Clear();

            RECT clientAreaMargins = _nonClientAreaScene->GetClientAreaMargins();
            D2D1_SIZE_F clientSize = {
                (float)_window->GetWidth() - (clientAreaMargins.left + clientAreaMargins.right),
                (float)_window->GetHeight() - (clientAreaMargins.top + clientAreaMargins.bottom)
            };
            if (_fullscreen)
            {
                clientSize = {
                    (float)_window->GetWidth(),
                    (float)_window->GetHeight()
                };
            }

            // Create bitmap for client area rendering
            ID2D1Bitmap1* clientAreaBitmap = nullptr;
            g.target->CreateBitmap(
                D2D1::SizeU(clientSize.width, clientSize.height),
                nullptr,
                0,
                D2D1::BitmapProperties1(
                    D2D1_BITMAP_OPTIONS_TARGET,
                    { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
                ),
                &clientAreaBitmap
            );

            // Set content bitmap as target
            ID2D1Image* stash = nullptr;
            g.target->GetTarget(&stash);
            g.target->SetTarget(clientAreaBitmap);
            g.target->Clear();

            if (!_fullscreen && _TitleBarAvailable())
            {
                // Draw the title bar scene
                if (((zcom::Scene*)_titleBarScene.get())->Redraw())
                    ((zcom::Scene*)_titleBarScene.get())->Draw(g);
                if (_titleBarScene->TitleBarSceneHeight() > 0)
                    g.target->DrawBitmap(((zcom::Scene*)_titleBarScene.get())->ContentImage());
            }

            // Draw other scenes
            for (auto& scene : activeScenes)
            {
                if (scene->Redraw())
                    scene->Draw(g);
                g.target->DrawBitmap(
                    scene->ContentImage(),
                    D2D1::RectF(
                        0.0f,
                        (_fullscreen || !_TitleBarAvailable()) ? 0.0f : _titleBarScene->TitleBarSceneHeight(),
                        g.target->GetSize().width,
                        g.target->GetSize().height
                    )
                );
            }

            // Unstash original target
            g.target->SetTarget(stash);
            stash->Release();

            if (!_fullscreen)
            {
                // Draw non-client area scene
                _nonClientAreaScene->SetClientAreaBitmap(clientAreaBitmap);
                ((zcom::Scene*)_nonClientAreaScene.get())->Draw(g);

                g.target->DrawBitmap(((zcom::Scene*)_nonClientAreaScene.get())->ContentImage());
                clientAreaBitmap->Release();
            }
            else
            {
                // Draw client area
                g.target->DrawBitmap(clientAreaBitmap);
                clientAreaBitmap->Release();
            }

            if (GetKeyState(VK_SPACE) & 0x8000)
                g.target->Clear(D2D1::ColorF(0.2f, 0.2f, 0.2f, 0.8f));

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
                    ss.str().length(),
                    dwriteTextFormat,
                    100,
                    30,
                    &dwriteTextLayout
                );

                ID2D1SolidColorBrush* brush;
                g.target->CreateSolidColorBrush(D2D1::ColorF(0.4f, 0.8f, 0.0f, 0.9f), &brush);
                g.target->DrawTextLayout(D2D1::Point2F(5.0f, 5.0f), dwriteTextLayout, brush);

                brush->Release();
                dwriteTextLayout->Release();
            }

            // Update layered window
            _window->UpdateLayeredWindow();

            g.target->EndDraw();
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

    for (auto& scene : _activeScenes)
        scene->Uninit();
    _activeScenes.clear();
    if (_titleBarScene)
    {
        ((zcom::Scene*)_titleBarScene.get())->Uninit();
        _titleBarScene.reset();
    }
    ((zcom::Scene*)_nonClientAreaScene.get())->Uninit();
    _nonClientAreaScene.reset();

    // Release text rendering resources
    dwriteTextFormat->Release();
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

std::unique_ptr<zcom::Panel> zwnd::Window::_BuildMasterPanel()
{
    RECT margins = _nonClientAreaScene->GetClientAreaMargins();

    std::unique_ptr<zcom::Panel> masterPanel = _nonClientAreaScene->CreatePanel();
    masterPanel->Resize(Width(), Height());
    masterPanel->DeferLayoutUpdates();
    if (!_fullscreen)
    {
        zcom::Panel* panel = _nonClientAreaScene->GetCanvas()->BasePanel();
        panel->SetX(0);
        panel->SetY(0);
        masterPanel->AddItem(panel);
    }
    for (auto& scene : _activeScenes)
    {
        zcom::Panel* panel = scene->GetCanvas()->BasePanel();
        if (!_fullscreen)
        {
            panel->SetX(margins.left);
            panel->SetY(margins.top + (_TitleBarAvailable() ? _titleBarScene->TitleBarSceneHeight() : 0));
        }
        else
        {
            panel->SetX(0);
            panel->SetY(0);
        }
        masterPanel->AddItem(panel);
    }
    if (!_fullscreen && _TitleBarAvailable())
    {
        zcom::Panel* panel = _titleBarScene->GetCanvas()->BasePanel();
        panel->SetX(margins.left);
        panel->SetY(margins.top);
        masterPanel->AddItem(panel);
    }
    for (int i = 0; i < masterPanel->ItemCount(); i++)
        masterPanel->GetItem(i)->SetZIndex(i);
    masterPanel->ResumeLayoutUpdates(false);

    return masterPanel;
}

bool zwnd::Window::_TitleBarAvailable()
{
    return _titleBarScene && _type != WindowType::TOOL;
}