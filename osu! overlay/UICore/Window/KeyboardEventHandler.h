#pragma once

#include <vector>
#include <string>

#include "WindowsEx.h"
#include "Helper/EventEmitter.h"

#define _KEY_COUNT 256
// Used for clarity and consistency
#define EVENT_HANDLED(expr) expr == true

enum KeyModifiers
{
    // Values taken from Microsoft documentation for 'RegisterHotKey()'
    KMOD_ALT = 0x0001,
    KMOD_CONTROL = 0x0002,
    KMOD_NOREPEAT = 0x4000,
    KMOD_SHIFT = 0x0004
};

class KeyboardEventHandler
{
protected:
    BYTE _keyStates[_KEY_COUNT]{ 0 };

    virtual bool _OnHotkey(int id) { return false; };
    virtual bool _OnKeyDown(BYTE vkCode) { return false; };
    virtual bool _OnKeyUp(BYTE vkCode) { return false; };
    virtual bool _OnChar(wchar_t ch) { return false; };
    
    EventEmitter<bool, int> _onHotkeyHandlers;
    EventEmitter<bool, BYTE> _onKeyDownHandlers;
    EventEmitter<bool, BYTE> _onKeyUpHandlers;
    EventEmitter<bool, wchar_t> _onCharHandlers;

public:
    KeyboardEventHandler() {}

    bool OnHotkey(int id)
    {
        bool handled = _onHotkeyHandlers->InvokeAll([](bool result) { return EVENT_HANDLED(result); }, id);
        if (handled)
            return true;

        return _OnHotkey(id);
    }

    bool OnKeyDown(BYTE vkCode)
    {
        _keyStates[vkCode] = 0x80;
        bool handled = _onKeyDownHandlers->InvokeAll([](bool result) { return EVENT_HANDLED(result); }, vkCode);
        if (handled)
            return true;

        return _OnKeyDown(vkCode);
    }

    bool OnKeyUp(BYTE vkCode)
    {
        _keyStates[vkCode] = 0;
        bool handled = _onKeyUpHandlers->InvokeAll([](bool result) { return EVENT_HANDLED(result); }, vkCode);
        if (handled)
            return true;

        return _OnKeyUp(vkCode);
    }

    bool OnChar(wchar_t ch)
    {
        bool handled = _onCharHandlers->InvokeAll([](bool result) { return EVENT_HANDLED(result); }, ch);
        if (handled)
            return true;

        return _OnChar(ch);
    }

    EventSubscription<bool, int> SubscribeOnHotkey(const std::function<bool(int)>& func)
    {
        return _onHotkeyHandlers->Subscribe(func);
    }

    EventSubscription<bool, BYTE> SubscribeOnKeyDown(const std::function<bool(BYTE)>& func)
    {
        return _onKeyDownHandlers->Subscribe(func);
    }

    EventSubscription<bool, BYTE> SubscribeOnKeyUp(const std::function<bool(BYTE)>& func)
    {
        return _onKeyUpHandlers->Subscribe(func);
    }

    EventSubscription<bool, wchar_t> SubscribeOnChar(const std::function<bool(wchar_t)>& func)
    {
        return _onCharHandlers->Subscribe(func);
    }

    // 'modifiers' specify which modifiers from 'KeyModifiers' enum must be pressed
    // for the key state to return true. If 'exact' is true, then ONLY the specified
    // modifiers need to be pressed.
    bool KeyState(int vkCode, int modifiers = 0, bool exact = false)
    {
        if (!(_keyStates[vkCode] & 0x80))
            return false;
        if (modifiers == 0 && !exact)
            return true;

        bool altNeeded = modifiers & KMOD_ALT;
        bool altState = _keyStates[VK_MENU] & 0x80;
        bool ctrlNeeded = modifiers & KMOD_CONTROL;
        bool ctrlState = _keyStates[VK_CONTROL] & 0x80;
        bool shiftNeeded = modifiers & KMOD_SHIFT;
        bool shiftState = _keyStates[VK_SHIFT] & 0x80;

        if ((altNeeded && !altState) || (exact && !altNeeded && altState))
            return false;
        if ((ctrlNeeded && !ctrlState) || (exact && !ctrlNeeded && ctrlState))
            return false;
        if ((shiftNeeded && !shiftState) || (exact && !shiftNeeded && shiftState))
            return false;
        return true;
    }

    BYTE* KeyStates()
    {
        return _keyStates;
    }
};