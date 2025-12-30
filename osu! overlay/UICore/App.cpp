#include "App.h"

App::App(HINSTANCE hinst)
    : _hinst(hinst)
    , config(L"config")
    , _windowEvent(EventEmitterThreadMode::MULTITHREADED)
{
    config.LoadConfig();
    _messageWindow = std::make_unique<zwnd::Window>(_hinst);
    _windowCleaningThread = std::thread(&App::_RemoveUnusedWindows, this);
}

App::~App()
{
    _messageWindow->Close();
    _closeThread.store(true);
    if (_windowCleaningThread.joinable())
        _windowCleaningThread.join();
}

std::optional<zwnd::WindowId> App::CreateTopWindow(zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction)
{
    std::unique_lock<std::mutex> lock(_m_windows);
    _windows.push_back({
        std::make_unique<zwnd::Window>(this, zwnd::WindowType::TOP, std::nullopt, props, _hinst, std::move(initFunction), [&](zwnd::Window* window) {
            std::lock_guard<std::mutex> lock(_m_windows);
            _TryDestruct(window->GetWindowId(), true);
        })
    });
    zwnd::WindowId windowId = _windows.back().window->GetWindowId();
    lock.unlock();
    _windowEvent->InvokeAll(WindowEvent{ WindowEvent::CREATED, windowId, zwnd::WindowType::TOP, props });
    return windowId;
}

std::optional<zwnd::WindowId> App::CreateChildWindow(zwnd::WindowId parentWindowId, zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction)
{
    std::unique_lock<std::mutex> lock(_m_windows);

    zwnd::Window* parentWindow = _FindWindow(parentWindowId);
    if (parentWindow == nullptr)
        return std::nullopt;

    // Check if a blocking child already exists
    if (props.blockParent)
    {
        for (auto& window : _windows)
        {
            if (window.window->GetWindowType() != zwnd::WindowType::CHILD)
                continue;
            if (window.window->GetParent().value() == parentWindowId)
                return std::nullopt;
        }
    }

    _windows.push_back({
        std::make_unique<zwnd::Window>(this, zwnd::WindowType::CHILD, std::optional(parentWindowId), props, _hinst, std::move(initFunction), [&, parentWindowId](zwnd::Window* window) {
            std::lock_guard<std::mutex> lock(_m_windows);
            _TryDestruct(window->GetWindowId(), true);
            //zwnd::Window* parentWindow = _FindWindow(parentWindowId);
            //if (parentWindow)
            //    parentWindow->ResetBlockingWindow();
        })
    });

    zwnd::WindowId windowId = _windows.back().window->GetWindowId();
    //parentWindow->SetBlockingWindow(windowId);
    lock.unlock();
    _windowEvent->InvokeAll(WindowEvent{ WindowEvent::CREATED, windowId, zwnd::WindowType::CHILD, props });
    return windowId;
}

std::optional<zwnd::WindowId> App::CreateToolWindow(zwnd::WindowId parentWindowId, zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction)
{
    std::unique_lock<std::mutex> lock(_m_windows);

    zwnd::Window* parentWindow = _FindWindow(parentWindowId);
    if (parentWindow == nullptr)
        return std::nullopt;
    if (parentWindow->GetWindowType() != zwnd::WindowType::TOP && parentWindow->GetWindowType() != zwnd::WindowType::CHILD)
        return std::nullopt;

    _windows.push_back({
        std::make_unique<zwnd::Window>(this, zwnd::WindowType::TOOL, std::optional(parentWindowId), props, _hinst, std::move(initFunction), [&](zwnd::Window* window) {
            std::lock_guard<std::mutex> lock(_m_windows);
            _TryDestruct(window->GetWindowId(), true);
        })
    });

    zwnd::WindowId windowId = _windows.back().window->GetWindowId();
    lock.unlock();
    _windowEvent->InvokeAll(WindowEvent{ WindowEvent::CREATED, windowId, zwnd::WindowType::TOP, props });
    return windowId;
}

std::future<std::optional<zwnd::WindowId>> App::CreateTopWindowAsync(zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction)
{
    // TODO: properly clean up on app close instead of detaching the thread
    std::packaged_task<std::optional<zwnd::WindowId>()> task([=, initFunction = std::move(initFunction)]() mutable { return CreateTopWindow(props, std::move(initFunction)); });
    std::future<std::optional<zwnd::WindowId>> future = task.get_future();
    std::thread(std::move(task)).detach();
    return future;
}

std::future<std::optional<zwnd::WindowId>> App::CreateChildWindowAsync(zwnd::WindowId parentWindowId, zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction)
{
    std::packaged_task<std::optional<zwnd::WindowId>()> task([=, initFunction = std::move(initFunction)]() mutable { return CreateChildWindow(parentWindowId, props, std::move(initFunction)); });
    std::future<std::optional<zwnd::WindowId>> future = task.get_future();
    std::thread(std::move(task)).detach();
    return future;
}

std::future<std::optional<zwnd::WindowId>> App::CreateToolWindowAsync(zwnd::WindowId parentWindowId, zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction)
{
    std::packaged_task<std::optional<zwnd::WindowId>()> task([=, initFunction = std::move(initFunction)]() mutable { return CreateToolWindow(parentWindowId, props, std::move(initFunction)); });
    std::future<std::optional<zwnd::WindowId>> future = task.get_future();
    std::thread(std::move(task)).detach();
    return future;
}

std::unique_ptr<AsyncEventSubscription<void, WindowEvent>> App::SubscribeOnWindowEvent(std::function<void(WindowEvent)> handler)
{
    return _windowEvent->SubscribeAsync(handler);
}

Handle<zwnd::Window> App::GetWindow(zwnd::WindowId windowId)
{
    std::lock_guard<std::mutex> lock(_m_windows);
    for (auto& window : _windows)
    {
        if (window.window->GetWindowId() == windowId && !window.window->Closed())
        {
            window.handleCount++;
            zwnd::WindowId id = window.window->GetWindowId();
            return Handle<zwnd::Window>(window.window.get(), [&, id]() {
                std::lock_guard<std::mutex> lock(_m_windows);
                _ReleaseHandle(id);
            });
        }
    }
    return Handle<zwnd::Window>(nullptr, [&]() {});
}

Handle<zwnd::Window> App::GetWindowNoLock(zwnd::WindowId windowId)
{
    for (auto& window : _windows)
    {
        if (window.window->GetWindowId() == windowId && !window.window->Closed())
        {
            window.handleCount++;
            zwnd::WindowId id = window.window->GetWindowId();
            return Handle<zwnd::Window>(window.window.get(), [&, id]() {
                _ReleaseHandle(id);
            });
        }
    }
    return Handle<zwnd::Window>(nullptr, [&]() {});
}

Handle<zwnd::Window> App::FindWindowByClassName(std::wstring className)
{
    std::lock_guard<std::mutex> lock(_m_windows);
    for (auto& window : _windows)
    {
        if (window.window->Properties().windowClassName == className && !window.window->Closed())
        {
            window.handleCount++;
            zwnd::WindowId id = window.window->GetWindowId();
            return Handle<zwnd::Window>(window.window.get(), [&, id]() {
                std::lock_guard<std::mutex> lock(_m_windows);
                _ReleaseHandle(id);
            });
        }
    }
    return Handle<zwnd::Window>(nullptr, [&]() {});
}

std::vector<zwnd::WindowId> App::GetWindowList()
{
    std::lock_guard<std::mutex> lock(_m_windows);
    std::vector<zwnd::WindowId> ids;
    for (auto& window : _windows)
        ids.push_back(window.window->GetWindowId());
    return ids;
}

zwnd::Window* App::GetMessageWindow()
{
    return _messageWindow.get();
}

bool App::WindowsClosed()
{
    std::lock_guard<std::mutex> lock(_m_windows);
    return _windows.empty();
}

void App::Exit()
{
    std::lock_guard<std::mutex> lock(_m_windows);
    for (auto& window : _windows)
        window.window->Close();
}

zwnd::Window* App::_FindWindow(zwnd::WindowId windowId)
{
    for (auto& window : _windows)
    {
        if (window.window->GetWindowId() == windowId)
        {
            return window.window.get();
        }
    }
    return nullptr;
}

void App::_ReleaseHandle(zwnd::WindowId windowId)
{
    for (auto& window : _windows)
    {
        if (window.window->GetWindowId() == windowId)
        {
            window.handleCount--;
            _TryDestruct(windowId, false);
            return;
        }
    }
}

void App::_TryDestruct(zwnd::WindowId windowId, bool markAsClosed)
{
    for (auto it = _windows.begin(); it != _windows.end(); it++)
    {
        if (it->markedForDeleting)
            continue;
        if (it->window->GetWindowId() != windowId)
            continue;

        // When windows are closed, the window->Closed() flag is set outside of the windows mutex lock.
        //
        // This means that when closing child window, invocations of _TryDestruct can mark the window for deletion
        // after window->Closed() becomes true, but before the close handler (which calls _TryDestruct itself
        // after locking the windows mutex) is called. This gap allows the cleanup thread to lock the mutex
        // itself and try to delete the not yet deletable window, causing a deadlock.
        //
        // This extra flag is set inside the locked context to avoid the issue described
        if (markAsClosed)
            it->closed = true;

        if (!it->window->Closed() || !it->closed)
            return;

        if (it->window->GetWindowType() == zwnd::WindowType::TOP)
        {
            // Close child and tool windows
            bool childWindowsExist = false;
            for (auto& window : _windows)
            {
                if (window.markedForDeleting)
                    continue;
                if (window.window->GetWindowType() != zwnd::WindowType::CHILD && window.window->GetWindowType() != zwnd::WindowType::TOOL)
                    continue;
                if (window.window->GetParent().value() != windowId)
                    continue;
                childWindowsExist = true;
                if (window.window->Closing())
                    continue;

                window.window->Close();
            }
            // Defer window closing until after all child windows are closed
            if (childWindowsExist)
                return;

            if (it->window->Properties().mainWindow)
            {
                // Close all remaining top-level windows
                for (auto& window : _windows)
                {
                    if (window.markedForDeleting)
                        continue;
                    if (window.window->GetWindowType() != zwnd::WindowType::TOP)
                        continue;
                    if (window.window->GetWindowId() == windowId)
                        continue;
                    if (window.window->Closing())
                        continue;

                    window.window->Close();
                }
            }

            _windowEvent->InvokeAll(WindowEvent{ WindowEvent::CLOSED, it->window->GetWindowId(), std::nullopt, std::nullopt });
            it->markedForDeleting = true;
        }
        else if (it->window->GetWindowType() == zwnd::WindowType::CHILD)
        {
            // Close child and tool windows
            bool childWindowsExist = false;
            for (auto& window : _windows)
            {
                if (window.markedForDeleting)
                    continue;
                if (window.window->GetWindowType() != zwnd::WindowType::CHILD && window.window->GetWindowType() != zwnd::WindowType::TOOL)
                    continue;
                if (window.window->GetParent().value() != windowId)
                    continue;
                childWindowsExist = true;
                if (window.window->Closing())
                    continue;

                window.window->Close();
            }
            // Defer window closing until after all child windows are closed
            if (childWindowsExist)
                return;

            _windowEvent->InvokeAll(WindowEvent{ WindowEvent::CLOSED, it->window->GetWindowId(), std::nullopt, std::nullopt });
            it->markedForDeleting = true;
            
            // Invoke parent destruction check
            _TryDestruct(it->window->GetParent().value(), false);
        }
        else if (it->window->GetWindowType() == zwnd::WindowType::TOOL)
        {
            _windowEvent->InvokeAll(WindowEvent{ WindowEvent::CLOSED, it->window->GetWindowId(), std::nullopt, std::nullopt });
            it->markedForDeleting = true;

            // Invoke parent destruction check
            _TryDestruct(it->window->GetParent().value(), false);
        }

        return;
    }
}

void App::_RemoveUnusedWindows()
{
    while (!_closeThread.load())
    {
        std::unique_lock<std::mutex> lock(_m_windows);
        _windows.erase(
            std::remove_if(
                _windows.begin(),
                _windows.end(),
                [](const WindowInfo& info) { return info.markedForDeleting && info.handleCount <= 0; }
            ),
            _windows.end()
        );
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
