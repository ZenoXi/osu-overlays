#pragma once

#include "KeyboardEventHandler.h"

class KeyboardManager : public KeyboardEventHandler
{
    std::vector<KeyboardEventHandler*> _handlers;
    KeyboardEventHandler* _exclusiveHandler = nullptr;

public:
    KeyboardManager() {}

    void AddHandler(KeyboardEventHandler* handler)
    {
        _handlers.push_back(handler);
    }

    void RemoveHandler(KeyboardEventHandler* handler)
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

    void SetExclusiveHandler(KeyboardEventHandler* handler)
    {
        _exclusiveHandler = handler;
    }

    void ResetExclusiveHandler()
    {
        _exclusiveHandler = nullptr;
    }

private:  // Keyboard event handling
    bool _OnHotkey(int id)
    {
        if (_exclusiveHandler)
        {
            if (EVENT_HANDLED(_exclusiveHandler->OnHotkey(id)))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            handler->OnHotkey(id);
        }
        return true;
    }

    bool _OnKeyDown(BYTE vkCode)
    {
        if (_exclusiveHandler)
        {
            if (EVENT_HANDLED(_exclusiveHandler->OnKeyDown(vkCode)))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            handler->OnKeyDown(vkCode);
        }
        return true;
    }

    bool _OnKeyUp(BYTE vkCode)
    {
        if (_exclusiveHandler)
        {
            if (EVENT_HANDLED(_exclusiveHandler->OnKeyUp(vkCode)))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            handler->OnKeyUp(vkCode);
        }
        return true;
    }

    bool _OnChar(wchar_t ch)
    {
        if (_exclusiveHandler)
        {
            if (EVENT_HANDLED(_exclusiveHandler->OnChar(ch)))
            {
                return true;
            }
        }

        for (auto& handler : _handlers)
        {
            handler->OnChar(ch);
        }
        return true;
    }
};