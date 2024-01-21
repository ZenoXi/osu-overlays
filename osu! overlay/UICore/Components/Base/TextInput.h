#pragma once

#include "App.h"
#include "Panel.h"
#include "Label.h"
#include "Window/KeyboardEventHandler.h"

#include "Helper/Time.h"
#include "Helper/StringHelper.h"

#include <regex>

namespace zcom
{
    class TextInput : public Panel, public KeyboardEventHandler
    {
        // Used to select when the match pattern is enforced
        enum class MatchEnforcing
        {
            // Prevents non matching text from being typed altogether
            IMMEDIATE,
            // If the entered text does not match the pattern when the
            // input is deselected, the contents are reverted
            ON_DESELECT
        };

#pragma region base_class
    protected:
        void _OnUpdate()
        {
            Panel::_OnUpdate();

            // Check if cursor is not out of bounds
            // This cound happen if the label text is changed directly
            if (_cursorPos > _textLabel->GetText().length())
                _cursorPos = _textLabel->GetText().length();

            // Update text visibility
            if (_textLabel->GetText().empty())
            {
                _textLabel->SetVisible(false);
                _placeholderTextLabel->SetVisible(true);
            }
            else
            {
                _textLabel->SetVisible(true);
                _placeholderTextLabel->SetVisible(false);
            }

            // Update caret visibility
            if (!Selected())
            {
                if (_caretVisible)
                {
                    _caretVisible = false;
                    InvokeRedraw();
                }
            }
            else
            {
                _caretTimer.Update();
                if (_caretTimer.Now().GetTime(MILLISECONDS) % 1000 < 500)
                {
                    if (!_caretVisible)
                    {
                        _caretVisible = true;
                        InvokeRedraw();
                    }
                }
                else if (_caretVisible)
                {
                    _caretVisible = false;
                    InvokeRedraw();
                }
            }

            // Scroll
            if (GetMouseLeftClicked())
            {
                int newX = _textLabel->GetHorizontalOffsetPixels();
                int newY = _textLabel->GetVerticalOffsetPixels();

                if (GetMousePosX() < _textArea.left)
                {
                    float offBoundsAmount = (_textArea.left - GetMousePosX()) / 200.0f;
                    if (offBoundsAmount > 1.0f)
                        offBoundsAmount = 1.0f;
                    if (offBoundsAmount < 0.0f)
                        offBoundsAmount = 0.0f;
                    offBoundsAmount = std::powf(std::sinf(offBoundsAmount * 1.5708f), 0.1f);
                    //offBoundsAmount = std::powf(offBoundsAmount, 0.2f);
                    int scrollInterval = 100 - 100.0f * offBoundsAmount;
                    if (scrollInterval < 1)
                        scrollInterval = 1;
                    while (ztime::Main() - _lastHorizontalScroll > Duration(scrollInterval, MILLISECONDS))
                    {
                        _lastHorizontalScroll += Duration(scrollInterval, MILLISECONDS);
                        newX++;
                    }
                }
                else if (GetMousePosX() > _textArea.right)
                {
                    float offBoundsAmount = (GetMousePosX() - _textArea.right) / 200.0f;
                    if (offBoundsAmount > 1.0f)
                        offBoundsAmount = 1.0f;
                    if (offBoundsAmount < 0.0f)
                        offBoundsAmount = 0.0f;
                    offBoundsAmount = std::powf(std::sinf(offBoundsAmount * 1.5708f), 0.1f);
                    //offBoundsAmount = std::powf(offBoundsAmount, 0.2f);
                    int scrollInterval = 100 - 100.0f * offBoundsAmount;
                    if (scrollInterval < 1)
                        scrollInterval = 1;
                    while (ztime::Main() - _lastHorizontalScroll > Duration(scrollInterval, MILLISECONDS))
                    {
                        _lastHorizontalScroll += Duration(scrollInterval, MILLISECONDS);
                        newX--;
                    }
                }
                else
                {
                    _lastHorizontalScroll = ztime::Main();
                }

                if (GetMousePosY() < _textArea.top)
                {
                    float offBoundsAmount = (_textArea.top - GetMousePosY()) / 200.0f;
                    if (offBoundsAmount > 1.0f)
                        offBoundsAmount = 1.0f;
                    if (offBoundsAmount < 0.0f)
                        offBoundsAmount = 0.0f;
                    offBoundsAmount = std::powf(std::sinf(offBoundsAmount * 1.5708f), 0.1f);
                    //offBoundsAmount = std::powf(offBoundsAmount, 0.2f);
                    int scrollInterval = 100 - 100.0f * offBoundsAmount;
                    if (scrollInterval < 1)
                        scrollInterval = 1;
                    while (ztime::Main() - _lastVerticalScroll > Duration(scrollInterval, MILLISECONDS))
                    {
                        _lastVerticalScroll += Duration(scrollInterval, MILLISECONDS);
                        newY++;
                    }
                }
                else if (GetMousePosY() > _textArea.bottom)
                {
                    float offBoundsAmount = (GetMousePosY() - _textArea.bottom) / 200.0f;
                    if (offBoundsAmount > 1.0f)
                        offBoundsAmount = 1.0f;
                    if (offBoundsAmount < 0.0f)
                        offBoundsAmount = 0.0f;
                    offBoundsAmount = std::powf(std::sinf(offBoundsAmount * 1.5708f), 0.1f);
                    //offBoundsAmount = std::powf(offBoundsAmount, 0.2f);
                    int scrollInterval = 100 - 100.0f * offBoundsAmount;
                    if (scrollInterval < 1)
                        scrollInterval = 1;
                    while (ztime::Main() - _lastVerticalScroll > Duration(scrollInterval, MILLISECONDS))
                    {
                        _lastVerticalScroll += Duration(scrollInterval, MILLISECONDS);
                        newY--;
                    }
                }
                else
                {
                    _lastVerticalScroll = ztime::Main();
                }

                // Check if new positions are out of bounds
                if (newX + _textLabel->GetBaseWidth() <= _textArea.right)
                    newX = _textArea.right - _textLabel->GetBaseWidth();
                else if (newX > _textArea.left)
                    newX = _textArea.left;
                if (newY + _textLabel->GetBaseHeight() <= _textArea.bottom)
                    newY = _textArea.bottom - _textLabel->GetBaseHeight();
                else if (newY > _textArea.top)
                    newY = _textArea.top;

                _textLabel->SetHorizontalOffsetPixels(newX);
                _textLabel->SetVerticalOffsetPixels(newY);
            }
            else
            {
                _lastHorizontalScroll = ztime::Main();
                _lastVerticalScroll = ztime::Main();
            }
        }

        void _OnDraw(Graphics g)
        {
            Panel::_OnDraw(g);

            // Draw caret
            if (_caretVisible)
            {
                auto textMetrics = _textLabel->TextMetrics();
                auto metrics = _textLabel->HitTestTextPosition(_cursorPos);

                D2D1_RECT_F caretRect;
                caretRect.left = metrics.posX + _textLabel->GetHorizontalOffsetPixels();
                caretRect.right = caretRect.left + 2.0f;
                caretRect.top = metrics.posY + _textLabel->GetVerticalOffsetPixels();
                caretRect.bottom = caretRect.top + metrics.hitMetrics.height;

                ID2D1SolidColorBrush* brush = nullptr;
                g.target->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 0.9f), &brush);
                g.target->FillRectangle(caretRect, brush);
                brush->Release();
            }
        }

        void _OnResize(int width, int height)
        {
            Panel::_OnResize(width, height);

            _UpdateTextArea();
            _UpdateLabelPlacement();
        }

        EventTargets _OnLeftPressed(int x, int y)
        {
            Panel::_OnLeftPressed(x, y);

            // Get click position and place cursor there
            auto result = _textLabel->HitTestPoint((float)(x - _textLabel->GetX()), (float)(y - _textLabel->GetY()));
            int position = result.hitMetrics.textPosition;
            if (result.isTrailingHit)
                position++;

            if (position != _cursorPos)
            {
                _cursorPos = position;
                _caretTimer.Reset();
                InvokeRedraw();
            }

            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnMouseMove(int deltaX, int deltaY)
        {
            auto targets = Panel::_OnMouseMove(deltaX, deltaY);

            // Get new cursor position
            if (_textLabel->GetSelectionLength() != 0)
            {
                int newPosition = _textLabel->GetSelectionStart() + _textLabel->GetSelectionLength();
                if (newPosition != _cursorPos)
                {
                    _cursorPos = newPosition;
                    _caretTimer.Reset();
                    InvokeRedraw();
                }
            }

            // Set cursor icon
            int trueX = GetMousePosX();
            int trueY = GetMousePosY();
            RECT margins = GetTextAreaMargins();
            if (trueX < margins.left || trueX > GetWidth() - margins.right ||
                trueY < margins.top || trueY > GetHeight() - margins.bottom)
                SetDefaultCursor(targets.MainTarget()->GetDefaultCursor());
            else
                SetDefaultCursor(zwnd::CursorIcon::IBEAM);

            return EventTargets().Add(this, GetMousePosX(), GetMousePosY());
        }

        void _OnSelected(bool reverse); // Uses 'App'

        void _OnDeselected(); // Uses 'App'

        // Override 'Panel' tab handling
        Component* IterateTab(bool reverse)
        {
            return Component::IterateTab(reverse);
        }

        bool _OnHotkey(int id)
        {
            return false;
        }

        bool _OnKeyDown(BYTE vkCode)
        {
            static const std::wstring symbols = L"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
            static const std::wstring newline = L"\n\r";

            switch (vkCode)
            {
            case VK_LEFT:
            {
                bool selecting = false;
                bool wordMode = false;
                if (KeyState(VK_LEFT, KMOD_SHIFT))
                    selecting = true;
                if (KeyState(VK_LEFT, KMOD_CONTROL))
                    wordMode = true;

                int selStart = _textLabel->GetSelectionStart();
                int selLength = _textLabel->GetSelectionLength();
                std::wstring text = _textLabel->GetText();

                // Calculate new cursor pos
                unsigned int newCursorPos = _cursorPos;
                if (wordMode && newCursorPos > 0)
                {
                    bool skippingWhitespace = false;
                    bool skippingWord = false;
                    bool skippingSymbols = false;
                    bool skippingNewline = false;

                    newCursorPos--;
                    if (text[newCursorPos] == L' ')
                    {
                        skippingWhitespace = true;
                    }
                    else
                    {
                        if (std::find(symbols.begin(), symbols.end(), text[newCursorPos]) != symbols.end())
                            skippingSymbols = true;
                        else if (std::find(newline.begin(), newline.end(), text[newCursorPos]) != newline.end())
                            skippingNewline = true;
                        else
                            skippingWord = true;
                    }

                    while (newCursorPos > 0)
                    {
                        newCursorPos--;
                        if (text[newCursorPos] != L' ')
                        {
                            bool symbolChar = std::find(symbols.begin(), symbols.end(), text[newCursorPos]) != symbols.end();
                            bool newlineChar = std::find(newline.begin(), newline.end(), text[newCursorPos]) != newline.end();

                            if (skippingWhitespace)
                            {
                                skippingWhitespace = false;
                                if (newlineChar)
                                {
                                    newCursorPos++;
                                    break;
                                }
                                if (symbolChar)
                                    skippingSymbols = true;
                                else
                                    skippingWord = true;
                            }
                            else if (skippingSymbols)
                            {
                                if (!symbolChar)
                                {
                                    newCursorPos++;
                                    break;
                                }
                            }
                            else if (skippingNewline)
                            {
                                //if (!newlineChar)
                                //{
                                    newCursorPos++;
                                    break;
                                //}
                            }
                            else if (skippingWord)
                            {
                                if (newlineChar || symbolChar)
                                {
                                    newCursorPos++;
                                    break;
                                }
                            }
                        }
                        else if (!skippingWhitespace)
                        {
                            newCursorPos++;
                            break;
                        }
                    }
                }
                else
                {
                    if (!selecting && selLength != 0)
                    {
                        if (selLength < 0)
                            newCursorPos = selStart + selLength;
                        else
                            newCursorPos = selStart;
                    }
                    else if (newCursorPos > 0)
                    {
                        newCursorPos--;
                    }
                }

                _UpdateSelection(newCursorPos, selecting);
                _cursorPos = newCursorPos;
                _UpdateTargetCursorXPos();
                _UpdateLabelPlacement();
                _caretTimer.Reset();
                InvokeRedraw();
                break;
            }
            case VK_RIGHT:
            {
                bool selecting = false;
                bool wordMode = false;
                if (KeyState(VK_RIGHT, KMOD_SHIFT))
                    selecting = true;
                if (KeyState(VK_RIGHT, KMOD_CONTROL))
                    wordMode = true;

                int selStart = _textLabel->GetSelectionStart();
                int selLength = _textLabel->GetSelectionLength();
                std::wstring text = _textLabel->GetText();

                // Calculate new cursor pos
                unsigned int newCursorPos = _cursorPos;
                if (wordMode && newCursorPos < text.length())
                {
                    bool skippingWhitespace = false;
                    bool skippingWord = false;
                    bool skippingSymbols = false;
                    bool skippingNewline = false;

                    if (text[newCursorPos] == L' ')
                    {
                        skippingWhitespace = true;
                    }
                    else
                    {
                        if (std::find(symbols.begin(), symbols.end(), text[newCursorPos]) != symbols.end())
                            skippingSymbols = true;
                        else if (std::find(newline.begin(), newline.end(), text[newCursorPos]) != newline.end())
                            skippingNewline = true;
                        else
                            skippingWord = true;
                    }
                    newCursorPos++;

                    while (newCursorPos < text.length())
                    {
                        if (text[newCursorPos] != L' ')
                        {
                            if (skippingWhitespace)
                                break;

                            bool symbolChar = std::find(symbols.begin(), symbols.end(), text[newCursorPos]) != symbols.end();
                            bool newlineChar = std::find(newline.begin(), newline.end(), text[newCursorPos]) != newline.end();

                            if (skippingSymbols)
                            {
                                if (!symbolChar)
                                    break;
                            }
                            else if (skippingNewline)
                            {
                                //if (!newlineChar)
                                    break;
                            }
                            else if (skippingWord)
                            {
                                if (symbolChar || newlineChar)
                                    break;
                            }
                        }
                        else
                        {
                            if (skippingNewline)
                                break;
                            if (!skippingWhitespace)
                                skippingWhitespace = true;
                        }

                        newCursorPos++;
                    }
                }
                else
                {
                    if (!selecting && selLength != 0)
                    {
                        if (selLength > 0)
                            newCursorPos = selStart + selLength;
                        else
                            newCursorPos = selStart;
                    }
                    else if (newCursorPos < text.length())
                    {
                        newCursorPos++;
                    }
                }

                _UpdateSelection(newCursorPos, selecting);
                _cursorPos = newCursorPos;
                _UpdateTargetCursorXPos();
                _UpdateLabelPlacement();
                _caretTimer.Reset();
                InvokeRedraw();
                break;
            }
            case VK_UP:
            {
                bool selecting = false;
                if (KeyState(VK_UP, KMOD_SHIFT))
                    selecting = true;

                int newCursorPos = _cursorPos;

                auto hitTestResult = _textLabel->HitTestTextPosition(_cursorPos);
                auto lineMetrics = _textLabel->LineMetrics().lineMetrics;
                int lineIndex = _CurrentLineIndex(lineMetrics);

                // Move caret a line up
                if (lineIndex > 0)
                {
                    float aboveLineHeight = lineMetrics[lineIndex - 1].height;
                    float testPointYPos = hitTestResult.posY - aboveLineHeight * 0.5f;

                    auto result = _textLabel->HitTestPoint(_targetCursorXPos, testPointYPos);
                    int position = result.hitMetrics.textPosition;
                    if (result.isTrailingHit)
                        position++;

                    if (position != newCursorPos)
                    {
                        newCursorPos = position;
                    }
                }

                _UpdateSelection(newCursorPos, selecting);
                _cursorPos = newCursorPos;
                _UpdateLabelPlacement();
                _caretTimer.Reset();
                InvokeRedraw();
                break;
            }
            case VK_DOWN:
            {
                bool selecting = false;
                if (KeyState(VK_DOWN, KMOD_SHIFT))
                    selecting = true;

                int newCursorPos = _cursorPos;

                auto hitTestResult = _textLabel->HitTestTextPosition(_cursorPos);
                auto lineMetrics = _textLabel->LineMetrics().lineMetrics;
                int lineIndex = _CurrentLineIndex(lineMetrics);

                // Move caret a line down
                if (lineIndex < lineMetrics.size() - 1)
                {
                    float thisLineHeight = lineMetrics[lineIndex].height;
                    float belowLineHeight = lineMetrics[lineIndex + 1].height;
                    float testPointYPos = hitTestResult.posY + thisLineHeight + belowLineHeight * 0.5f;

                    auto result = _textLabel->HitTestPoint(_targetCursorXPos, testPointYPos);
                    int position = result.hitMetrics.textPosition;
                    if (result.isTrailingHit)
                        position++;

                    if (position != newCursorPos)
                    {
                        newCursorPos = position;
                    }
                }

                _UpdateSelection(newCursorPos, selecting);
                _cursorPos = newCursorPos;
                _UpdateLabelPlacement();
                _caretTimer.Reset();
                InvokeRedraw();
                break;
            }
            case VK_HOME:
            {
                bool selecting = false;
                bool pageMode = false;
                if (KeyState(VK_HOME, KMOD_SHIFT))
                    selecting = true;
                if (KeyState(VK_HOME, KMOD_CONTROL))
                    pageMode = true;

                std::wstring text = _textLabel->GetText();
                int newCursorPos = _cursorPos;

                if (pageMode)
                {
                    newCursorPos = 0;
                }
                else
                {
                    auto lineMetrics = _textLabel->LineMetrics().lineMetrics;
                    auto linePositions = _LineStartPositions(lineMetrics);
                    int lineIndex = _CurrentLineIndex(lineMetrics);

                    int startPosition = linePositions[lineIndex];
                    // Move start position after whitespace
                    while (startPosition < text.length() && text[startPosition] == L' ')
                        startPosition++;

                    // If cursor is at the line start, move it to text start
                    if (newCursorPos == linePositions[lineIndex])
                        newCursorPos = startPosition;
                    // If cursor is at or before text start, move it line start
                    else if (startPosition >= newCursorPos)
                        newCursorPos = linePositions[lineIndex];
                    // Otherwise (cursor is after text start), move it to text start
                    else
                        newCursorPos = startPosition;
                }

                _UpdateSelection(newCursorPos, selecting);
                _cursorPos = newCursorPos;
                _UpdateTargetCursorXPos();
                _UpdateLabelPlacement();
                _caretTimer.Reset();
                InvokeRedraw();
                break;
            }
            case VK_END:
            {
                bool selecting = false;
                bool pageMode = false;
                if (KeyState(VK_END, KMOD_SHIFT))
                    selecting = true;
                if (KeyState(VK_END, KMOD_CONTROL))
                    pageMode = true;

                std::wstring text = _textLabel->GetText();
                int newCursorPos = _cursorPos;

                if (pageMode)
                {
                    newCursorPos = text.length();
                }
                else
                {
                    auto lineMetrics = _textLabel->LineMetrics().lineMetrics;
                    auto linePositions = _LineStartPositions(lineMetrics);
                    int lineIndex = _CurrentLineIndex(lineMetrics);

                    // Move cursor to end of current line, before line end characters
                    newCursorPos = linePositions[lineIndex] + lineMetrics[lineIndex].length - lineMetrics[lineIndex].newlineLength;
                }

                _UpdateSelection(newCursorPos, selecting);
                _cursorPos = newCursorPos;
                _UpdateTargetCursorXPos();
                _UpdateLabelPlacement();
                _caretTimer.Reset();
                InvokeRedraw();
                break;
            }
            case 'A':
            {
                if (KeyState('A', KMOD_CONTROL))
                {
                    _cursorPos = _textLabel->GetText().length();
                    _textLabel->SetSelectionStart(0);
                    _textLabel->SetSelectionLength(_cursorPos);
                }
                break;
            }
            case 'C':
            {
                if (KeyState('C', KMOD_CONTROL))
                {
                    int selStart = _textLabel->GetSelectionStart();
                    int selLength = _textLabel->GetSelectionLength();
                    if (selLength != 0)
                    {
                        if (selLength < 0)
                        {
                            selStart = selStart + selLength;
                            selLength = -selLength;
                        }
                    }
                    std::wstring text = _textLabel->GetText();

                    if (selLength != 0)
                    {
                        std::wstring copyTextW = text.substr(selStart, selLength);
                        _ConvertNewlinesToCRLF(copyTextW);
                        std::string copyText = wstring_to_string(copyTextW);
                        copyTextW.resize(copyTextW.length() + 1);
                        copyText.resize(copyText.length() + 1);
                        // Passing the handle to the window causes some 'EmptyClipboard'
                        // calls take up to 5 seconds to complete. In addition, while the
                        // documentation states that 'SetClipboardData' should fail after
                        // emptying the clipboard after OpenClipboard(NULL), that does
                        // not appear to actually happen.
                        if (OpenClipboard(NULL))
                        {
                            EmptyClipboard();

                            { // Add wstring
                                HGLOBAL hGlobalMem = GlobalAlloc(GMEM_MOVEABLE, copyTextW.length() * sizeof(wchar_t));
                                wchar_t* wstrMem = (wchar_t*)GlobalLock(hGlobalMem);
                                if (wstrMem)
                                    std::copy_n(copyTextW.data(), copyTextW.length(), wstrMem);
                                GlobalUnlock(hGlobalMem);
                                SetClipboardData(CF_UNICODETEXT, hGlobalMem);
                            }
                            { // Add string
                                HGLOBAL hGlobalMem = GlobalAlloc(GMEM_MOVEABLE, copyText.length() * sizeof(char));
                                wchar_t* strMem = (wchar_t*)GlobalLock(hGlobalMem);
                                if (strMem)
                                    std::copy_n(copyText.data(), copyText.length(), strMem);
                                GlobalUnlock(hGlobalMem);
                                SetClipboardData(CF_TEXT, hGlobalMem);
                            }

                            CloseClipboard();
                        }
                    }
                }
                break;
            }
            case VK_TAB:
            {
                if (_tabAllowed)
                {
                    break;
                }
                else
                {
                    return false;
                }
            }
            case VK_SHIFT:
            {
                if (_tabAllowed)
                {
                    break;
                }
                else
                {
                    return false;
                }
            }
            default:
                break;
            }

            // Text modifying key handling
            if (1) {
                std::wstring newText = _textLabel->GetText();
                unsigned int newCursorPos = _cursorPos;

                int selStart = _textLabel->GetSelectionStart();
                int selLength = _textLabel->GetSelectionLength();
                if (selLength != 0)
                {
                    if (selLength < 0)
                    {
                        selStart = selStart + selLength;
                        selLength = -selLength;
                    }
                }
                auto EraseSelectedText = [&]()
                {
                    newText.erase(selStart, selLength);
                    newCursorPos = selStart;
                    _textLabel->SetSelectionStart(0);
                    _textLabel->SetSelectionLength(0);
                    InvokeRedraw();
                };

                switch (vkCode)
                {
                case VK_BACK:
                {
                    if (selLength != 0)
                    {
                        EraseSelectedText();
                    }
                    else if (_cursorPos > 0)
                    {
                        newText.erase(newText.begin() + _cursorPos - 1);
                        newCursorPos--;
                    }
                    break;
                }
                case VK_DELETE:
                {
                    if (selLength != 0)
                    {
                        EraseSelectedText();
                    }
                    else if (_cursorPos < newText.length())
                    {
                        newText.erase(newText.begin() + _cursorPos);
                    }
                    break;
                }
                case VK_RETURN:
                {
                    if (_multiline)
                    {
                        newText.insert(newText.begin() + newCursorPos, L'\n');
                        newCursorPos++;
                    }
                    break;
                }
                case 'V':
                {
                    if (KeyState('V', KMOD_CONTROL))
                    {
                        // Paste
                        std::wstring pasteText;
                        if (OpenClipboard(NULL))
                        {
                            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
                            if (hData != nullptr)
                            {
                                wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
                                if (pszText != nullptr)
                                {
                                    pasteText = pszText;
                                }
                                GlobalUnlock(hData);
                            }
                            CloseClipboard();
                        }

                        if (!pasteText.empty())
                        {
                            _ParseNewlines(pasteText);

                            if (selLength != 0)
                                EraseSelectedText();

                            newText.insert(newText.begin() + newCursorPos, pasteText.begin(), pasteText.end());
                            newCursorPos += pasteText.length();
                        }
                    }
                    break;
                }
                case 'X':
                {
                    if (KeyState('X', KMOD_CONTROL))
                    {
                        if (selLength != 0)
                        {
                            std::wstring copyTextW = newText.substr(selStart, selLength);
                            _ConvertNewlinesToCRLF(copyTextW);
                            std::string copyText = wstring_to_string(copyTextW);
                            copyTextW.resize(copyTextW.length() + 1);
                            copyText.resize(copyText.length() + 1);
                            // Passing the handle to the window causes some 'EmptyClipboard'
                            // calls take up to 5 seconds to complete. In addition, while the
                            // documentation states that 'SetClipboardData' should fail after
                            // emptying the clipboard after OpenClipboard(NULL), that does
                            // not appear to actually happen.
                            if (OpenClipboard(NULL))
                            {
                                EmptyClipboard();

                                { // Add wstring
                                    HGLOBAL hGlobalMem = GlobalAlloc(GMEM_MOVEABLE, copyTextW.length() * sizeof(wchar_t));
                                    wchar_t* wstrMem = (wchar_t*)GlobalLock(hGlobalMem);
                                    if (wstrMem)
                                        std::copy_n(copyTextW.data(), copyTextW.length(), wstrMem);
                                    GlobalUnlock(hGlobalMem);
                                    SetClipboardData(CF_UNICODETEXT, hGlobalMem);
                                }
                                { // Add string
                                    HGLOBAL hGlobalMem = GlobalAlloc(GMEM_MOVEABLE, copyText.length() * sizeof(char));
                                    wchar_t* strMem = (wchar_t*)GlobalLock(hGlobalMem);
                                    if (strMem)
                                        std::copy_n(copyText.data(), copyText.length(), strMem);
                                    GlobalUnlock(hGlobalMem);
                                    SetClipboardData(CF_TEXT, hGlobalMem);
                                }

                                CloseClipboard();
                            }
                            EraseSelectedText();
                        }
                    }
                    break;
                }
                default:
                    break;
                }

                if (newText != _textLabel->GetText() && _TextMatches(newText, _pattern))
                {
                    _settingInternally = true;
                    _textLabel->SetText(newText);
                    _settingInternally = false;
                    _cursorPos = newCursorPos;
                    _UpdateTargetCursorXPos();
                }
                _caretTimer.Reset();
                _UpdateLabelPlacement();
            }

            return true;
        }

        bool _OnKeyUp(BYTE vkCode)
        {
            switch (vkCode)
            {
            case VK_TAB:
            {
                if (_tabAllowed)
                {
                    break;
                }
                else
                {
                    return false;
                }
            }
            case VK_SHIFT:
            {
                if (_tabAllowed)
                {
                    break;
                }
                else
                {
                    return false;
                }
            }
            }

            return true;
        }

        bool _OnChar(wchar_t ch)
        {
            std::wstring newText = _textLabel->GetText();
            unsigned int newCursorPos = _cursorPos;

            int selStart = _textLabel->GetSelectionStart();
            int selLength = _textLabel->GetSelectionLength();
            if (selLength != 0)
            {
                if (selLength < 0)
                {
                    selStart = selStart + selLength;
                    selLength = -selLength;
                }
            }
            auto EraseSelectedText = [&]()
            {
                newText.erase(selStart, selLength);
                newCursorPos = selStart;
                _textLabel->SetSelectionStart(0);
                _textLabel->SetSelectionLength(0);
                InvokeRedraw();
            };

            bool handled = false;
            if (ch == L'\t')
            {
                if (!_tabAllowed)
                {
                    handled = true;
                }
            }
            else if (ch >= 0 && ch < 32)
            {
                handled = true;
            }
            
            if (!handled)
            {
                if (selLength != 0)
                    EraseSelectedText();

                newText.insert(newText.begin() + newCursorPos, ch);
                newCursorPos++;
            }

            if (_TextMatches(newText, _pattern))
            {
                _settingInternally = true;
                _textLabel->SetText(newText);
                _settingInternally = false;
                _cursorPos = newCursorPos;
            }

            _caretTimer.Reset();
            _UpdateTargetCursorXPos();
            _UpdateLabelPlacement();
            return true;
        }

    public:
        const char* GetName() const { return Name(); }
        static const char* Name() { return "text_input"; }
#pragma endregion

    private:
        bool _multiline = false;
        bool _tabAllowed = false;

        unsigned int _cursorPos = 0;
        // When going up/down lines, the visual cursor X position
        // should be kept around the same. This value stays the same
        // while going up/down and changes when going sideways.
        float _targetCursorXPos = 0.0f;

        TimePoint _lastHorizontalScroll = TimePoint(0);
        TimePoint _lastVerticalScroll = TimePoint(0);

        bool _caretVisible = false;
        Clock _caretTimer = Clock(0);

        std::unique_ptr<Label> _textLabel = nullptr;
        std::unique_ptr<Label> _placeholderTextLabel = nullptr;
        RECT _textArea;
        RECT _textAreaMargins = RECT{ 0, 0, 0, 0 };

        std::wstring _pattern = L"";
        MatchEnforcing _matchEnforcing = MatchEnforcing::IMMEDIATE;
        std::wstring _initialText = L"";

        EventEmitter<void, Label*, std::wstring*> _textChangedEvent;
        bool _settingInternally = false;

    protected:
        friend class Scene;
        friend class Component;
        TextInput(Scene* scene) : Panel(scene) {}
        void Init()
        {
            Panel::Init();

            _customInactiveDraw = true;
            SetDefaultCursor(zwnd::CursorIcon::IBEAM);
            SetSelectable(true);
            SetBorderVisibility(true);
            SetBorderColor(D2D1::ColorF(0.3f, 0.3f, 0.3f));
            SetBackgroundColor(D2D1::ColorF(0.1f, 0.1f, 0.1f));

            _textLabel = Create<zcom::Label>(L"");
            //_textLabel->SetParentSizePercent(1.0f, 1.0f);
            _textLabel->SetVerticalTextAlignment(zcom::Alignment::CENTER);
            _textLabel->SetHorizontalTextAlignment(zcom::TextAlignment::LEADING);
            _textLabel->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
            _textLabel->SetMargins({ 5.0f, 0.0f, 5.0f });
            _textLabel->SetTextSelectable(true);
            _textLabel->SetVisible(true);

            _placeholderTextLabel = Create<zcom::Label>(L"");
            //_placeholderTextLabel->SetParentSizePercent(1.0f, 1.0f);
            _placeholderTextLabel->SetVerticalTextAlignment(zcom::Alignment::CENTER);
            _placeholderTextLabel->SetHorizontalTextAlignment(zcom::TextAlignment::LEADING);
            _placeholderTextLabel->SetFontColor(D2D1::ColorF(0.3f, 0.3f, 0.3f));
            _placeholderTextLabel->SetMargins({ 5.0f, 0.0f, 5.0f });
            _placeholderTextLabel->SetVisible(false);

            AddItem(_textLabel.get());
            AddItem(_placeholderTextLabel.get());

            _textLabel->SubscribeOnTextChanged([&](Label* label, std::wstring* newText)
            {
                _OnLabelTextChanged(label, newText);
            }).Detach();
            _textLabel->SubscribeOnTextFormatChanged([&](Label* label)
            {
                _OnLabelTextFormatChanged(label);
            }).Detach();
            _textLabel->SubscribeOnTextLayoutChanged([&](Label* label)
            {
                _OnLabelTextLayoutChanged(label);
            }).Detach();

            _UpdateTargetCursorXPos();
            _UpdateTextArea();
        }
    public:
        ~TextInput() {}
        TextInput(TextInput&&) = delete;
        TextInput& operator=(TextInput&&) = delete;
        TextInput(const TextInput&) = delete;
        TextInput& operator=(const TextInput&) = delete;

        Label* Text() const
        {
            return _textLabel.get();
        }

        Label* PlaceholderText() const
        {
            return _placeholderTextLabel.get();
        }

        RECT GetTextAreaMargins() const
        {
            return _textAreaMargins;
        }

        bool GetMultiline() const
        {
            return _multiline;
        }

        bool GetTabAllowed() const
        {
            return _tabAllowed;
        }

        std::wstring GetPattern() const
        {
            return _pattern;
        }

        MatchEnforcing GetMatchEnforcing() const
        {
            return _matchEnforcing;
        }

        void SetTextAreaMargins(RECT margins)
        {
            if (margins == _textAreaMargins)
                return;

            _textAreaMargins = margins;
            _UpdateTextArea();
        }

        void SetMultiline(bool multiline)
        {
            if (multiline == _multiline)
                return;

            _multiline = multiline;
            _textLabel->SetWordWrap(_multiline);
            _placeholderTextLabel->SetWordWrap(_multiline);
            _UpdateLabelPlacement();
        }

        void SetTabAllowed(bool allowed)
        {
            _tabAllowed = allowed;
        }

        void SetPattern(std::wstring pattern)
        {
            if (pattern == _pattern)
                return;

            _pattern = pattern;
            if (_pattern.empty())
                return;
            if (!std::regex_match(_textLabel->GetText(), std::wregex(_pattern)))
                _textLabel->SetText(L"");
        }

        void SetMatchEnforcing(MatchEnforcing matchEnforcing)
        {
            _matchEnforcing = matchEnforcing;
        }

        // Handler parameters:
        // - a pointer to the label object
        // - a reference to the new text string. This parameter can be modified
        EventSubscription<void, Label*, std::wstring*> SubscribeOnTextChanged(std::function<void(Label*, std::wstring*)> handler)
        {
            return _textChangedEvent->Subscribe(handler);
        }

    protected:
        int _CurrentLineIndex(const std::vector<DWRITE_LINE_METRICS>& metrics)
        {
            int charCounter = 0;
            int lineIndex = 0;
            for (auto& line : metrics)
            {
                charCounter += line.length;
                if (_cursorPos < charCounter)
                    break;
                lineIndex++;
                if (lineIndex == metrics.size())
                    lineIndex--;
            }
            return lineIndex;
        }

        std::vector<int> _LineStartPositions(const std::vector<DWRITE_LINE_METRICS>& metrics)
        {
            if (metrics.size() == 0)
                return std::vector<int>();

            std::vector<int> positions;
            positions.resize(metrics.size());
            positions[0] = 0;
            for (int i = 1; i < metrics.size(); i++)
            {
                positions[i] = positions[i - 1] + metrics[i - 1].length;
            }
            return positions;
        }

        void _UpdateTextArea()
        {
            _textArea.left = _textAreaMargins.left;
            _textArea.right = GetWidth() - _textAreaMargins.right;
            _textArea.top = _textAreaMargins.top;
            _textArea.bottom = GetHeight() - _textAreaMargins.bottom;
            _UpdateLabelPlacement();
        }

        void _UpdateLabelPlacement()
        {
            // Update label size
            if (!_multiline)
            {
                _textLabel->SetCutoff(L"");
                _textLabel->SetWordWrap(false);
                int width = std::ceilf(_textLabel->GetTextWidth());
                if (width < _textArea.right - _textArea.left)
                    width = _textArea.right - _textArea.left;
                _textLabel->SetBaseSize(width, _textArea.bottom - _textArea.top);

                _placeholderTextLabel->SetCutoff(L"");
                _placeholderTextLabel->SetWordWrap(false);
                width = std::ceilf(_placeholderTextLabel->GetTextWidth());
                if (width < _textArea.right - _textArea.left)
                    width = _textArea.right - _textArea.left;
                _placeholderTextLabel->SetBaseSize(width, _textArea.bottom - _textArea.top);
            }
            else
            {
                _textLabel->SetCutoff(L"");
                _textLabel->SetWordWrap(true);
                int height = std::ceilf(_textLabel->GetTextHeight());
                if (height < _textArea.bottom - _textArea.top)
                    height = _textArea.bottom - _textArea.top;
                _textLabel->SetBaseSize(_textArea.right - _textArea.left, height);

                _placeholderTextLabel->SetCutoff(L"");
                _placeholderTextLabel->SetWordWrap(true);
                height = std::ceilf(_placeholderTextLabel->GetTextHeight());
                if (height < _textArea.bottom - _textArea.top)
                    height = _textArea.bottom - _textArea.top;
                _placeholderTextLabel->SetBaseSize(_textArea.right - _textArea.left, height);
            }

            // Move label to make cursor visible
            auto result = _textLabel->HitTestTextPosition(_cursorPos);
            float caretTop = result.posY + _textLabel->GetVerticalOffsetPixels();
            float caretBottom = caretTop + result.hitMetrics.height;
            float caretLeft = result.posX + _textLabel->GetHorizontalOffsetPixels();
            float caretRight = caretLeft + 5.0f;

            if (caretTop < _textArea.top)
                _textLabel->SetVerticalOffsetPixels(_textArea.top - std::floorf(result.posY));
            else if (caretBottom > _textArea.bottom)
                _textLabel->SetVerticalOffsetPixels(_textArea.bottom - std::ceilf(result.posY + result.hitMetrics.height));

            if (caretLeft < _textArea.left)
                _textLabel->SetHorizontalOffsetPixels(_textArea.left - std::floorf(result.posX));
            else if (caretRight > _textArea.right)
                _textLabel->SetHorizontalOffsetPixels(_textArea.right - std::ceilf(result.posX + 5.0f));

            // Check (and fix) if new position is out of bounds
            if (_textLabel->GetHorizontalOffsetPixels() + _textLabel->GetBaseWidth() <= _textArea.right)
                _textLabel->SetHorizontalOffsetPixels(_textArea.right - _textLabel->GetBaseWidth());
            else if (_textLabel->GetHorizontalOffsetPixels() > _textArea.left)
                _textLabel->SetHorizontalOffsetPixels(_textArea.left);
            if (_textLabel->GetVerticalOffsetPixels() + _textLabel->GetBaseHeight() <= _textArea.bottom)
                _textLabel->SetVerticalOffsetPixels(_textArea.bottom - _textLabel->GetBaseHeight());
            else if (_textLabel->GetVerticalOffsetPixels() > _textArea.top)
                _textLabel->SetVerticalOffsetPixels(_textArea.top);
        }

        void _UpdateTargetCursorXPos()
        {
            _targetCursorXPos = _textLabel->HitTestTextPosition(_cursorPos).posX;
        }

        void _UpdateSelection(int newCursorPos, bool selecting = true)
        {
            int selStart = _textLabel->GetSelectionStart();
            int selLength = _textLabel->GetSelectionLength();
            if (selecting)
            {
                if (selLength == 0)
                {
                    _textLabel->SetSelectionStart(_cursorPos);
                    _textLabel->SetSelectionLength(newCursorPos - _cursorPos);
                }
                else
                {
                    _textLabel->SetSelectionLength(newCursorPos - selStart);
                }
            }
            else
            {
                _textLabel->SetSelectionStart(0);
                _textLabel->SetSelectionLength(0);
            }
        }

        void _ParseNewlines(std::wstring& str)
        {
            int index = 0;
            while (index < str.length())
            {
                if (str[index] == L'\r')
                {
                    if (index + 1 < str.length() && str[index + 1] == L'\n')
                        str.erase(str.begin() + index);
                    else
                        str[index] = L'\n';
                }
                index++;
            }
        }

        void _ConvertNewlinesToCRLF(std::wstring& str)
        {
            for (int i = 0; i < str.length(); i++)
            {
                if (str[i] == L'\n')
                {
                    str.insert(str.begin() + i, L'\r');
                    i++;
                }
            }
        }

        bool _TextMatches(const std::wstring& text, const std::wstring& pattern)
        {
            return pattern.empty() || text.empty() || std::regex_match(text, std::wregex(pattern));
        }

        void _OnLabelTextChanged(Label* label, std::wstring* newText)
        {
            if (!_settingInternally)
            {
                if (!_TextMatches(*newText, _pattern))
                {
                    *newText = label->GetText();
                }
            }

            _textChangedEvent->InvokeAll(label, newText);
        }

        void _OnLabelTextFormatChanged(Label* label)
        {

        }

        void _OnLabelTextLayoutChanged(Label* label)
        {
            _UpdateLabelPlacement();
        }
    };
}