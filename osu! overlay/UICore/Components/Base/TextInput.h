#pragma once

#include "App.h"
#include "ScrollPanel.h"
#include "Label.h"
#include "Window/KeyboardEventHandler.h"

#include "Helper/Time.h"
#include "Helper/StringHelper.h"

#include <regex>

namespace zcom
{
    class TextInput : public Panel, public KeyboardEventHandler
    {
        DEFINE_COMPONENT(TextInput, Panel)
    public:
        ~TextInput()
        {
            _textPanel->ClearItems();
        }
        HIDE_PANEL_METHODS
    protected:
        void Init()
        {
            Panel::Init();

            _customInactiveDraw = true;
            selectable = true;
            border.visible = true;
            border.color = Color(0x4D4D4D);
            backgroundColor = Color(0x1A1A1A);

            _textPanel = Create<ScrollPanel>();
            _textPanel->parentSize = { 1.0f, 1.0f };
            _textPanel->xScrollbar.scrollable = true;
            _textPanel->xScrollbar.visibleOnScroll = false;
            _textPanel->cursorIcon = zwnd::CursorIcon::IBEAM;
            _textPanel->SubscribePostDraw([=](Component*, Graphics* g) {
                // Draw caret
                if (caretVisible_)
                {
                    auto metrics = _textLabel->HitTestTextPosition(cursorPos_);

                    RectF caretRect{};
                    caretRect.left = _textLabel->position_->x + metrics.position.x - _textPanel->xScrollbar.visualScrollAmount;
                    caretRect.right = _textLabel->position_->x + caretRect.left + 2.0f;
                    caretRect.top = _textLabel->position_->y + metrics.position.y - _textPanel->yScrollbar.visualScrollAmount;
                    caretRect.bottom = _textLabel->position_->y + caretRect.top + metrics.hitMetrics.height;
                    g->FillRectangle(caretRect, Color(0xDDDDDD));
                }
            }).Detach();
            _textPanel->contentSize_.Subscribe([=](Size) {
                _MoveViewToCursor();
            }).Detach();

            _textLabel = Create<Label>(L"");
            _textLabel->autoWidth = true;
            _textLabel->autoHeight = true;
            _textLabel->minAutoWidth.ComputedFrom([](Size panelSize) { return panelSize.width; }, _textPanel->size_);
            _textLabel->minAutoHeight.ComputedFrom([](Size panelSize) { return panelSize.height; }, _textPanel->size_);
            _textLabel->maxAutoWidth.ComputedFrom([](Size panelSize, bool multiline) { return multiline ? std::optional(panelSize.width) : std::nullopt; }, _textPanel->size_, multiline);
            _textLabel->xTextAlign = TextAlignment::LEADING;
            _textLabel->yTextAlign = Alignment::CENTER;
            _textLabel->fontColor = Color(0xCCCCCC);
            _textLabel->padding = RectF{ 5.0f, 0.0f, 5.0f };
            _textLabel->textSelectable = true;
            _textLabel->visible.ComputedFrom([](std::wstring text) { return !text.empty(); }, text);

            _placeholderTextLabel = Create<zcom::Label>(L"");
            _placeholderTextLabel->autoWidth = true;
            _placeholderTextLabel->autoHeight = true;
            _placeholderTextLabel->maxAutoWidth.ComputedFrom([](Size panelSize, bool multiline) { return multiline ? std::optional(panelSize.width) : std::nullopt; }, _textPanel->size_, multiline);
            _placeholderTextLabel->xTextAlign = TextAlignment::LEADING;
            _placeholderTextLabel->yTextAlign = Alignment::CENTER;
            _placeholderTextLabel->fontColor = Color(0x4D4D4D);
            _placeholderTextLabel->padding = RectF{ 5.0f, 0.0f, 5.0f };
            _placeholderTextLabel->visible.ComputedFrom([](bool textVisible) { return !textVisible; }, _textLabel->visible);

            _textPanel->AddItem(_textLabel.get());
            _textPanel->AddItem(_placeholderTextLabel.get());
            AddItem(_textPanel.get());

            _textLabel->SubscribeOnTextChanged([&](Label* label, std::wstring* newText) {
                _OnLabelTextChanged(label, newText);
            }).Detach();
            _textLabel->SubscribeOnLeftPressed([=](Component*, Point point) {
                _OnTextLeftClicked(point);
            }).Detach();
            _textLabel->SubscribePostMouseMove([=](Component*, std::vector<EventContext::Params>, Point, Point) {
                _OnTextMouseMove();
            }).Detach();

            _UpdateTargetCursorXPos();
        }

    public:
        // Used to select when the match pattern is enforced
        enum class MatchEnforcing
        {
            // Prevents non matching text from being typed altogether
            IMMEDIATE,
            // If the entered text does not match the pattern when the
            // input is deselected, the contents are reverted
            ON_DESELECT
        };
        constexpr std::vector<std::pair<int64_t, std::wstring>> MatchEnforcingValueProxySelectionValues()
        {
            return {
                { (int64_t)MatchEnforcing::IMMEDIATE, L"Immediate" },
                { (int64_t)MatchEnforcing::ON_DESELECT, L"On deselect" }
            };
        }

        ScrollPanel* TextPanel() const { return _textPanel.get(); }
        Label* TextLabel() const { return _textLabel.get(); }
        Label* PlaceholderTextLabel() const { return _placeholderTextLabel.get(); }

        Value<std::wstring> text = Value<std::wstring>(L"", [=](std::wstring& currentValue, const std::wstring& newText) {
            if (!_TextMatches(newText, pattern))
                return;

            std::wstring newTextFinal = newText;
            if (_settingTypedText)
                _textChangedEvent->InvokeAll(&newTextFinal);

            currentValue = std::move(newTextFinal);

            _settingLabelInternally = true;
            if (maskCharacter->has_value())
                _textLabel->text = std::wstring(currentValue.length(), maskCharacter->value());
            else
                _textLabel->text = currentValue;
            _settingLabelInternally = false;

            if (cursorPos_ > currentValue.length())
                cursorPos_ = currentValue.length();
        });
        Value<bool> multiline = Value<bool>(false, [=](bool& currentValue, const bool& multiline) {
            currentValue = multiline;
            _textLabel->wordWrapping = multiline ? WordWrapping::WRAP : WordWrapping::NO_WRAP;
            _placeholderTextLabel->wordWrapping = multiline ? WordWrapping::WRAP : WordWrapping::NO_WRAP;
            _MoveViewToCursor();
        });
        Value<bool> tabAllowed = false;
        Value<std::wstring> pattern = Value<std::wstring>(L"", [=](std::wstring& currentValue, const std::wstring& pattern) {
            currentValue = pattern;
            if (pattern.empty())
                return;
            if (!_TextMatches(text, pattern))
                text = L"";
        });
        Value<MatchEnforcing> matchEnforcing = MatchEnforcing::IMMEDIATE;
        Value<bool> hideCaret = false;
        Value<std::optional<wchar_t>> maskCharacter = Value<std::optional<wchar_t>>(std::nullopt, [=](std::optional<wchar_t>& currentValue, const std::optional<wchar_t>& newValue) {
            currentValue = newValue;
            _settingLabelInternally = true;
            if (newValue)
                _textLabel->text = std::wstring(text->length(), newValue.value());
            else
                _textLabel->text = text;
            _settingLabelInternally = false;
        });

        Value<size_t> cursorPos_ = 0;
        Value<bool> caretVisible_ = false;
        Value<Clock> caretTimer_ = Clock(0);

        // Handler parameters:
        // - a reference to the new text string. This parameter can be modified
        [[nodiscard]] EventSubscription<void, std::wstring*> SubscribeOnTextChanged(std::function<void(std::wstring*)> handler)
        {
            return _textChangedEvent->Subscribe(handler);
        }

    private:
        // When going up/down lines, the visual cursor X position
        // should be kept around the same. This value stays the same
        // while going up/down and changes when going sideways.
        float _targetCursorXPos = 0.0f;

        TimePoint _lastHorizontalScroll = TimePoint(0);
        TimePoint _lastVerticalScroll = TimePoint(0);

        std::unique_ptr<ScrollPanel> _textPanel = nullptr;
        std::unique_ptr<Label> _textLabel = nullptr;
        std::unique_ptr<Label> _placeholderTextLabel = nullptr;

        std::wstring _initialText = L"";

        EventEmitter<void, std::wstring*> _textChangedEvent;
        bool _settingLabelInternally = false;
        bool _settingTypedText = false;

    protected:
        size_t _CurrentLineIndex(const std::vector<TextLineMetrics>& metrics)
        {
            size_t charCounter = 0;
            size_t lineIndex = 0;
            for (auto& line : metrics)
            {
                charCounter += line.length;
                if (cursorPos_ < charCounter)
                    break;
                lineIndex++;
                if (lineIndex == metrics.size())
                    lineIndex--;
            }
            return lineIndex;
        }

        std::vector<size_t> _LineStartPositions(const std::vector<TextLineMetrics>& metrics)
        {
            if (metrics.size() == 0)
                return std::vector<size_t>();

            std::vector<size_t> positions;
            positions.resize(metrics.size());
            positions[0] = 0;
            for (size_t i = 1; i < metrics.size(); i++)
            {
                positions[i] = positions[i - 1] + metrics[i - 1].length;
            }
            return positions;
        }

        void _MoveViewToCursor()
        {
            // Move label to make cursor visible
            auto result = _textLabel->HitTestTextPosition(cursorPos_);
            float caretTop = result.position.y - _textPanel->yScrollbar.scrollAmount_;
            float caretBottom = caretTop + result.hitMetrics.height;
            float caretLeft = result.position.x - _textPanel->xScrollbar.scrollAmount_;
            float caretRight = caretLeft + 5.0f;

            if (caretTop - 5.0f < 0)
                _textPanel->Scroll(_textPanel->yScrollbar, int(std::floorf(result.position.y - 5.0f)));
            else if (caretBottom + 5.0f > _textPanel->size_->height)
                _textPanel->Scroll(_textPanel->yScrollbar, int(std::ceilf(result.position.y + 5.0f + result.hitMetrics.height)) - _textPanel->size_->height);

            if (caretLeft - 5.0f < 0)
                _textPanel->Scroll(_textPanel->xScrollbar, int(std::floorf(result.position.x - 5.0f)));
            else if (caretRight + 5.0f > _textPanel->size_->width)
                _textPanel->Scroll(_textPanel->xScrollbar, int(std::ceilf(result.position.x + 5.0f + 5.0f)) - _textPanel->size_->width);
        }

        void _UpdateTargetCursorXPos()
        {
            _targetCursorXPos = _textLabel->HitTestTextPosition(cursorPos_).position.x;
        }

        void _UpdateSelection(size_t newCursorPos, bool selecting = true)
        {
            size_t selStart = _textLabel->selectionStart;
            size_t selEnd = _textLabel->selectionEnd;
            if (selecting)
            {
                if (selStart == selEnd)
                    _textLabel->selectionStart = cursorPos_.Get();
                _textLabel->selectionEnd = newCursorPos;
            }
            else
            {
                _textLabel->selectionStart = 0;
                _textLabel->selectionEnd = 0;
            }
        }

        void _ParseNewlines(std::wstring& str)
        {
            size_t index = 0;
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
            if (!_settingLabelInternally)
            {
                // Prevent any external changes to the inner label
                *newText = label->text;
            }
        }

        void _OnTextLeftClicked(Point point)
        {
            auto result = _textLabel->HitTestPoint(point.ToPointF());
            size_t position = result.hitMetrics.textPosition;
            if (result.isTrailingHit)
                position++;

            if (position != cursorPos_)
            {
                cursorPos_ = position;
                caretTimer_->Reset();
                _textPanel->InvokeRedraw();
            }
        }

        void _OnTextMouseMove()
        {
            if (!_textLabel->leftClicked_)
                return;

            if (_textLabel->selectionStart != _textLabel->selectionEnd)
            {
                if (_textLabel->selectionEnd != cursorPos_)
                {
                    cursorPos_ = _textLabel->selectionEnd.Get();
                    caretTimer_->Reset();
                    _textPanel->InvokeRedraw();
                }
            }
        }

    protected:
        void _OnUpdate() override
        {
            Panel::_OnUpdate();

            for (auto& item : _items)
                item.item->disabled = disabled;

            if (!selected_ || hideCaret)
            {
                if (caretVisible_)
                {
                    caretVisible_ = false;
                    _textPanel->InvokeRedraw();
                }
            }
            else
            {
                caretTimer_->Update();
                if (caretTimer_->Now().GetTime(MILLISECONDS) % 1000 < 500)
                {
                    if (!caretVisible_)
                    {
                        caretVisible_ = true;
                        _textPanel->InvokeRedraw();
                    }
                }
                else if (caretVisible_)
                {
                    caretVisible_ = false;
                    _textPanel->InvokeRedraw();
                }
            }

            // Scroll
            if (_textLabel->leftClicked_)
            {
                int newX = _textPanel->xScrollbar.scrollAmount_;
                int newY = _textPanel->yScrollbar.scrollAmount_;
                int mousePosX = _textPanel->mousePosition_->x;
                int mousePosY = _textPanel->mousePosition_->y;

                if (mousePosX < 0)
                {
                    float offBoundsAmount = -mousePosX / 200.0f;
                    if (offBoundsAmount > 1.0f)
                        offBoundsAmount = 1.0f;
                    if (offBoundsAmount < 0.0f)
                        offBoundsAmount = 0.0f;
                    offBoundsAmount = std::powf(std::sinf(offBoundsAmount * 1.5708f), 0.1f);
                    int scrollInterval = int(100 - 100.0f * offBoundsAmount);
                    if (scrollInterval < 1)
                        scrollInterval = 1;
                    while (ztime::Main() - _lastHorizontalScroll > Duration(scrollInterval, MILLISECONDS))
                    {
                        _lastHorizontalScroll += Duration(scrollInterval, MILLISECONDS);
                        newX--;
                    }
                }
                else if (mousePosX > _textPanel->size_->width)
                {
                    float offBoundsAmount = (mousePosX - _textPanel->size_->width) / 200.0f;
                    if (offBoundsAmount > 1.0f)
                        offBoundsAmount = 1.0f;
                    if (offBoundsAmount < 0.0f)
                        offBoundsAmount = 0.0f;
                    offBoundsAmount = std::powf(std::sinf(offBoundsAmount * 1.5708f), 0.1f);
                    int scrollInterval = int(100 - 100.0f * offBoundsAmount);
                    if (scrollInterval < 1)
                        scrollInterval = 1;
                    while (ztime::Main() - _lastHorizontalScroll > Duration(scrollInterval, MILLISECONDS))
                    {
                        _lastHorizontalScroll += Duration(scrollInterval, MILLISECONDS);
                        newX++;
                    }
                }
                else
                {
                    _lastHorizontalScroll = ztime::Main();
                }

                if (mousePosY < 0)
                {
                    float offBoundsAmount = -mousePosY / 200.0f;
                    if (offBoundsAmount > 1.0f)
                        offBoundsAmount = 1.0f;
                    if (offBoundsAmount < 0.0f)
                        offBoundsAmount = 0.0f;
                    offBoundsAmount = std::powf(std::sinf(offBoundsAmount * 1.5708f), 0.1f);
                    int scrollInterval = int(100 - 100.0f * offBoundsAmount);
                    if (scrollInterval < 1)
                        scrollInterval = 1;
                    while (ztime::Main() - _lastVerticalScroll > Duration(scrollInterval, MILLISECONDS))
                    {
                        _lastVerticalScroll += Duration(scrollInterval, MILLISECONDS);
                        newY--;
                    }
                }
                else if (mousePosY > _textPanel->size_->height)
                {
                    float offBoundsAmount = (mousePosY - _textPanel->size_->height) / 200.0f;
                    if (offBoundsAmount > 1.0f)
                        offBoundsAmount = 1.0f;
                    if (offBoundsAmount < 0.0f)
                        offBoundsAmount = 0.0f;
                    offBoundsAmount = std::powf(std::sinf(offBoundsAmount * 1.5708f), 0.1f);
                    int scrollInterval = int(100 - 100.0f * offBoundsAmount);
                    if (scrollInterval < 1)
                        scrollInterval = 1;
                    while (ztime::Main() - _lastVerticalScroll > Duration(scrollInterval, MILLISECONDS))
                    {
                        _lastVerticalScroll += Duration(scrollInterval, MILLISECONDS);
                        newY++;
                    }
                }
                else
                {
                    _lastVerticalScroll = ztime::Main();
                }

                _textPanel->Scroll(_textPanel->xScrollbar, newX);
                _textPanel->Scroll(_textPanel->yScrollbar, newY);
            }
            else
            {
                _lastHorizontalScroll = ztime::Main();
                _lastVerticalScroll = ztime::Main();
            }
        }

        EventContext _OnLeftPressed(Point point) override
        {
            auto targets = Panel::_OnLeftPressed(point);
            if (targets.Contains(_textPanel.get()))
                return EventContext().Add(this, point);
            else
                return targets;
        }

        void _OnSelected(bool reverse) override; // Uses 'App'

        void _OnDeselected() override; // Uses 'App'

        // Override 'Panel' tab handling
        Component* IterateTab(bool reverse) override
        {
            return Component::IterateTab(reverse);
        }

        bool _OnHotkey(int id) override
        {
            return false;
        }

        bool _OnKeyDown(BYTE vkCode) override
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

                size_t selStart = _textLabel->selectionStart;
                size_t selEnd = _textLabel->selectionEnd;

                // Calculate new cursor pos
                size_t newCursorPos = cursorPos_;
                if (wordMode && newCursorPos > 0)
                {
                    bool skippingWhitespace = false;
                    bool skippingWord = false;
                    bool skippingSymbols = false;
                    bool skippingNewline = false;

                    newCursorPos--;
                    if (_textLabel->text.Get()[newCursorPos] == L' ')
                    {
                        skippingWhitespace = true;
                    }
                    else
                    {
                        if (std::find(symbols.begin(), symbols.end(), _textLabel->text.Get()[newCursorPos]) != symbols.end())
                            skippingSymbols = true;
                        else if (std::find(newline.begin(), newline.end(), _textLabel->text.Get()[newCursorPos]) != newline.end())
                            skippingNewline = true;
                        else
                            skippingWord = true;
                    }

                    while (newCursorPos > 0)
                    {
                        newCursorPos--;
                        if (_textLabel->text.Get()[newCursorPos] != L' ')
                        {
                            bool symbolChar = std::find(symbols.begin(), symbols.end(), _textLabel->text.Get()[newCursorPos]) != symbols.end();
                            bool newlineChar = std::find(newline.begin(), newline.end(), _textLabel->text.Get()[newCursorPos]) != newline.end();

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
                    if (!selecting && selStart != selEnd)
                        newCursorPos = std::min(selStart, selEnd);
                    else if (newCursorPos > 0)
                        newCursorPos--;
                }

                _UpdateSelection(newCursorPos, selecting);
                cursorPos_ = newCursorPos;
                _UpdateTargetCursorXPos();
                _MoveViewToCursor();
                caretTimer_->Reset();
                _textPanel->InvokeRedraw();
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

                size_t selStart = _textLabel->selectionStart;
                size_t selEnd = _textLabel->selectionEnd;

                // Calculate new cursor pos
                size_t newCursorPos = cursorPos_;
                if (wordMode && newCursorPos < _textLabel->text->length())
                {
                    bool skippingWhitespace = false;
                    bool skippingWord = false;
                    bool skippingSymbols = false;
                    bool skippingNewline = false;

                    if (_textLabel->text.Get()[newCursorPos] == L' ')
                    {
                        skippingWhitespace = true;
                    }
                    else
                    {
                        if (std::find(symbols.begin(), symbols.end(), _textLabel->text.Get()[newCursorPos]) != symbols.end())
                            skippingSymbols = true;
                        else if (std::find(newline.begin(), newline.end(), _textLabel->text.Get()[newCursorPos]) != newline.end())
                            skippingNewline = true;
                        else
                            skippingWord = true;
                    }
                    newCursorPos++;

                    while (newCursorPos < _textLabel->text->length())
                    {
                        if (_textLabel->text.Get()[newCursorPos] != L' ')
                        {
                            if (skippingWhitespace)
                                break;

                            bool symbolChar = std::find(symbols.begin(), symbols.end(), _textLabel->text.Get()[newCursorPos]) != symbols.end();
                            bool newlineChar = std::find(newline.begin(), newline.end(), _textLabel->text.Get()[newCursorPos]) != newline.end();

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
                    if (!selecting && selStart != selEnd)
                        newCursorPos = std::max(selStart, selEnd);
                    else if (newCursorPos < _textLabel->text->length())
                        newCursorPos++;
                }

                _UpdateSelection(newCursorPos, selecting);
                cursorPos_ = newCursorPos;
                _UpdateTargetCursorXPos();
                _MoveViewToCursor();
                caretTimer_->Reset();
                _textPanel->InvokeRedraw();
                break;
            }
            case VK_UP:
            {
                bool selecting = false;
                if (KeyState(VK_UP, KMOD_SHIFT))
                    selecting = true;

                size_t newCursorPos = cursorPos_;

                auto hitTestResult = _textLabel->HitTestTextPosition(cursorPos_);
                auto lineMetrics = _textLabel->GetLineMetrics().lineMetrics;
                size_t lineIndex = _CurrentLineIndex(lineMetrics);

                // Move caret a line up
                if (lineIndex > 0)
                {
                    float aboveLineHeight = lineMetrics[lineIndex - 1].height;
                    float testPointYPos = hitTestResult.position.y - aboveLineHeight * 0.5f;

                    auto result = _textLabel->HitTestPoint({ _targetCursorXPos, testPointYPos });
                    size_t position = result.hitMetrics.textPosition;
                    if (result.isTrailingHit)
                        position++;

                    if (position != newCursorPos)
                    {
                        newCursorPos = position;
                    }
                }

                _UpdateSelection(newCursorPos, selecting);
                cursorPos_ = newCursorPos;
                _MoveViewToCursor();
                caretTimer_->Reset();
                _textPanel->InvokeRedraw();
                break;
            }
            case VK_DOWN:
            {
                bool selecting = false;
                if (KeyState(VK_DOWN, KMOD_SHIFT))
                    selecting = true;

                size_t newCursorPos = cursorPos_;

                auto hitTestResult = _textLabel->HitTestTextPosition(cursorPos_);
                auto lineMetrics = _textLabel->GetLineMetrics().lineMetrics;
                size_t lineIndex = _CurrentLineIndex(lineMetrics);

                // Move caret a line down
                if (lineIndex < lineMetrics.size() - 1)
                {
                    float thisLineHeight = lineMetrics[lineIndex].height;
                    float belowLineHeight = lineMetrics[lineIndex + 1].height;
                    float testPointYPos = hitTestResult.position.y + thisLineHeight + belowLineHeight * 0.5f;

                    auto result = _textLabel->HitTestPoint({ _targetCursorXPos, testPointYPos });
                    size_t position = result.hitMetrics.textPosition;
                    if (result.isTrailingHit)
                        position++;

                    if (position != newCursorPos)
                    {
                        newCursorPos = position;
                    }
                }

                _UpdateSelection(newCursorPos, selecting);
                cursorPos_ = newCursorPos;
                _MoveViewToCursor();
                caretTimer_->Reset();
                _textPanel->InvokeRedraw();
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

                size_t newCursorPos = cursorPos_;

                if (pageMode)
                {
                    newCursorPos = 0;
                }
                else
                {
                    auto lineMetrics = _textLabel->GetLineMetrics().lineMetrics;
                    auto linePositions = _LineStartPositions(lineMetrics);
                    size_t lineIndex = _CurrentLineIndex(lineMetrics);

                    size_t startPosition = linePositions[lineIndex];
                    // Move start position after whitespace
                    while (startPosition < _textLabel->text->length() && _textLabel->text.Get()[startPosition] == L' ')
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
                cursorPos_ = newCursorPos;
                _UpdateTargetCursorXPos();
                _MoveViewToCursor();
                caretTimer_->Reset();
                _textPanel->InvokeRedraw();
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

                size_t newCursorPos = cursorPos_;

                if (pageMode)
                {
                    newCursorPos = text->length();
                }
                else
                {
                    auto lineMetrics = _textLabel->GetLineMetrics().lineMetrics;
                    auto linePositions = _LineStartPositions(lineMetrics);
                    size_t lineIndex = _CurrentLineIndex(lineMetrics);

                    // Move cursor to end of current line, before line end characters
                    newCursorPos = linePositions[lineIndex] + lineMetrics[lineIndex].length - lineMetrics[lineIndex].newlineLength;
                }

                _UpdateSelection(newCursorPos, selecting);
                cursorPos_ = newCursorPos;
                _UpdateTargetCursorXPos();
                _MoveViewToCursor();
                caretTimer_->Reset();
                _textPanel->InvokeRedraw();
                break;
            }
            case 'A':
            {
                if (KeyState('A', KMOD_CONTROL))
                {
                    cursorPos_ = _textLabel->text->length();
                    _textLabel->selectionStart = 0;
                    _textLabel->selectionEnd = cursorPos_.Get();
                }
                break;
            }
            case 'C':
            {
                if (KeyState('C', KMOD_CONTROL))
                {
                    size_t selStart = 0;
                    size_t selLength = 0;
                    if (_textLabel->selectionStart > _textLabel->selectionEnd)
                    {
                        selStart = _textLabel->selectionEnd;
                        selLength = _textLabel->selectionStart - _textLabel->selectionEnd;
                    }
                    else
                    {
                        selStart = _textLabel->selectionStart;
                        selLength = _textLabel->selectionEnd - _textLabel->selectionStart;
                    }

                    if (selLength != 0)
                    {
                        // TODO: Add CopyToClipboard method to Label component and use it here instead of repeating logic

                        // If input is masked, copy the mask characters
                        const std::wstring& text = _textLabel->text;
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
                if (tabAllowed)
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
                if (tabAllowed)
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
                std::wstring newText = text;
                size_t newCursorPos = cursorPos_;

                size_t selStart = 0;
                size_t selLength = 0;
                if (_textLabel->selectionStart > _textLabel->selectionEnd)
                {
                    selStart = _textLabel->selectionEnd;
                    selLength = _textLabel->selectionStart - _textLabel->selectionEnd;
                }
                else
                {
                    selStart = _textLabel->selectionStart;
                    selLength = _textLabel->selectionEnd - _textLabel->selectionStart;
                }

                auto EraseSelectedText = [&]()
                {
                    newText.erase(selStart, selLength);
                    newCursorPos = selStart;
                    _textLabel->selectionStart = 0;
                    _textLabel->selectionEnd = 0;
                    _textPanel->InvokeRedraw();
                };

                switch (vkCode)
                {
                case VK_BACK:
                {
                    if (selLength != 0)
                    {
                        EraseSelectedText();
                    }
                    else if (cursorPos_ > 0)
                    {
                        newText.erase(newText.begin() + cursorPos_ - 1);
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
                    else if (cursorPos_ < newText.length())
                    {
                        newText.erase(newText.begin() + cursorPos_);
                    }
                    break;
                }
                case VK_RETURN:
                {
                    if (multiline)
                    {
                        // TODO: also delete selected text

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
                            std::wstring copyTextW = _textLabel->text->substr(selStart, selLength);
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

                if (newText != text && _TextMatches(newText, pattern))
                {
                    _settingTypedText = true;
                    text = newText;
                    _settingTypedText = false;
                    cursorPos_ = newCursorPos;
                    _UpdateTargetCursorXPos();
                }
                caretTimer_->Reset();
                _MoveViewToCursor();
            }

            return true;
        }

        bool _OnKeyUp(BYTE vkCode) override
        {
            switch (vkCode)
            {
            case VK_TAB:
            {
                if (tabAllowed)
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
                if (tabAllowed)
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

        bool _OnChar(wchar_t ch) override
        {
            std::wstring newText = text;
            size_t newCursorPos = cursorPos_;

            size_t selStart = 0;
            size_t selLength = 0;
            if (_textLabel->selectionStart > _textLabel->selectionEnd)
            {
                selStart = _textLabel->selectionEnd;
                selLength = _textLabel->selectionStart - _textLabel->selectionEnd;
            }
            else
            {
                selStart = _textLabel->selectionStart;
                selLength = _textLabel->selectionEnd - _textLabel->selectionStart;
            }

            auto EraseSelectedText = [&]()
            {
                newText.erase(selStart, selLength);
                newCursorPos = selStart;
                _textLabel->selectionStart = 0;
                _textLabel->selectionEnd = 0;
                _textPanel->InvokeRedraw();
            };

            bool handled = false;
            if (ch == L'\t')
            {
                if (!tabAllowed)
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

            if (_TextMatches(newText, pattern))
            {
                _settingTypedText = true;
                text = newText;
                _settingTypedText = false;
                cursorPos_ = newCursorPos;
            }

            caretTimer_->Reset();
            _UpdateTargetCursorXPos();
            _MoveViewToCursor();
            return true;
        }

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicBoolValueProxy("multiline", std::make_any<Value<bool>*>(&multiline)));
            values.push_back(ValueProxy::BasicBoolValueProxy("tab allowed", std::make_any<Value<bool>*>(&tabAllowed)));
            values.push_back(ValueProxy::BasicTextValueProxy("pattern", std::make_any<Value<std::wstring>*>(&pattern)));
            values.push_back(ValueProxy::BasicEnumValueProxy<MatchEnforcing>("match enforcing", std::make_any<Value<MatchEnforcing>*>(&matchEnforcing), MatchEnforcingValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicBoolValueProxy("hide caret", std::make_any<Value<bool>*>(&hideCaret)));
            //values.push_back(ValueProxy::BasicIntValueProxy<size_t>("cursor position", std::make_any<Value<size_t>*>(&cursorPos_), ValueProxy::Number(0)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("caret visible", std::make_any<Value<bool>*>(&caretVisible_)).Computed());

            auto data = Panel::GetReflectionData();
            data.insert(data.begin(), { "Text input", std::move(values) });
            return data;
        }
    };
}