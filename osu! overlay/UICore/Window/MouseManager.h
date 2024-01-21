#pragma once

#include "MouseEventHandler.h"

#include <vector>

class MouseManager : public MouseEventHandler
{
    std::vector<MouseEventHandler*> _handlers;
    MouseEventHandler* _overlayHandler = nullptr;
    MouseEventHandler* _topMenuHandler = nullptr;
    bool _topMenuVisible = false;
    int _topMenuHeight = 1;

public:
    MouseManager() {};

    void AddHandler(MouseEventHandler* handler)
    {
        _handlers.push_back(handler);
    }

    void RemoveHandler(MouseEventHandler* handler)
    {
        for (auto it = _handlers.begin(); it != _handlers.end(); it++)
        {
            if (*it == handler)
            {
                _handlers.erase(it);
                break;
            }
        }
    }

    void SetOverlayHandler(MouseEventHandler* handler)
    {
        _overlayHandler = handler;
    }

    void SetTopMenuHandler(MouseEventHandler* handler)
    {
        _topMenuHandler = handler;
        _topMenuVisible = true;
    }

    void SetTopMenuVisibility(bool visible)
    {
        _topMenuVisible = visible;
    }

    void SetTopMenuHeight(int height)
    {
        _topMenuHeight = height;
    }

private: // Mouse event handling
    bool _OnMouseMove(int x, int y)
    {
        if (_topMenuVisible)
        {
            y -= _topMenuHeight;
            _topMenuHandler->OnMouseMove(x, y + _topMenuHeight);
        }

        if (_overlayHandler)
        {
            bool clicked = false;
            for (auto& handler : _handlers)
            {
                if (handler->MouseLeftClicked()) { clicked = true; break; }
                if (handler->MouseRightClicked()) { clicked = true; break; }
            }
            if (!clicked)
            {
                if (_overlayHandler->OnMouseMove(x, y))
                {
                    for (auto& handler : _handlers)
                        handler->OnMouseLeave();
                    return true;
                }
            }
        }

        for (auto& handler : _handlers)
        {
            if (handler->OnMouseMove(x, y))
                return true;
        }
        return false;
    }

    void _OnMouseLeave()
    {
        if (_topMenuHandler)
            _topMenuHandler->OnMouseLeave();
        if (_overlayHandler)
            _overlayHandler->OnMouseLeave();

        for (auto& handler : _handlers)
        {
            handler->OnMouseLeave();
        }
    }

    void _OnMouseEnter()
    {
        if (_topMenuHandler)
            _topMenuHandler->OnMouseEnter();
        if (_overlayHandler)
            _overlayHandler->OnMouseEnter();

        for (auto& handler : _handlers)
        {
            handler->OnMouseEnter();
        }
    }

    bool _OnLeftPressed(int x, int y)
    {
        if (_topMenuVisible)
        {
            y -= _topMenuHeight;
            _topMenuHandler->OnLeftPressed(x, y + _topMenuHeight);
        }

        if (_overlayHandler)
        {
            if (_overlayHandler->OnLeftPressed(x, y))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            if (handler->OnLeftPressed(x, y))
                return true;
        }
        return false;
    }

    bool _OnLeftReleased(int x, int y)
    {
        if (_topMenuVisible)
        {
            y -= _topMenuHeight;
            _topMenuHandler->OnLeftReleased(x, y + _topMenuHeight);
        }

        if (_overlayHandler)
        {
            if (_overlayHandler->OnLeftReleased(x, y))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            if (handler->OnLeftReleased(x, y))
                return true;
        }
        return false;
    }

    bool _OnRightPressed(int x, int y)
    {
        if (_topMenuVisible)
        {
            y -= _topMenuHeight;
            _topMenuHandler->OnRightPressed(x, y + _topMenuHeight);
        }

        if (_overlayHandler)
        {
            if (_overlayHandler->OnRightPressed(x, y))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            if (handler->OnRightPressed(x, y))
                return true;
        }
        return false;
    }

    bool _OnRightReleased(int x, int y)
    {
        if (_topMenuVisible)
        {
            y -= _topMenuHeight;
            _topMenuHandler->OnRightReleased(x, y + _topMenuHeight);
        }

        if (_overlayHandler)
        {
            if (_overlayHandler->OnRightReleased(x, y))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            if (handler->OnRightReleased(x, y))
                return true;
        }
        return false;
    }

    bool _OnWheelUp(int x, int y)
    {
        if (_topMenuVisible)
        {
            y -= _topMenuHeight;
            _topMenuHandler->OnWheelUp(x, y + _topMenuHeight);
        }

        if (_overlayHandler)
        {
            if (_overlayHandler->OnWheelUp(x, y))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            if (handler->OnWheelUp(x, y))
                return true;
        }
        return false;
    }

    bool _OnWheelDown(int x, int y)
    {
        if (_topMenuVisible)
        {
            y -= _topMenuHeight;
            _topMenuHandler->OnWheelDown(x, y + _topMenuHeight);
        }

        if (_overlayHandler)
        {
            if (_overlayHandler->OnWheelDown(x, y))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            if (handler->OnWheelDown(x, y))
                return true;
        }
        return false;
    }
};