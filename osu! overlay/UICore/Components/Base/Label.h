#pragma once

#include "ComponentBase.h"
#include "../ComHelper.h"
#include "Window/KeyboardEventHandler.h"

#include "Helper/StringHelper.h"
#include "Helper/BinarySearchIterator.h"

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
        DWRITE_HIT_TEST_METRICS hitMetrics = {};
    };

    // To enable ClearType, the ignore alpha parameter in the component base must be set to true
    class Label : public Component, public KeyboardEventHandler
    {
        DEFINE_COMPONENT(Label, Component)
    public:
        ~Label()
        {
            SafeFullRelease((IUnknown**)&_textBrush);
            SafeFullRelease((IUnknown**)&_dwriteTextFormat);
            SafeFullRelease((IUnknown**)&_dwriteTextLayout);
            SafeFullRelease((IUnknown**)&_dwriteFactory);
        }
    protected:
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
            return _padding;
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
            return textMetrics.width + _padding.left + _padding.right;
        }

        float GetTextHeight() const
        {
            DWRITE_TEXT_METRICS textMetrics;
            _dwriteTextLayout->GetMetrics(&textMetrics);
            return textMetrics.height + _padding.top + _padding.bottom;
        }

        bool GetTextSelectable() const
        {
            return _textSelectable;
        }

        size_t GetSelectionStart() const
        {
            return _selectionStart;
        }

        size_t GetSelectionEnd() const
        {
            return _selectionEnd;
        }

        void SetText(std::wstring text)
        {
            if (text == _text)
                return;

            // 'text' can be modified by the handlers
            _textChangedEvent->InvokeAll(this, &text);

            _text = text;
            SetSelectionStart(0);
            SetSelectionEnd(0);
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

        void SetPadding(RECT_F padding)
        {
            if (_padding != padding)
            {
                _padding = padding;
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
                _selectionEnd = 0;
            }
            InvokeRedraw();
        }

        void SetSelectionStart(size_t selectionStart)
        {
            if (!_textSelectable)
                return;
            if (selectionStart == _selectionStart)
                return;

            _selectionStart = selectionStart;
            InvokeRedraw();
        }

        void SetSelectionEnd(size_t selectionEnd)
        {
            if (!_textSelectable)
                return;
            if (selectionEnd == _selectionEnd)
                return;

            _selectionEnd = selectionEnd;
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
            _dwriteTextLayout->GetLineMetrics(metrics.data(), (UINT32)metrics.size(), &lineCount);

            return LineMetricsResult{ metrics };
        }

        DWRITE_TEXT_METRICS TextMetrics() const
        {
            DWRITE_TEXT_METRICS metrics;
            _dwriteTextLayout->GetMetrics(&metrics);
            return metrics;
        }

        void AutomaticWidth()
        {
            if (!_autoWidth)
            {
                _autoWidth = true;
                _CreateTextLayout();
            }
        }

        void AutomaticHeight()
        {
            if (!_autoHeight)
            {
                _autoHeight = true;
                _CreateTextLayout();
            }
        }

        void AutomaticSize()
        {
            if (!_autoWidth || !_autoHeight)
            {
                _autoWidth = true;
                _autoHeight = true;
                _CreateTextLayout();
            }
        }

        void FixedWidth()
        {
            if (_autoWidth)
            {
                _autoWidth = false;
                _CreateTextLayout();
            }
        }

        void FixedHeight()
        {
            if (_autoHeight)
            {
                _autoHeight = false;
                _CreateTextLayout();
            }
        }

        void FixedSize()
        {
            if (_autoWidth || _autoHeight)
            {
                _autoWidth = false;
                _autoHeight = false;
                _CreateTextLayout();
            }
        }

        void SetMinAutoWidth(int minWidth)
        {
            if (_minAutoWidth == minWidth)
                return;

            _minAutoWidth = minWidth;
            if (_autoWidth)
                _CreateTextLayout();
        }

        void SetMinAutoHeight(int minHeight)
        {
            if (_minAutoHeight == minHeight)
                return;

            _minAutoHeight = minHeight;
            if (_autoHeight)
                _CreateTextLayout();
        }

        void SetMaxAutoWidth(int maxWidth)
        {
            if (_maxAutoWidth == maxWidth)
                return;

            _maxAutoWidth = maxWidth;
            if (_autoWidth)
                _CreateTextLayout();
        }

        void SetMaxAutoHeight(int maxHeight)
        {
            if (_maxAutoHeight == maxHeight)
                return;

            _maxAutoHeight = maxHeight;
            if (_autoHeight)
                _CreateTextLayout();
        }

        int GetMinAutoWidth() const
        {
            return _minAutoWidth;
        }

        int GetMinAutoHeight() const
        {
            return _minAutoHeight;
        }

        int GetMaxAutoWidth() const
        {
            return _maxAutoWidth;
        }

        int GetMaxAutoHeight() const
        {
            return _maxAutoHeight;
        }

        void SetWidthAutoResize(bool automatic)
        {
            SetAutoResize(automatic, _autoHeight);
        }

        void SetHeightAutoResize(bool automatic)
        {
            SetAutoResize(_autoWidth, automatic);
        }

        void SetAutoResize(bool autoWidth, bool autoHeight)
        {
            if (_autoWidth == autoWidth && _autoHeight == autoHeight)
                return;

            _autoWidth = autoWidth;
            _autoHeight = autoHeight;
            _CreateTextLayout();
        }

        bool IsWidthAutoResize() const
        {
            return _autoWidth;
        }

        bool IsHeightAutoResize() const
        {
            return _autoWidth;
        }

    protected:
        float _TextTopPos() const
        {
            auto metrics = TextMetrics();
            if (_vTextAlignment == Alignment::START)
                return _padding.top;
            else if (_vTextAlignment == Alignment::CENTER)
                return _padding.top + ((GetHeight() - _padding.top - _padding.bottom) - metrics.height) * 0.5f;
            else if (_vTextAlignment == Alignment::END)
                return _padding.top + GetHeight() - metrics.height - _padding.bottom;
            else
                return 0;
        }

    public:
        HitTestResult HitTestPoint(float x, float y)
        {
            HitTestResult result;
            _dwriteTextLayout->HitTestPoint(x - _padding.left, y - _TextTopPos(), &result.isTrailingHit, &result.isInside, &result.hitMetrics);
            return result;
        }

        TextPositionHitResult HitTestTextPosition(size_t textPosition, bool isTrailingHit = false) const
        {
            TextPositionHitResult metrics;
            _dwriteTextLayout->HitTestTextPosition(
                (UINT32)textPosition,
                isTrailingHit,
                &metrics.posX, &metrics.posY,
                &metrics.hitMetrics
            );
            metrics.posX += _padding.left;
            metrics.posY += _TextTopPos();
            return metrics;
        }

        TextRangeHitResult HitTestTextRange(size_t textPosition, size_t textLength) const
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
                    (UINT32)textPosition,
                    (UINT32)textLength,
                    _padding.left,
                    _TextTopPos(),
                    metricsArray.data(),
                    (UINT32)metricsArray.size(),
                    &actualCount
                );
                if (hr == E_NOT_SUFFICIENT_BUFFER)
                {
                    metricsArray.resize(size_t(metricsArray.size() * 1.5) + 1);
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

    private:
        std::wstring _text;
        TextAlignment _hTextAlignment = TextAlignment::LEADING;
        Alignment _vTextAlignment = Alignment::START;
        bool _wrapText = false;
        std::wstring _cutoff = L"";
        RECT_F _padding = { 0, 0, 0, 0 };
        bool _customHoverText = false;

        int _currentLayoutWidth = 0;
        int _currentLayoutHeight = 0;

        bool _autoHeight = false;
        bool _autoWidth = false;
        int _minAutoWidth = 0;
        int _minAutoHeight = 0;
        int _maxAutoWidth = std::numeric_limits<int>::max();
        int _maxAutoHeight = std::numeric_limits<int>::max();

        std::wstring _font = L"Calibri";
        float _fontSize = 14.0f;
        DWRITE_FONT_WEIGHT _fontWeight = DWRITE_FONT_WEIGHT_REGULAR;
        DWRITE_FONT_STYLE _fontStyle = DWRITE_FONT_STYLE_NORMAL;
        DWRITE_FONT_STRETCH _fontStretch = DWRITE_FONT_STRETCH_NORMAL;
        D2D1_COLOR_F _fontColor = D2D1::ColorF(0.8f, 0.8f, 0.8f);

        DWRITE_TEXT_RANGE _underlineRange = { 0, 0 };
        DWRITE_TEXT_RANGE _strikethroughRange = { 0, 0 };

        size_t _selectionStart = 0;
        size_t _selectionEnd = 0;
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
        void _OnDraw(Graphics g) override
        {
            // Create resources
            if (!_textBrush)
            {
                g.target->CreateSolidColorBrush(_fontColor, &_textBrush);
                g.refs->push_back({ (IUnknown**)&_textBrush, std::string("Label text brush. Text: ") + wstring_to_string(_text) });
            }

            // Get selected area
            TextRangeHitResult result;
            if (_selectionStart > _selectionEnd)
                result = HitTestTextRange(_selectionEnd, _selectionStart - _selectionEnd);
            else if (_selectionStart < _selectionEnd)
                result = HitTestTextRange(_selectionStart, _selectionEnd - _selectionStart);

            // Draw selection background
            if (!result.hitMetrics.empty())
            {
                ID2D1SolidColorBrush* brush = nullptr;
                g.target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DodgerBlue, 0.5f), &brush);

                if (brush)
                {
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
                else
                {
                    // TODO: Logging
                }
            }

            DWRITE_TEXT_METRICS textMetrics;
            _dwriteTextLayout->GetMetrics(&textMetrics);

            D2D1_POINT_2F pos;
            pos.x = _padding.left;
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

        void _OnResize(int width, int height) override
        {
            if (width != _currentLayoutWidth || height != _currentLayoutHeight)
                _CreateTextLayout(true);
        }

        EventTargets _OnLeftPressed(int x, int y) override
        {
            if (_textSelectable)
            {
                // Get click text position
                auto result = HitTestPoint((float)x, (float)y);
                _selectionStart = result.hitMetrics.textPosition;
                if (result.isTrailingHit)
                    _selectionStart++;
                _selectionEnd = _selectionStart;
                _selecting = true;

                InvokeRedraw();
            }
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnLeftReleased(int x, int y) override
        {
            _selecting = false;
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnMouseMove(int x, int y, int deltaX, int deltaY) override
        {
            if (deltaX == 0 && deltaY == 0)
                return EventTargets().Add(this, GetMousePosX(), GetMousePosY());

            if (_selecting)
            {
                auto result = HitTestPoint((float)GetMousePosX(), (float)GetMousePosY());
                size_t currentTextPosition = result.hitMetrics.textPosition;
                if (result.isTrailingHit)
                    currentTextPosition++;
                if (currentTextPosition != _selectionEnd)
                {
                    _selectionEnd = currentTextPosition;
                    InvokeRedraw();
                }
            }

            return EventTargets().Add(this, GetMousePosX(), GetMousePosY());
        }
 
        void _OnSelected(bool reverse) override;

        void _OnDeselected() override;

        bool _OnHotkey(int id) override
        {
            return false;
        }

        bool _OnKeyDown(BYTE vkCode) override
        {
            if (vkCode == 'C' && KeyState('C', KMOD_CONTROL))
            {
                size_t selStart = 0;
                size_t selLength = 0;
                if (_selectionStart > _selectionEnd)
                {
                    selStart = _selectionEnd;
                    selLength = _selectionStart - _selectionEnd;
                }
                else
                {
                    selStart = _selectionStart;
                    selLength = _selectionEnd - _selectionStart;
                }

                if (selLength != 0)
                {
                    std::wstring copyTextW = _text.substr(selStart, selLength);
                    std::string copyText = wstring_to_string(copyTextW);
                    copyTextW.resize(copyTextW.length() + 1);
                    copyText.resize(copyText.length() + 1);
                    // TODO: Move clipboard operations to separate OS utility class

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
                            if (hGlobalMem)
                            {
                                wchar_t* wstrMem = (wchar_t*)GlobalLock(hGlobalMem);
                                if (wstrMem)
                                    std::copy_n(copyTextW.data(), copyTextW.length(), wstrMem);
                                GlobalUnlock(hGlobalMem);
                                SetClipboardData(CF_UNICODETEXT, hGlobalMem);
                            }
                            else
                            {
                                // TODO: Logging
                            }
                        }
                        { // Add string
                            HGLOBAL hGlobalMem = GlobalAlloc(GMEM_MOVEABLE, copyText.length() * sizeof(char));
                            if (hGlobalMem)
                            {
                                wchar_t* strMem = (wchar_t*)GlobalLock(hGlobalMem);
                                if (strMem)
                                    std::copy_n(copyText.data(), copyText.length(), strMem);
                                GlobalUnlock(hGlobalMem);
                                SetClipboardData(CF_TEXT, hGlobalMem);
                            }
                            else
                            {
                                // TODO: Logging
                            }
                        }

                        CloseClipboard();
                    }
                }
                return true;
            }

            return false;
        }

        bool _OnKeyUp(BYTE vkCode) override
        {
            return false;
        }

        bool _OnChar(wchar_t ch) override
        {
            return false;
        }

        void _CreateTextFormat()
        {
            if (_dwriteTextFormat)
                _dwriteTextFormat->Release();

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

        void _CreateTextLayout(bool ignoreAutoSizing = false)
        {
            float finalWidth = GetWidth() - _padding.left - _padding.right;
            float finalHeight = GetHeight() - _padding.top - _padding.bottom;
            if (finalWidth <= 0) finalWidth = 1.f;
            if (finalHeight <= 0) finalHeight = 1.f;

            if (_dwriteTextLayout)
            {
                _dwriteTextLayout->Release();
                _dwriteTextLayout = nullptr;
            }

            if (_autoWidth || _autoHeight)
            {
                float newFinalWidth = finalWidth;
                float newFinalHeight = finalHeight;

                _dwriteFactory->CreateTextLayout(
                    _text.c_str(),
                    (UINT32)_text.length(),
                    _dwriteTextFormat,
                    finalWidth,
                    finalHeight,
                    &_dwriteTextLayout
                );
                _dwriteTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

                DWRITE_TEXT_METRICS textMetrics;
                _dwriteTextLayout->GetMetrics(&textMetrics);

                if (_autoWidth)
                {
                    newFinalWidth = std::ceilf(textMetrics.width);
                    if (newFinalWidth < _minAutoWidth)
                        newFinalWidth = (float)_minAutoWidth;
                    if (newFinalWidth > _maxAutoWidth)
                        newFinalWidth = (float)_maxAutoWidth;
                }
                if (_autoHeight)
                {
                    if (_wrapText)
                    {
                        _dwriteTextLayout->Release();
                        _dwriteFactory->CreateTextLayout(
                            _text.c_str(),
                            (UINT32)_text.length(),
                            _dwriteTextFormat,
                            newFinalWidth,
                            finalHeight,
                            &_dwriteTextLayout
                        );

                        _dwriteTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
                        _dwriteTextLayout->GetMetrics(&textMetrics);
                    }
                    newFinalHeight = std::ceilf(textMetrics.height);
                    if (newFinalHeight < _minAutoHeight)
                        newFinalHeight = (float)_minAutoHeight;
                    if (newFinalHeight > _maxAutoHeight)
                        newFinalHeight = (float)_maxAutoHeight;
                }

                if (finalWidth != newFinalWidth || finalHeight != newFinalHeight)
                {
                    _dwriteTextLayout->Release();
                    _dwriteTextLayout = nullptr;
                }

                finalWidth = newFinalWidth;
                finalHeight = newFinalHeight;
            }

            std::wstring finalText = _text;
            size_t charactersCut = 0;
            //BinarySearchIterator it = BinarySearchIterator(_text.size());

            while (true)
            {
                if (!_dwriteTextLayout)
                {
                    _dwriteFactory->CreateTextLayout(
                        finalText.c_str(),
                        (UINT32)finalText.length(),
                        _dwriteTextFormat,
                        finalWidth,
                        finalHeight,
                        &_dwriteTextLayout
                    );
                }

                _dwriteTextLayout->SetWordWrapping(_wrapText ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);

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

                if (_dwriteTextLayout)
                    _dwriteTextLayout->Release();
            }

            if (!_customHoverText)
            {
                // Set hover text if contents are cut off
                if (charactersCut > 0)
                    Component::SetHoverText(_text);
                else
                    Component::SetHoverText(L"");
            }

            if (_hTextAlignment == TextAlignment::LEADING)
                _dwriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            else if (_hTextAlignment == TextAlignment::CENTER)
                _dwriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            else if (_hTextAlignment == TextAlignment::JUSTIFIED)
                _dwriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
            else if (_hTextAlignment == TextAlignment::TRAILING)
                _dwriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

            if (_underlineRange.length != 0)
                _dwriteTextLayout->SetUnderline(true, _underlineRange);
            if (_strikethroughRange.length != 0)
                _dwriteTextLayout->SetStrikethrough(true, _strikethroughRange);

            _currentLayoutWidth = int(finalWidth + _padding.left + _padding.right);
            _currentLayoutHeight = int(finalHeight + _padding.top + _padding.bottom);
            if (_autoWidth || _autoHeight)
            {
                int newBaseWidth = _autoWidth ? _currentLayoutWidth : GetBaseWidth();
                int newBaseHeight = _autoHeight ? _currentLayoutHeight : GetBaseHeight();
                //std::cout << _currentLayoutWidth << ":" << _currentLayoutHeight << '\n';
                SetBaseSize(newBaseWidth, newBaseHeight);
            }

            InvokeRedraw();
            _textLayoutChangedEvent->InvokeAll(this);
        }
    };
}