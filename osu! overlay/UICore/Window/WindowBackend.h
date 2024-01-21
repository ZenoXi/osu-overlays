#pragma once

#include "WindowsEx.h"
#pragma comment( lib,"Shell32.lib" )

#include "WindowProperties.h"
#include "WindowGraphics.h"
#include "WindowMessage.h"
#include "WindowDisplayType.h"

#include "CursorIcon.h"

#include "MouseEventHandler.h"
#include "KeyboardEventHandler.h"
#include "FileDropHandler.h"

#include <string>
#include <thread>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <optional>

namespace zwnd
{
    struct MessageWindowSize
    {
        int width;
        int height;
        bool changed;
    };

    enum class MouseWindowInteraction
    {
        DEFAULT, // Mouse interacts with the window as usual
        PASS_THROUGH // The window is invisible to mouse events
    };

    class LayeredWindowInfo
    {
        const POINT _sourcePosition;
        POINT _windowPosition;
        SIZE _size;
        BLENDFUNCTION _blend;
        UPDATELAYEREDWINDOWINFO _info;

    public:
        LayeredWindowInfo()
            :
            _sourcePosition(),
            _windowPosition(),
            _size(),
            _blend(),
            _info()
        {
            _info.cbSize = sizeof(UPDATELAYEREDWINDOWINFO);
            _info.pptSrc = &_sourcePosition;
            _info.pptDst = &_windowPosition;
            _info.psize = &_size;
            _info.pblend = &_blend;
            _info.dwFlags = ULW_ALPHA;

            _blend.SourceConstantAlpha = 255;
            _blend.AlphaFormat = AC_SRC_ALPHA;
        }

        void Update(HWND window, HDC source)
        {
            _info.hdcSrc = source;
            RECT rect;
            GetWindowRect(window, &rect);
            _windowPosition.x = rect.left;
            _windowPosition.y = rect.top;

            //_info.pptDst = NULL;
            //_info.psize = NULL;
            //_info.dwFlags |= ULW_EX_NORESIZE;

            //std::cout << _info.psize->cx << ':' << _info.psize->cy << '\n';
            //_size.cx++;
            BOOL res = UpdateLayeredWindowIndirect(window, &_info);
            if (res == 0)
            {
                //std::cout << "LAYERED WINDOW UPDATE ERROR " << _info.psize->cx << ':' << _info.psize->cy << " vs " << GetWidth() << ':' << GetHeight() << '\n';
                //std::cout << "LAYERED WINDOW UPDATE ERROR\n";
            }
        }

        void SetWidth(UINT width)
        {
            _size.cx = width;
        }
        void SetHeight(UINT height)
        {
            _size.cy = height;
        }

        UINT GetWidth() const { return _size.cx; }
        UINT GetHeight() const { return _size.cy; }
    };

    class WindowBackend
    {
        HWND _hwnd = NULL;
        HWND _parentHwnd = NULL;
        LayeredWindowInfo _linfo;

        LPCWSTR _wndClassName = L"wndClassName";
        HINSTANCE _hInst;

        bool _mouseInWindow = false;
        HCURSOR _prevCursor = 0;
        HCURSOR _cursor = 0;

        RECT _borderThickness;

        // Handlers
        std::vector<KeyboardEventHandler*> _keyboardHandlers;
        std::unique_ptr<FileDropHandler> _fileDropHandler;

    public:
        WindowGraphics gfx;

        WindowBackend(HINSTANCE hInst, WindowProperties props, HWND parentWindow);
        // Message-only window constructor
        WindowBackend(HINSTANCE hInst);
        WindowBackend(const WindowBackend&) = delete;
        WindowBackend& operator=(const WindowBackend&) = delete;
        ~WindowBackend();

        HWND GetHWND() const { return _hwnd; }

        // Causes the handler of WM_WINDOWPOSCHANGING message to wait until UnlockSize()
        // is called, effectively preventing the window from changing its size.
        // While the lock is in effect, the window message loop will freeze after
        // receiving a WM_WINDOWPOSCHANGING message
        void LockSize();
        // Resumes processing of WM_WINDOWPOSCHANGING messages
        void UnlockSize();

        void UpdateLayeredWindow();

        bool ProcessMessages();
        void ProcessQueueMessages(std::function<void(WindowMessage)> callback);

        void AddKeyboardHandler(KeyboardEventHandler* handler);
        bool RemoveKeyboardHandler(KeyboardEventHandler* handler);
        void AddDragDropHandler(IDragDropEventHandler* handler);
        bool RemoveDragDropHandler(IDragDropEventHandler* handler);

        RECT GetWindowRectangle();
        void SetWindowRectangle(RECT rect);
        int GetWidth();
        int GetHeight();
        void SetWidth(int newWidth);
        void SetHeight(int newHeight);
        void SetSize(int newWidth, int newHeight);
        int GetXPos();
        int GetYPos();
        void SetXPos(int newX);
        void SetYPos(int newY);
        void SetPosition(int newX, int newY);
        bool Maximized();
        void Maximize();
        bool Minimized();
        void Minimize();
        void Restore();

        void SetDisplayType(WindowDisplayType displayType);

        RECT GetMonitorRectAtScreenPoint(int x, int y);
        RECT GetMonitorRectAtWindowPoint(int x, int y);

        void SetFullscreen(bool fullscreen);
        bool IsFullscreen();
        void SetCursorIcon(CursorIcon cursor);
        // Shows/Hides the cursor until it is moved or this function is called
        void SetCursorVisibility(bool visible);
        // Resets screen shutoff timer
        void ResetScreenTimer();
        // Sets how the mouse interacts with the window
        void SetMouseInteraction(MouseWindowInteraction interactionType);

        void HandleFullscreenChange();
        void HandleCursorVisibility();

        void SetResizingBorderMargins(RECT margins);
        void SetClientAreaMargins(RECT margins);
        void SetCaptionHeight(int height);
        void SetCloseButtonRect(RECT rect);
        void SetMinimizeButtonRect(RECT rect);
        void SetMaximizeButtonRect(RECT rect);
        void SetWinMenuButtonRect(RECT rect);
        void SetExcludedCaptionRects(const std::vector<RECT>& rects);

    private:
        static LRESULT WINAPI _HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT WINAPI _HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        bool _messageOnly = false;

        std::mutex _m_msg;
        WindowMessage _sizeResult;
        WindowMessage _moveResult;
        WindowMessage _exitResult;
        std::queue<WindowMessage> _msgQueue;

        // Window width, updated only in the WM_SIZE messages
        int _messageWidth;
        // Window height, updated only in the WM_SIZE messages
        int _messageHeight;
        std::mutex _m_windowSize;

        // Should be set to true, when displaying the window for the first time using ShowWindow().
        // When this flag is set, WM_WINDOWPOSCHANGING and WM_SIZE won't use a mutex, since certain
        // SW_*** flags result in WM_SIZE messages with a preceding WM_WINDOWPOSCHANGING message
        // that has the SWP_NOSIZE flag set, which leads into attempt to unlock an unlocked mutex.
        bool _insideInitialShowWindowCall = false;
        bool _initialShowWindowCallDone = false;

        bool _fullscreen = false;
        bool _maximized = false;
        bool _minimized = false;
        RECT _windowedRect;
        bool _windowedMaximized;
        bool _cursorVisible = true;
        bool _cursorVisibilityChanged = false;

        bool _activationDisabled = false;

        RECT _last2Moves[2];
        POINT _lastMouseMove;

        // Flag that gets set to true when window sizing (or moving, but moving is irrelevant) starts
        bool _sizingStarted = false;

        bool _fullscreenChanged = false;
        std::mutex _m_fullscreen;

        std::mutex _m_hittest;
        RECT _resizingBorderMargins;
        RECT _clientAreaMargins;
        int _titleBarHeight;
        RECT _closeButtonRect;
        RECT _minimizeButtonRect;
        RECT _maximizeButtonRect;
        RECT _winMenuButtonRect;
        std::vector<RECT> _excludedCaptionRects;
    };

    // Window backend view class, used to hide more esoteric window functions which are used by the 'Window' class
    class WindowBackendView
    {
    public:
        WindowBackendView(WindowBackend* wnd) : _wnd(wnd) {}

        bool Valid() { return _wnd != nullptr; }
        // Returns the window handle
        // Only use if you know what you're doing
        HWND WindowHandle() { return _wnd->GetHWND(); }
        // Returns a pointer to the window graphics object
        WindowGraphics* Graphics() { return &_wnd->gfx; }

        RECT GetWindowRectangle() { return _wnd->GetWindowRectangle(); }
        void SetWindowRectangle(RECT rect) { _wnd->SetWindowRectangle(rect); }
        int GetWidth() { return _wnd->GetWidth(); }
        int GetHeight() { return _wnd->GetHeight(); }
        void SetWidth(int newWidth) { _wnd->SetWidth(newWidth); }
        void SetHeight(int newHeight) { _wnd->SetHeight(newHeight); }
        void SetSize(int newWidth, int newHeight) { _wnd->SetSize(newWidth, newHeight); }
        int GetXPos() { return _wnd->GetXPos(); }
        int GetYPos() { return _wnd->GetYPos(); }
        void SetXPos(int newX) { _wnd->SetXPos(newX); }
        void SetYPos(int newY) { _wnd->SetYPos(newY); }
        void SetPosition(int newX, int newY) { _wnd->SetPosition(newX, newY); }
        bool Maximized() { return _wnd->Maximized(); }
        void Maximize() { _wnd->Maximize(); }
        bool Minimized() { return _wnd->Minimized(); }
        void Minimize() { _wnd->Minimize(); }
        void Restore() { _wnd->Restore(); }

        void SetDisplayType(WindowDisplayType displayType) { _wnd->SetDisplayType(displayType); }

        RECT GetMonitorRectAtScreenPoint(int x, int y) { return _wnd->GetMonitorRectAtScreenPoint(x, y); }
        RECT GetMonitorRectAtWindowPoint(int x, int y) { return _wnd->GetMonitorRectAtWindowPoint(x, y); }

        void SetCursorIcon(CursorIcon cursor) { _wnd->SetCursorIcon(cursor); }
        // Shows/Hides the cursor until it is moved or this function is called
        void SetCursorVisibility(bool visible) { _wnd->SetCursorVisibility(visible); }
        // Resets screen shutoff timer
        void ResetScreenTimer() { _wnd->ResetScreenTimer(); }
        // Sets how the mouse interacts with the window
        void SetMouseInteraction(MouseWindowInteraction interactionType) { _wnd->SetMouseInteraction(interactionType); }

        void AddKeyboardHandler(KeyboardEventHandler* handler) { _wnd->AddKeyboardHandler(handler); }
        bool RemoveKeyboardHandler(KeyboardEventHandler* handler) { return _wnd->RemoveKeyboardHandler(handler); }
        void AddDragDropHandler(IDragDropEventHandler* handler) { _wnd->AddDragDropHandler(handler); }
        bool RemoveDragDropHandler(IDragDropEventHandler* handler) { return _wnd->RemoveDragDropHandler(handler); }
    private:
        WindowBackend* _wnd;
    };
}