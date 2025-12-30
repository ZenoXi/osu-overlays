#pragma once

#include "Window/WindowsEx.h"
#include "Window/Window.h"
#include "Helper/Handle.h"
#include "Helper/EventEmitter.h"
#include "Helper/Config.h"
#include "WindowEvent.h"

#include <mutex>
#include <optional>
#include <future>

class App
{
    // Top level windows:
    //  - Can have property 'main'
    //  - Closing any main window shuts down the application
    //  - Possible to have no main windows, in which case all top level windows must be closed for the app to close
    // Child windows:
    //  - Belong to a top level or child window
    //  - Closing the parent window closes all child windows belonging to the window
    //  - Can have property 'blocking'
    //  - Blocking child windows prevent interaction with the parent window and it's non-blocking child windows
    //  - A window can have only one blocking child active at once
    // Tool windows:
    //  - Belong to a top level or child window
    //  - Don't have a title bar or resizing border
    //  - Meant for small/short lived/popup windows like hover text, context menus, and so on

public:
    App(HINSTANCE hinst);
    ~App();

    std::optional<zwnd::WindowId> CreateTopWindow(zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction);
    std::optional<zwnd::WindowId> CreateChildWindow(zwnd::WindowId parentWindowId, zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction);
    std::optional<zwnd::WindowId> CreateToolWindow(zwnd::WindowId parentWindowId, zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction);

    std::future<std::optional<zwnd::WindowId>> CreateTopWindowAsync(zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction);
    std::future<std::optional<zwnd::WindowId>> CreateChildWindowAsync(zwnd::WindowId parentWindowId, zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction);
    std::future<std::optional<zwnd::WindowId>> CreateToolWindowAsync(zwnd::WindowId parentWindowId, zwnd::WindowProperties props, std::function<void(zwnd::Window* window)> initFunction);

    std::unique_ptr<AsyncEventSubscription<void, WindowEvent>> SubscribeOnWindowEvent(std::function<void(WindowEvent)> handler = nullptr);

    Handle<zwnd::Window> GetWindow(zwnd::WindowId windowId);
    Handle<zwnd::Window> GetWindowNoLock(zwnd::WindowId windowId);
    Handle<zwnd::Window> FindWindowByClassName(std::wstring className);
    std::vector<zwnd::WindowId> GetWindowList();
    zwnd::Window* GetMessageWindow();
    bool WindowsClosed();

    void Exit();
private:
    // _m_windows must be locked before calling
    zwnd::Window* _FindWindow(zwnd::WindowId windowId);
    // _m_windows must be locked before calling
    void _ReleaseHandle(zwnd::WindowId windowId);
    // _m_windows must be locked before calling
    void _TryDestruct(zwnd::WindowId windowId, bool markAsClosed);

    void _RemoveUnusedWindows();

private:
    HINSTANCE _hinst;

    struct WindowInfo
    {
        std::unique_ptr<zwnd::Window> window;
        size_t handleCount = 0;
        bool closed = false;
        bool markedForDeleting = false;
    };
    std::vector<WindowInfo> _windows;
    std::mutex _m_windows;

    EventEmitter<void, WindowEvent> _windowEvent;

    std::unique_ptr<zwnd::Window> _messageWindow;

    std::thread _windowCleaningThread;
    std::atomic<bool> _closeThread;

public:
    Config config;

private:
    // Shared application specific context
    std::any _shared;
public:
    template<class _Shared>
    void SetSharedContext(_Shared shared)
    {
        _shared = std::make_any<_Shared>(shared);
    }
    template<class _Shared>
    _Shared Shared()
    {
        return std::any_cast<_Shared>(_shared);
    }
};