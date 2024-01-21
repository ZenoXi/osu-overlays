#pragma once

#include "ComponentBase.h"
#include "../ComHelper.h"
#include "Window/KeyboardEventHandler.h"

#include "Helper/StringHelper.h"

#include <string_view>

namespace zcom
{
    enum class TextAlignment
    {
        LEADING,
        CENTER,
        JUSTIFIED,
        TRAILING
    };

    struct LineMetricsResult
    {
        std::vector<DWRITE_LINE_METRICS> lineMetrics;
    };

    struct TextPositionHitResult
    {
        FLOAT posX;
        FLOAT posY;
        DWRITE_HIT_TEST_METRICS hitMetrics;
    };

    struct TextRangeHitResult
    {
        std::vector<DWRITE_HIT_TEST_METRICS> hitMetrics;
    };

    struct HitTestResult
    {
        BOOL isTrailingHit = false;
        BOOL isInside = false;
        DWRITE_HIT_TEST_METRICS hitMetrics;
    };

    // To enable ClearType, the ignore alpha parameter in the component base must be set to true
    class Label : public Component, public KeyboardEventHandler
    {
#pragma region base_class
    protected:
        void _OnDraw(Graphics g)
        {
            // Create resources
            if (!_textBrush)
            {
                g.target->CreateSolidColorBrush(_fontColor, &_textBrush);
                g.refs->push_back({ (IUnknown**)&_textBrush, std::string("Label text brush. Text: ") + wstring_to_string(_text) });
            }

            // Get selected area
            TextRangeHitResult result;
            if (_selectionLength < 0)
                result = HitTestTextRange(_selectionStart + _selectionLength, -_selectionLength);
            else if (_selectionLength > 0)
                result = HitTestTextRange(_selectionStart, _selectionLength);

            // Draw selection background
            if (!result.hitMetrics.empty())
            {
                ID2D1SolidColorBrush* brush = nullptr;
                g.target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DodgerBlue, 0.5f), &brush);

                for (auto& metric : result.hitMetrics)
                {
                    D2D1_RECT_F rect;
                    rect.left = metric.left;
                    rect.top = metric.top;
                    rect.right = rect.left + metric.width;
                    rect.bottom = rect.top + metric.height;
                    g.target->FillRectangle(rect, brush);
                }

                brush->Release();
            }

            DWRITE_TEXT_METRICS textMetrics;
            _dwriteTextLayout->GetMetrics(&textMetrics);

            D2D1_POINT_2F pos;
            pos.x = _margins.left;
            pos.y = _TextTopPos();

            // Draw text
            if (!_text.empty())
            {
                g.target->DrawTextLayout(
                    pos,
                    _dwriteTextLayout,
                    _textBrush
                );
            }
        }

        void _OnResize(int width, int height)
        {
            _CreateTextLayout();
        }

        EventTargets _OnLeftPressed(int x, int y)
        {
            if (_textSelectable)
            {
                // Get click text position
                auto result = HitTestPoint((float)x, (float)y);
                _selectionStart = result.hitMetrics.textPosition;
                if (result.isTrailingHit)
                    _selectionStart++;
                _selectionLength = 0;
                _selecting = true;

                InvokeRedraw();
            }
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnLeftReleased(int x, int y)
        {
            _selecting = false;
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnMouseMove(int deltaX, int deltaY)
        {
            if (deltaX == 0 && deltaY == 0)
                return EventTargets().Add(this, GetMousePosX(), GetMousePosY());

            if (_selecting)
            {
                auto result = HitTestPoint((float)GetMousePosX(), (float)GetMousePosY());
                int currentTextPosition = result.hitMetrics.textPosition;
                if (result.isTrailingHit)
                    currentTextPosition++;
                int selectionLength = currentTextPosition - _selectionStart;

                if (selectionLength != _selectionLength)
                {
                    _selectionLength = selectionLength;
                    InvokeRedraw();
                }
            }

            return EventTargets().Add(this, GetMousePosX(), GetMousePosY());
        }

        void _OnSelected(bool reverse);

        void _OnDeselected();

        bool _OnHotkey(int id)
        {
            return false;
        }

        bool _OnKeyDown(BYTE vkCode)
        {
            if (vkCode == 'C' && KeyState('C', KMOD_CONTROL))
            {
                int selStart = _selectionStart;
                int selLength = _selectionLength;
                if (selLength != 0)
                {
                    if (selLength < 0)
                    {
                        selStart = selStart + selLength;
                        selLength = -selLength;
                    }
                }

                if (selLength != 0)
                {
                    std::wstring copyTextW = _text.substr(selStart, selLength);
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
                return true;
            }

            return false;
        }

        bool _OnKeyUp(BYTE vkCode)
        {
            return false;
        }

        bool _OnChar(wchar_t ch)
        {
            return false;
        }

        void _CreateTextFormat()
        {
            if (_dwriteTextFormat)
            {
                _dwriteTextFormat->Release();
            }

            _dwriteFactory->CreateTextFormat(
                _font.c_str(),
                NULL,
                _fontWeight,
                _fontStyle,
                _fontStretch,
                _fontSize,
                L"en-us",
                &_dwriteTextFormat
            );

            InvokeRedraw();
            _textFormatChangedEvent->InvokeAll(this);
        }

        void _CreateTextLayout()
        {
            float finalWidth = GetWidth() - _margins.left - _margins.right;
            float finalHeight = GetHeight() - _margins.top - _margins.bottom;
            if (finalWidth <= 0) finalWidth = 1.f;
            if (finalHeight <= 0) finalHeight = 1.f;

            std::wstring finalText = _text;
            size_t charactersCut = 0;

            while (true)
            {
                if (_dwriteTextLayout)
                {
                    _dwriteTextLayout->Release();
                }

                // Create the text layout
                _dwriteFactory->CreateTextLayout(
                    finalText.c_str(),
                    (UINT32)finalText.length(),
                    _dwriteTextFormat,
                    finalWidth,
                    finalHeight,
                    &_dwriteTextLayout
                );

                // Wrapping
                if (!_wrapText)
                {
                    _dwriteTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
                }

                // If a cutoff is specified, truncate the text until it fits (including the cutoff sequence)
                if (!_cutoff.empty())
                {
                    // OPTIMIZATION: Use binary search to speed up truncation of long strings

                    DWRITE_TEXT_METRICS textMetrics;
                    _dwriteTextLayout->GetMetrics(&textMetrics);
                    if (textMetrics.width > textMetrics.layoutWidth ||
                        (textMetrics.height > textMetrics.layoutHeight && textMetrics.lineCount > 1))
                    {
                        // Stop if the entire string is cut
                        if (charactersCut == _text.length())
                            break;

                        charactersCut++;
                        finalText = _text.substr(0, _text.length() - charactersCut) + _cutoff;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            if (!_customHoverText)
            {
                // Set hover text if contents are cut off
                if (charactersCut > 0)
                    Component::SetHoverText(_text);
                else
                    Component::SetHoverText(L"");
            }

            // Alignment
            if (_hTextAlignment == TextAlignment::LEADING)
            {
                _dwriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            }
            else if (_hTextAlignment == TextAlignment::CENTER)
            {
                _dwriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            }
            else if (_hTextAlignment == TextAlignment::JUSTIFIED)
            {
                _dwriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
            }
            else if (_hTextAlignment == TextAlignment::TRAILING)
            {
                _dwriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
            }

            // Effects
            if (_underlineRange.length != 0)
                _dwriteTextLayout->SetUnderline(true, _underlineRange);
            if (_strikethroughRange.length != 0)
                _dwriteTextLayout->SetStrikethrough(true, _strikethroughRange);

            InvokeRedraw();
            _textLayoutChangedEvent->InvokeAll(this);
        }

    public:
        const char* GetName() const { return Name(); }
        static const char* Name() { return "label"; }
#pragma endregion

    private:
        std::wstring _text;
        TextAlignment _hTextAlignment = TextAlignment::LEADING;
        Alignment _vTextAlignment = Alignment::START;
        bool _wrapText = false;
        std::wstring _cutoff = L"";
        RECT_F _margins = { 0, 0, 0, 0 };
        bool _customHoverText = false;

        std::wstring _font = L"Calibri";
        float _fontSize = 14.0f;
        DWRITE_FONT_WEIGHT _fontWeight = DWRITE_FONT_WEIGHT_REGULAR;
        DWRITE_FONT_STYLE _fontStyle = DWRITE_FONT_STYLE_NORMAL;
        DWRITE_FONT_STRETCH _fontStretch = DWRITE_FONT_STRETCH_NORMAL;
        D2D1_COLOR_F _fontColor = D2D1::ColorF(0.8f, 0.8f, 0.8f);

        DWRITE_TEXT_RANGE _underlineRange = { 0, 0 };
        DWRITE_TEXT_RANGE _strikethroughRange = { 0, 0 };

        int _selectionStart = 0;
        int _selectionLength = 0;
        bool _selecting = false;
        bool _textSelectable = false;

        ID2D1SolidColorBrush* _textBrush = nullptr;

        IDWriteFactory* _dwriteFactory = nullptr;
        IDWriteTextFormat* _dwriteTextFormat = nullptr;
        IDWriteTextLayout* _dwriteTextLayout = nullptr;

        EventEmitter<void, Label*, std::wstring*> _textChangedEvent;
        EventEmitter<void, Label*> _textFormatChangedEvent;
        EventEmitter<void, Label*> _textLayoutChangedEvent;

    protected:
        friend class Scene;
        friend class Component;
        Label(Scene* scene) : Component(scene) {}
        void Init(std::wstring text = L"")
        {
            _text = text;

            SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));

            // Create text rendering resources
            DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&_dwriteFactory)
            );
            _CreateTextFormat();
            _CreateTextLayout();
        }
    public:
        ~Label()
        {
            SafeFullRelease((IUnknown**)&_textBrush);
            SafeFullRelease((IUnknown**)&_dwriteTextFormat);
            SafeFullRelease((IUnknown**)&_dwriteTextLayout);
            SafeFullRelease((IUnknown**)&_dwriteFactory);
        }
        Label(Label&&) = delete;
        Label& operator=(Label&&) = delete;
        Label(const Label&) = delete;
        Label& operator=(const Label&) = delete;

        std::wstring GetText() const
        {
            return _text;
        }

        TextAlignment GetHorizontalTextAlignment() const
        {
            return _hTextAlignment;
        }

        Alignment GetVerticalTextAlignment() const
        {
            return _vTextAlignment;
        }

        bool GetWordWrap() const
        {
            return _wrapText;
        }

        std::wstring GetCutoff() const
        {
            return _cutoff;
        }

        RECT_F GetMargins() const
        {
            return _margins;
        }

        std::wstring GetFont() const
        {
            return _font;
        }

        float GetFontSize() const
        {
            return _fontSize;
        }

        DWRITE_FONT_WEIGHT GetFontWeight() const
        {
            return _fontWeight;
        }

        DWRITE_FONT_STYLE GetFontStyle() const
        {
            return _fontStyle;
        }

        DWRITE_FONT_STRETCH GetFontStretch() const
        {
            return _fontStretch;
        }

        D2D1_COLOR_F GetFontColor() const
        {
            return _fontColor;
        }

        float GetTextWidth() const
        {
            DWRITE_TEXT_METRICS textMetrics;
            _dwriteTextLayout->GetMetrics(&textMetrics);
            return textMetrics.width + _margins.left + _margins.right;
        }

        float GetTextHeight() const
        {
            DWRITE_TEXT_METRICS textMetrics;
            _dwriteTextLayout->GetMetrics(&textMetrics);
            return textMetrics.height + _margins.top + _margins.bottom;
        }

        bool GetTextSelectable() const
        {
            return _textSelectable;
        }

        int GetSelectionStart() const
        {
            return _selectionStart;
        }

        int GetSelectionLength() const
        {
            return _selectionLength;
        }

        void SetText(std::wstring text)
        {
            if (text == _text)
                return;

            // 'text' can be modified by the handlers
            _textChangedEvent->InvokeAll(this, &text);

            _text = text;
            SetSelectionStart(0);
            SetSelectionLength(0);
            _CreateTextLayout();
        }

        void SetHorizontalTextAlignment(TextAlignment alignment)
        {
            if (_hTextAlignment != alignment)
            {
                _hTextAlignment = alignment;
                _CreateTextLayout();
            }
        }

        void SetVerticalTextAlignment(Alignment alignment)
        {
            _vTextAlignment = alignment;
            InvokeRedraw();
        }

        void SetWordWrap(bool wrap)
        {
            if (_wrapText != wrap)
            {
                _wrapText = wrap;
                _CreateTextLayout();
            }
        }

        // If set to a non-empty string, the text will be truncated to fit within the boundaries.
        // 'cutoff' - The string appended to the end of truncated text (e.g. "trunca..." if 'cutoff' is "...").
        void SetCutoff(std::wstring cutoff)
        {
            if (_cutoff != cutoff)
            {
                _cutoff = cutoff;
                _CreateTextLayout();
            }
        }

        void SetMargins(RECT_F margins)
        {
            if (_margins != margins)
            {
                _margins = margins;
                _CreateTextLayout();
            }
        }

        void SetFont(std::wstring font)
        {
            if (_font != font)
            {
                _font = font;
                _CreateTextFormat();
                _CreateTextLayout();
            }
        }

        void SetFontSize(float size)
        {
            if (_fontSize != size)
            {
                _fontSize = size;
                _CreateTextFormat();
                _CreateTextLayout();
            }
        }

        void SetFontWeight(DWRITE_FONT_WEIGHT weight)
        {
            if (_fontWeight != weight)
            {
                _fontWeight = weight;
                _CreateTextFormat();
                _CreateTextLayout();
            }
        }

        void SetFontStyle(DWRITE_FONT_STYLE style)
        {
            if (_fontStyle != style)
            {
                _fontStyle = style;
                _CreateTextFormat();
                _CreateTextLayout();
            }
        }

        void SetFontStretch(DWRITE_FONT_STRETCH stretch)
        {
            if (_fontStretch != stretch)
            {
                _fontStretch = stretch;
                _CreateTextFormat();
                _CreateTextLayout();
            }
        }

        void SetFontColor(D2D1_COLOR_F color)
        {
            _fontColor = color;
            SafeFullRelease((IUnknown**)&_textBrush);
            InvokeRedraw();
        }

        void SetUnderline(DWRITE_TEXT_RANGE range)
        {
            if (range.length == _underlineRange.length &&
                range.startPosition == _underlineRange.startPosition)
                return;

            _underlineRange = range;
            _CreateTextLayout();
        }

        void SetStrikethrough(DWRITE_TEXT_RANGE range)
        {
            if (range.length == _strikethroughRange.length &&
                range.startPosition == _strikethroughRange.startPosition)
                return;

            _strikethroughRange = range;
            _CreateTextLayout();
        }

        void SetTextSelectable(bool selectable)
        {
            if (selectable == _textSelectable)
                return;

            _textSelectable = selectable;
            if (_textSelectable)
            {
                SetDefaultCursor(zwnd::CursorIcon::IBEAM);
                SetSelectable(true);
            }
            else
            {
                SetDefaultCursor(zwnd::CursorIcon::ARROW);
                SetSelectable(false);
                _selecting = false;
                _selectionStart = 0;
                _selectionLength = 0;
            }
            InvokeRedraw();
        }

        void SetSelectionStart(int selectionStart)
        {
            if (!_textSelectable)
                return;
            if (selectionStart == _selectionStart)
                return;

            _selectionStart = selectionStart;
            InvokeRedraw();
        }

        void SetSelectionLength(int selectionLength)
        {
            if (!_textSelectable)
                return;
            if (selectionLength == _selectionLength)
                return;

            _selectionLength = selectionLength;
            InvokeRedraw();
        }

        void SetHoverText(std::wstring text)
        {
            if (!text.empty())
                _customHoverText = true;
            else
                _customHoverText = false;

            Component::SetHoverText(text);
        }

        LineMetricsResult LineMetrics() const
        {
            UINT32 lineCount;
            _dwriteTextLayout->GetLineMetrics(nullptr, 0, &lineCount);
            if (lineCount == 0)
                return LineMetricsResult{};

            std::vector<DWRITE_LINE_METRICS> metrics;
            metrics.resize(lineCount);
            _dwriteTextLayout->GetLineMetrics(metrics.data(), metrics.size(), &lineCount);

            return LineMetricsResult{ metrics };
        }

        DWRITE_TEXT_METRICS TextMetrics() const
        {
            DWRITE_TEXT_METRICS metrics;
            _dwriteTextLayout->GetMetrics(&metrics);
            return metrics;
        }

    protected:
        float _TextTopPos() const
        {
            auto metrics = TextMetrics();
            if (_vTextAlignment == Alignment::START)
                return _margins.top;
            else if (_vTextAlignment == Alignment::CENTER)
                return _margins.top + ((GetHeight() - _margins.top - _margins.bottom) - metrics.height) * 0.5f;
            else if (_vTextAlignment == Alignment::END)
                return _margins.top + GetHeight() - metrics.height - _margins.bottom;
        }
    public:

        HitTestResult HitTestPoint(float x, float y)
        {
            HitTestResult result;
            _dwriteTextLayout->HitTestPoint(x - _margins.left, y - _TextTopPos(), &result.isTrailingHit, &result.isInside, &result.hitMetrics);
            return result;
        }

        TextPositionHitResult HitTestTextPosition(uint32_t textPosition, bool isTrailingHit = false) const
        {
            TextPositionHitResult metrics;
            _dwriteTextLayout->HitTestTextPosition(
                textPosition,
                isTrailingHit,
                &metrics.posX, &metrics.posY,
                &metrics.hitMetrics
            );
            metrics.posX += _margins.left;
            metrics.posY += _TextTopPos();
            return metrics;
        }

        TextRangeHitResult HitTestTextRange(uint32_t textPosition, uint32_t textLength) const
        {
            std::vector<DWRITE_HIT_TEST_METRICS> metricsArray;

            auto metrics = TextMetrics();
            metricsArray.resize((size_t)metrics.lineCount * metrics.maxBidiReorderingDepth);
            
            while (true)
            {
                // Arbitrarily large limit
                if (metricsArray.size() > 10000000)
                    return {};

                uint32_t actualCount;
                HRESULT hr = _dwriteTextLayout->HitTestTextRange(
                    textPosition,
                    textLength,
                    _margins.left,
                    _TextTopPos(),
                    metricsArray.data(),
                    metricsArray.size(),
                    &actualCount
                );
                if (hr == E_NOT_SUFFICIENT_BUFFER)
                {
                    metricsArray.resize(metricsArray.size() * 1.5 + 1);
                    continue;
                }

                if (actualCount < metricsArray.size())
                    metricsArray.resize(actualCount);

                return { metricsArray };
            }
        }

        // Handler parameters:
        // - a pointer to the label object
        // - a reference to the new text string. This parameter can be modified
        EventSubscription<void, Label*, std::wstring*> SubscribeOnTextChanged(std::function<void(Label*, std::wstring*)> handler)
        {
            return _textChangedEvent->Subscribe(handler);
        }

        EventSubscription<void, Label*> SubscribeOnTextFormatChanged(std::function<void(Label*)> handler)
        {
            return _textFormatChangedEvent->Subscribe(handler);
        }

        EventSubscription<void, Label*> SubscribeOnTextLayoutChanged(std::function<void(Label*)> handler)
        {
            return _textLayoutChangedEvent->Subscribe(handler);
        }
    };
}