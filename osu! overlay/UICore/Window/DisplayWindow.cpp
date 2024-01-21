#include "DisplayWindow.h"

#include "D2DEffects/TintEffect.h"

#include <iostream>
#include <cassert>
#include <sstream>

#include <WinUser.h>
#pragma comment( lib,"User32.lib" )
#include <Shellapi.h>
#include <dwmapi.h>
#pragma comment( lib,"Dwmapi.lib" )
#pragma comment( lib,"UxTheme.lib" )
#include "GdiInclude.h"
namespace gdi = Gdiplus;
#pragma comment (lib, "gdiplus.lib")

//std::wstring DisplayWindow::WINDOW_NAME = L"UI Framework";
std::wstring DisplayWindow::WINDOW_NAME = L"";

#define EXEC_PTR_LOOP(container, expression) for (auto item : container) item->expression
#define EXEC_LOOP(container, expression) for (auto item : container) item.expression

using Microsoft::WRL::ComPtr;

DisplayWindow::DisplayWindow(HINSTANCE hInst, wchar_t* pArgs, LPCWSTR name) : _args(pArgs), _hInst(hInst), _wndClassName(name)
{
    OleInitialize(NULL);

    _cursor = LoadCursor(NULL, IDC_ARROW);
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        0/*CS_CLASSDC*/,
        _HandleMsgSetup,
        0,
        0,
        hInst,
        NULL,
        NULL,//_cursor,
        nullptr,
        nullptr,
        _wndClassName,
        NULL
    };
    
    RegisterClassEx(&wc);

    // Calculate initial window size
    RECT workRect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workRect, 0);
    int x = (workRect.right - width) / 2;
    int y = (workRect.bottom - height) / 2;
    int w = width;
    int h = height;

    // Set windowed rect size
    _windowedRect.left = x;
    _windowedRect.top = y;
    _windowedRect.right = x + w;
    _windowedRect.bottom = y + h;
    _last2Moves[0] = _windowedRect;
    _last2Moves[1] = _windowedRect;

    // Create and show window
    _hwnd = CreateWindowEx(
        NULL,
        _wndClassName,
        WINDOW_NAME.c_str(),
        //WS_OVERLAPPEDWINDOW,
        //WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        //WS_OVERLAPPED | WS_BORDER | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        WS_VISIBLE | WS_THICKFRAME | WS_SYSMENU,
        //WS_CAPTION,
        //WS_THICKFRAME,
        x, y, w, h,
        NULL,
        NULL,
        hInst,
        this
    );

    _fileDropHandler = std::make_unique<FileDropHandler>(_hwnd);
    HRESULT hr = RegisterDragDrop(_hwnd, _fileDropHandler.get());

    gfx.Initialize(&_hwnd);
    
    // Enable dark mode
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
    BOOL value = TRUE;
    //::DwmSetWindowAttribute(_hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    ShowWindow(_hwnd, SW_SHOWNORMAL);
    UpdateWindow(_hwnd);
}

DisplayWindow::~DisplayWindow()
{
    gfx.Close();

    RevokeDragDrop(_hwnd);
    CloseWindow(_hwnd);
    // unregister window class
    UnregisterClass(_wndClassName, _hInst);
}

bool DisplayWindow::ProcessMessages()
{
    MSG msg;
    bool msgProcessed = false;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        msgProcessed = true;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
        {
            exit(0);
        }
    }
    return msgProcessed;
}

void DisplayWindow::ProcessQueueMessages()
{
    std::lock_guard<std::mutex> lock(_m_msg);
    while (!_msgQueue.empty())
    {
        HandleMsgFromQueue(_msgQueue.front());
        _msgQueue.pop();
    }
}

WindowMessage DisplayWindow::GetSizeResult()
{
    _m_msg.lock();
    WindowMessage wm = _sizeResult;
    _sizeResult.handled = true;
    _sizeResult.lParam = 0;
    _sizeResult.wParam = 0;
    _m_msg.unlock();
    return wm;
}

WindowMessage DisplayWindow::GetMoveResult()
{
    _m_msg.lock();
    WindowMessage wm = _moveResult;
    _moveResult.handled = true;
    _moveResult.lParam = 0;
    _moveResult.wParam = 0;
    _m_msg.unlock();
    return wm;
}

WindowMessage DisplayWindow::GetExitResult()
{
    _m_msg.lock();
    WindowMessage wm = _exitResult;
    _exitResult.handled = true;
    _m_msg.unlock();
    return wm;
}

LRESULT WINAPI DisplayWindow::_HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
    if (msg == WM_NCCREATE)
    {
        // extract ptr to window class from creation data
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        DisplayWindow* const pWnd = reinterpret_cast<DisplayWindow*>(pCreate->lpCreateParams);
        // set WinAPI-managed user data to store ptr to window class
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // set message proc to normal (non-setup) handler now that setup is finished
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&DisplayWindow::_HandleMsgThunk));
        // forward message to window class handler
        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }
    // if we get a message before the WM_NCCREATE message, handle with default handler
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WINAPI DisplayWindow::_HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // retrieve ptr to window class
    DisplayWindow* const pWnd = reinterpret_cast<DisplayWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    // forward message to window class handler
    return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT DisplayWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    //if (msg != WM_MOUSEMOVE) std::cout << msg << std::endl;

    static RECT border_thickness;

    switch (msg)
    {
        case WM_DESTROY:
        {
            _m_msg.lock();
            _exitResult.handled = false;
            _m_msg.unlock();
            break;
        }
        case WM_CREATE:
        {
            // find border thickness
            SetRectEmpty(&border_thickness);
            if (GetWindowLongPtr(hWnd, GWL_STYLE) & WS_THICKFRAME)
            {
                AdjustWindowRectEx(&border_thickness, GetWindowLongPtr(hWnd, GWL_STYLE) & ~WS_CAPTION, FALSE, NULL);
                border_thickness.left *= -1;
                border_thickness.top *= -1;
            }
            else if (GetWindowLongPtr(hWnd, GWL_STYLE) & WS_BORDER)
            {
                SetRect(&border_thickness, 1, 1, 1, 1);
            }

            MARGINS margins = { 0 };
            DwmExtendFrameIntoClientArea(hWnd, &margins);
            SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);


            // Paint everything black
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOWTEXT));
            EndPaint(hWnd, &ps);

            break;
        }
        case WM_NCACTIVATE:
        {
            break;
        }
        case WM_NCCALCSIZE:
        {
            if (lParam)
            {
                NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)lParam;
                sz->rgrc[0].left += border_thickness.left;
                sz->rgrc[0].right -= border_thickness.right;
                sz->rgrc[0].bottom -= border_thickness.bottom;
                return 0;
            }
            break;
        }
        case WM_NCHITTEST:
        {
            //do default processing, but allow resizing from top-border
            LRESULT result = DefWindowProc(_hwnd, msg, wParam, lParam);
            if (result == HTCLIENT)
            {
                POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
                ScreenToClient(_hwnd, &pt);
                if (pt.y < border_thickness.top)
                    return HTTOP;
                if (pt.y < 30)
                    return HTCAPTION;
            }
            return result;
        }
        //case WM_ACTIVATE:
        //{
        //    MARGINS margins{ -1, -1, 1, -1 };
        //    HRESULT hr = DwmExtendFrameIntoClientArea(_hwnd, &margins);
        //    SetWindowPos(_hwnd, NULL, 0, 0, 0, 0,
        //        SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        //    break;
        //}
        //case WM_NCCALCSIZE:
        //{
        //    //if (!lParam)
        //    //    return DefWindowProc(hWnd, msg, wParam, lParam);
        //    //break;

        //    if (lParam)
        //    {
        //        NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)lParam;
        //        sz->rgrc[0].top += 30;
        //        sz->rgrc[0].left += 7;
        //        sz->rgrc[0].right -= 7;
        //        sz->rgrc[0].bottom -= 7;
        //        //return WVR_ALIGNBOTTOM;
        //        break;
        //    }
        //    return DefWindowProc(hWnd, msg, wParam, lParam);
        //}
        //case WM_NCHITTEST:
        //{
        //    int x = (short)LOWORD(lParam);
        //    int y = (short)HIWORD(lParam);
        //    break;
        //}
        //case WM_PAINT:
        //{
        //    PAINTSTRUCT ps;
        //    HDC hdc = BeginPaint(_hwnd, &ps);

        //    //paint caption area
        //    //paint_caption(hwnd, hdc, margins.cyTopHeight);
        //    gdi::Graphics g(hdc);
        //    gdi::SolidBrush brush(gdi::Color(255, 128, 128));
        //    g.FillRectangle(&brush, gdi::Rect(0, 0, 150, 150));

        //    EndPaint(_hwnd, &ps);
        //    return 0;
        //}
        //case WM_NCPAINT:
        //{
        //    HDC hdc;
        //    hdc = GetDCEx(_hwnd, (HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);

        //    //HBRUSH brush = CreateSolidBrush(RGBA(50, 151, 151, 0));

        //    //FillRect(hdc, )
        //    gdi::Graphics g(hdc);
        //    g.Clear(gdi::Color(128, 128, 128));

        //    //DeleteObject(brush);
        //    ReleaseDC(_hwnd, hdc);
        //    break;
        //}
        case WM_MOUSEMOVE:
        {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            if (x != _lastMouseMove.x || y != _lastMouseMove.y)
            {
                _lastMouseMove.x = x;
                _lastMouseMove.y = y;
            }
            else
            {
                break;
            }
            _m_msg.lock();

            if (x >= 0 && x < width && y >= 0 && y < height)
            {
                if (!_mouseInside)
                {
                    TRACKMOUSEEVENT ev;
                    ev.cbSize = sizeof(TRACKMOUSEEVENT);
                    ev.dwFlags = TME_LEAVE;
                    ev.hwndTrack = _hwnd;
                    TrackMouseEvent(&ev);
                    _prevCursor = GetCursor();
                }
            }
            else
            {
                if (wParam & (MK_LBUTTON | MK_RBUTTON))
                {

                }
                else
                {
                    SetCursor(_prevCursor);
                }
            }
            _msgQueue.push({ false, WM_MOUSEMOVE, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_MOUSELEAVE:
        {
            _mouseInside = false;
            _m_msg.lock();
            _msgQueue.push({ false, WM_MOUSEMOVE, wParam, MAKELPARAM(-1, -1) });
            _m_msg.unlock();
            break;
        }
        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) != HTCLIENT)
                return DefWindowProc(hWnd, msg, wParam, lParam);

            _m_msg.lock();
            SetCursor(_cursor);
            _m_msg.unlock();
            break;
        }
        case WM_LBUTTONDOWN:
        {
            SetCapture(_hwnd);
            SetWindowPos(_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

            _m_msg.lock();
            _msgQueue.push({ false, WM_LBUTTONDOWN, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_RBUTTONDOWN:
        {
            _m_msg.lock();
            _msgQueue.push({ false, WM_RBUTTONDOWN, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            _m_msg.lock();
            _msgQueue.push({ false, WM_LBUTTONUP, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_RBUTTONUP:
        {
            _m_msg.lock();
            _msgQueue.push({ false, WM_RBUTTONUP, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_MOUSEWHEEL:
        {
            _m_msg.lock();
            _msgQueue.push({ false, WM_MOUSEWHEEL, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_MOVE:
        {
            if (!_fullscreen)
            {
                int x = (int)(short)LOWORD(lParam);
                int y = (int)(short)HIWORD(lParam);
                int w = _windowedRect.right - _windowedRect.left;
                int h = _windowedRect.bottom - _windowedRect.top;
                _last2Moves[0] = _last2Moves[1];
                _last2Moves[1].left = x;
                _last2Moves[1].top = y;
                _last2Moves[1].right = x + w;
                _last2Moves[1].bottom = y + h;
            }

            _m_msg.lock();
            _msgQueue.push({ false, WM_MOVE, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* info = (MINMAXINFO*)lParam;
            info->ptMinTrackSize.x = 900;
            info->ptMinTrackSize.y = 600;
            break;
        }
        case WM_SIZE:
        {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if (w == width && h == height) break;

            if (wParam == SIZE_RESTORED && !_fullscreen)
            {
                _last2Moves[0] = _last2Moves[1];
                GetWindowRect(_hwnd, &_last2Moves[1]);
                GetWindowRect(_hwnd, &_windowedRect);
            }
            if (wParam == SIZE_MAXIMIZED)
            {
                _last2Moves[1] = _last2Moves[0];
            }
            if (wParam == SIZE_MINIMIZED)
            {
                _last2Moves[1] = _last2Moves[0];
            }

            if (gfx.Initialized())
                gfx.ResizeBuffers(w, h);
            width = w;
            height = h;

            _m_msg.lock();
            _msgQueue.push({ false, WM_SIZE, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_KEYDOWN:
        {
            _m_msg.lock();
            _msgQueue.push({ false, WM_KEYDOWN, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_KEYUP:
        {
            _m_msg.lock();
            _msgQueue.push({ false, WM_KEYUP, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_SYSKEYDOWN:
        {
            // Translate to a regular key down message
            bool contextCode = (lParam & KF_ALTDOWN) == KF_ALTDOWN;
            if (!contextCode)
            {
                _m_msg.lock();
                _msgQueue.push({ false, WM_KEYDOWN, wParam, lParam });
                _m_msg.unlock();
            }
            else
            {
                // null context code
                lParam &= ~KF_ALTDOWN;

                _m_msg.lock();
                _msgQueue.push({ false, WM_KEYDOWN, VK_MENU, lParam });
                _msgQueue.push({ false, WM_KEYDOWN, wParam, lParam });
                _m_msg.unlock();
            }

            _m_msg.lock();
            _msgQueue.push({ false, WM_SYSKEYDOWN, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_SYSKEYUP:
        {
            // Translate to a regular key up message
            bool contextCode = (lParam & KF_ALTDOWN) == KF_ALTDOWN;
            if (!contextCode)
            {
                _m_msg.lock();
                _msgQueue.push({ false, WM_KEYUP, wParam, lParam });
                _m_msg.unlock();
            }
            else
            {
                // null context code
                lParam &= ~KF_ALTDOWN;

                _m_msg.lock();
                _msgQueue.push({ false, WM_KEYUP, VK_MENU, lParam });
                _msgQueue.push({ false, WM_KEYUP, wParam, lParam });
                _m_msg.unlock();
            }

            _m_msg.lock();
            _msgQueue.push({ false, WM_SYSKEYUP, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        case WM_CHAR:
        {
            _m_msg.lock();
            _msgQueue.push({ false, WM_CHAR, wParam, lParam });
            _m_msg.unlock();
            break;
        }
        default:
        {
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }

    return 0;
}

void DisplayWindow::HandleMsgFromQueue(WindowMessage msg)
{
    switch (msg.msg)
    {
        case WM_MOUSEMOVE:
        {
            int x = (short)LOWORD(msg.lParam);
            int y = (short)HIWORD(msg.lParam);
            if (x >= 0 && x < width && y >= 0 && y < height)
            {
                if (!_mouseInside)
                {
                    _mouseInside = true;
                    for (auto h : _mouseHandlers) h->OnMouseEnter();
                }
                for (auto h : _mouseHandlers) h->OnMouseMove(x, y);
            }
            else
            {
                if (msg.wParam & (MK_LBUTTON | MK_RBUTTON))
                {
                    for (auto h : _mouseHandlers) h->OnMouseMove(x, y);
                }
                else
                {
                    _mouseInside = false;
                    for (auto h : _mouseHandlers)
                    {
                        h->OnLeftReleased(x, y);
                        h->OnRightReleased(x, y);
                        h->OnMouseLeave();
                    }
                }
            }
            break;
        }
        case WM_LBUTTONDOWN:
        {
            int x = LOWORD(msg.lParam);
            int y = HIWORD(msg.lParam);
            for (auto h : _mouseHandlers) h->OnLeftPressed(x, y);
            break;
        }
        case WM_RBUTTONDOWN:
        {
            int x = LOWORD(msg.lParam);
            int y = HIWORD(msg.lParam);
            for (auto h : _mouseHandlers) h->OnRightPressed(x, y);
            break;
        }
        case WM_LBUTTONUP:
        {
            int x = LOWORD(msg.lParam);
            int y = HIWORD(msg.lParam);
            for (auto h : _mouseHandlers) h->OnLeftReleased(x, y);
            break;
        }
        case WM_RBUTTONUP:
        {
            int x = LOWORD(msg.lParam);
            int y = HIWORD(msg.lParam);
            for (auto h : _mouseHandlers) h->OnRightReleased(x, y);
            break;
        }
        case WM_MOUSEWHEEL:
        {
            int x = LOWORD(msg.lParam);
            int y = HIWORD(msg.lParam);
            if (GET_WHEEL_DELTA_WPARAM(msg.wParam) > 0)
            {
                for (auto h : _mouseHandlers) h->OnWheelUp(x, y);
            }
            else if (GET_WHEEL_DELTA_WPARAM(msg.wParam) < 0)
            {
                for (auto h : _mouseHandlers) h->OnWheelDown(x, y);
            }
            break;
        }
        case WM_KEYDOWN:
        {
            for (auto& h : _keyboardHandlers) h->OnKeyDown(msg.wParam);
            break;
        }
        case WM_KEYUP:
        {
            for (auto& h : _keyboardHandlers) h->OnKeyUp(msg.wParam);
            break;
        }
        case WM_CHAR:
        {
            for (auto& h : _keyboardHandlers) h->OnChar(msg.wParam);
            break;
        }
        case WM_MOVE:
        {
            _moveResult.handled = false;
            _moveResult.msg = WM_MOVE;
            _moveResult.lParam = msg.lParam;
            _moveResult.wParam = msg.wParam;
            break;
        }
        case WM_SIZE:
        {
            _sizeResult.handled = false;
            _sizeResult.msg = WM_SIZE;
            _sizeResult.lParam = msg.lParam;
            _sizeResult.wParam = msg.wParam;
            break;
        }
    }
}

void DisplayWindow::HandleFullscreenChange()
{
    std::lock_guard<std::mutex> lock(_m_fullscreen);
    if (_fullscreenChanged)
    {
        _fullscreenChanged = false;

        if (_fullscreen)
        {
            WINDOWPLACEMENT placement;
            placement.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(_hwnd, &placement);
            _windowedMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);

            HMONITOR hMonitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO info;
            info.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(hMonitor, &info);
            RECT monitor = info.rcMonitor;

            int w = monitor.right - monitor.left;
            int h = monitor.bottom - monitor.top;
            SetWindowLongPtr(_hwnd, GWL_STYLE, /*WS_VISIBLE |*/ WS_POPUP);
            SetWindowPos(_hwnd, HWND_TOP, monitor.left, monitor.top, w, h, SWP_FRAMECHANGED);

            ShowWindow(_hwnd, SW_SHOW);
        }
        else
        {
            //int x = _windowedRect.left;
            //int y = _windowedRect.top;
            //int w = _windowedRect.right - _windowedRect.left;
            //int h = _windowedRect.bottom - _windowedRect.top;
            int x = _last2Moves[1].left;
            int y = _last2Moves[1].top;
            int w = _last2Moves[1].right - _last2Moves[1].left;
            int h = _last2Moves[1].bottom - _last2Moves[1].top;
            SetWindowLongPtr(_hwnd, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);
            SetWindowPos(_hwnd, NULL, x, y, w, h, SWP_FRAMECHANGED);

            if (_windowedMaximized)
                ShowWindow(_hwnd, SW_SHOWMAXIMIZED);
            else
                ShowWindow(_hwnd, SW_SHOW);
        }
        //UpdateWindow(_hwnd);
    }
}

void DisplayWindow::SetCursorIcon(CursorIcon cursor)
{
    switch (cursor)
    {
    case CursorIcon::APP_STARTING:
    {
        _cursor = LoadCursor(NULL, IDC_APPSTARTING);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::ARROW:
    {
        _cursor = LoadCursor(NULL, IDC_ARROW);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::CROSS:
    {
        _cursor = LoadCursor(NULL, IDC_CROSS);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::HAND:
    {
        _cursor = LoadCursor(NULL, IDC_HAND);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::HELP:
    {
        _cursor = LoadCursor(NULL, IDC_HELP);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::IBEAM:
    {
        _cursor = LoadCursor(NULL, IDC_IBEAM);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::NO:
    {
        _cursor = LoadCursor(NULL, IDC_NO);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::SIZE_ALL:
    {
        _cursor = LoadCursor(NULL, IDC_SIZEALL);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::SIZE_NESW:
    {
        _cursor = LoadCursor(NULL, IDC_SIZENESW);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::SIZE_NS:
    {
        _cursor = LoadCursor(NULL, IDC_SIZENS);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::SIZE_NWSE:
    {
        _cursor = LoadCursor(NULL, IDC_SIZENWSE);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::SIZE_WE:
    {
        _cursor = LoadCursor(NULL, IDC_SIZEWE);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::UP_ARROW:
    {
        _cursor = LoadCursor(NULL, IDC_UPARROW);
        //SetCursor(_cursor);
        break;
    }
    case CursorIcon::WAIT:
    {
        _cursor = LoadCursor(NULL, IDC_WAIT);
        //SetCursor(_cursor);
        break;
    }
    default:
        break;
    }
}

void DisplayWindow::SetCursorVisibility(bool visible)
{
    if (visible != _cursorVisible)
    {
        _cursorVisible = visible;
        _cursorVisibilityChanged = true;
    }
}

void DisplayWindow::HandleCursorVisibility()
{
    if (_cursorVisibilityChanged)
    {
        _cursorVisibilityChanged = false;

        if (!_cursorVisible)
        {
            int value;
            while ((value = ShowCursor(false)) > -1);
            while (value < -1) value = ShowCursor(true);
        }
        else
        {
            int value;
            while ((value = ShowCursor(true)) < 1);
            while (value > 1) value = ShowCursor(false);
        }
    }
}

void DisplayWindow::ResetScreenTimer()
{
    SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
}

void DisplayWindow::SetHotkey(int id, UINT modifiers, UINT keyCode)
{
    if (_hotkeys.find(id) != _hotkeys.end())
        UnregisterHotKey(_hwnd, id);

    RegisterHotKey(_hwnd, id, modifiers, keyCode);
    _hotkeys[id] = Hotkey{ modifiers, keyCode };
}

void DisplayWindow::AddMouseHandler(MouseEventHandler* handler)
{
    _mouseHandlers.push_back(handler);
}

bool DisplayWindow::RemoveMouseHandler(MouseEventHandler* handler)
{
    for (auto it = _mouseHandlers.begin(); it != _mouseHandlers.end(); it++)
    {
        if (*it == handler)
        {
            _mouseHandlers.erase(it);
            return true;
        }
    }
    return false;
}

void DisplayWindow::AddKeyboardHandler(KeyboardEventHandler* handler)
{
    _keyboardHandlers.push_back(handler);
}

bool DisplayWindow::RemoveKeyboardHandler(KeyboardEventHandler* handler)
{
    for (auto it = _keyboardHandlers.begin(); it != _keyboardHandlers.end(); it++)
    {
        if (*it == handler)
        {
            _keyboardHandlers.erase(it);
            return true;
        }
    }
    return false;
}

void DisplayWindow::AddDragDropHandler(IDragDropEventHandler* handler)
{
    _fileDropHandler->AddDragDropEventHandler(handler);
}

bool DisplayWindow::RemoveDragDropHandler(IDragDropEventHandler* handler)
{
    return _fileDropHandler->RemoveDragDropEventHandler(handler);
}

void DisplayWindow::SetFullscreen(bool fullscreen)
{
    std::lock_guard<std::mutex> lock(_m_fullscreen);
    if (_fullscreen == fullscreen) return;
    _fullscreenChanged = true;
    _fullscreen = fullscreen;
}

bool DisplayWindow::GetFullscreen()
{
    return _fullscreen;
}

void WindowGraphics::Initialize(HWND* hwnd_t)
{
    p_hwnd = hwnd_t;
    HRESULT hr;
    
    // Create DirectWrite factory
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&p_DWriteFactory)
    );

    p_DWriteFactory->CreateTextFormat(
        L"Calibri",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        12.0f,
        L"en_us",
        &p_DebugTextFormat
    );

    // Obtain the size of the drawing area.
    GetClientRect(*p_hwnd, &_windowRect);

    D2D1CreateFactory(
        D2D1_FACTORY_TYPE_MULTI_THREADED,
        {},
        p_D2DFactory.GetAddressOf()
    );

    // Register custom effects
    hr = CustomTintEffect::Register(p_D2DFactory.Get());

    D3D11CreateDevice(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0,
        D3D11_SDK_VERSION,
        p_Device.GetAddressOf(),
        nullptr,
        nullptr
    );

    HR(p_Device.As(&p_DXGIDevice));
    HR(p_DXGIDevice->GetAdapter(p_DXGIAdapter.GetAddressOf()));
    HR(p_DXGIAdapter->GetParent(__uuidof(p_DXGIFactory), reinterpret_cast<void**>(p_DXGIFactory.GetAddressOf())));

    // Swap chain stuff
    DXGI_SWAP_CHAIN_DESC1 scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));

    scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 1;

    HR(p_DXGIFactory->CreateSwapChainForHwnd(
        p_Device.Get(),
        *p_hwnd,
        &scd,
        nullptr,
        nullptr,
        p_SwapChain.GetAddressOf()
    ));

    HR(p_D2DFactory->CreateDevice(p_DXGIDevice.Get(), p_D2DDevice.GetAddressOf()));
    HR(p_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, p_Target.GetAddressOf()));
    HR(p_SwapChain->GetBuffer(
        0, // buffer index
        __uuidof(p_Surface),
        reinterpret_cast<void**>(p_Surface.GetAddressOf())
    ));
    auto props = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
    );
    HR(p_Target->CreateBitmapFromDxgiSurface(p_Surface.Get(),
        props,
        p_Bitmap.GetAddressOf()
    ));
    p_Target->SetTarget(p_Bitmap.Get());

    _initialized = true;
}

void WindowGraphics::Close()
{
    p_SwapChain->SetFullscreenState(FALSE, NULL);

    // Release all references
    for (int i = 0; i < _references.size(); i++)
    {
        if (*_references[i].first)
        {
            (*_references[i].first)->Release();
            (*_references[i].first) = nullptr;
        }
    }
    _references.clear();

    // Release all objects
    p_Bitmap.Reset();
    p_Bitmap.Reset();
    p_Surface.Reset();
    p_Target.Reset();
    p_D2DDevice.Reset();
    p_SwapChain.Reset();
    p_DXGIAdapter.Reset();
    p_DXGIDevice.Reset();
    p_Device.Reset();
    p_D2DFactory.Reset();

    p_DebugTextFormat->Release();
    p_DWriteFactory->Release();
}

void WindowGraphics::BeginFrame()
{

}

void WindowGraphics::EndFrame(bool swap)
{
    //p_Target->EndDraw();
    if (swap)
        p_SwapChain->Present(1, 0);
    else
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void WindowGraphics::SetFullscreen(bool f)
{
    p_SwapChain->SetFullscreenState(f, NULL);
}

void WindowGraphics::ResizeBuffers(int width, int height)
{
    Lock();

    // Release all references
    for (int i = 0; i < _references.size(); i++)
    {
        if (*_references[i].first)
        {
            (*_references[i].first)->Release();
            (*_references[i].first) = nullptr;
        }
    }
    _references.clear();
    p_Bitmap->Release();
    p_Surface->Release();
    p_Target->Release();

    // Resize swapchain
    HR(p_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0/*DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH*/));

    // Recreate target reference
    HR(p_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, p_Target.GetAddressOf()));
    HR(p_SwapChain->GetBuffer(
        0, // buffer index
        __uuidof(p_Surface),
        reinterpret_cast<void**>(p_Surface.GetAddressOf())
    ));
    auto props = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
    );
    HR(p_Target->CreateBitmapFromDxgiSurface(p_Surface.Get(),
        props,
        p_Bitmap.GetAddressOf()
    ));
    p_Target->SetTarget(p_Bitmap.Get());

    Unlock();
}

void WindowGraphics::ReleaseResource(IUnknown** res)
{
    // Release resource
    if (*res)
    {
        (*res)->Release();
        *res = nullptr;
    }

    // Remove pointer from vector
    for (auto it = _references.begin(); it != _references.end(); it++)
    {
        if ((*it).first == res)
        {
            _references.erase(it);
            break;
        }
    }
}

void WindowGraphics::Lock()
{
    _m_gfx.lock();
}

void WindowGraphics::Unlock()
{
    _m_gfx.unlock();
}

Graphics WindowGraphics::GetGraphics()
{
    return { p_Target.Get(), p_D2DFactory.Get(), &_references };
}