#pragma once

#include "ComponentBase.h"
#include "../ComHelper.h"
#include "Window/Text.h"
#include "Window/KeyboardEventHandler.h"

#include "Helper/StringHelper.h"
#include "Helper/BinarySearchIterator.h"

#include <string_view>

namespace zcom
{
    constexpr std::vector<std::pair<int64_t, std::wstring>> TextAlignmentValueProxySelectionValues()
    {
        return {
            { (int64_t)TextAlignment::LEADING, L"Leading" },
            { (int64_t)TextAlignment::CENTER, L"Center" },
            { (int64_t)TextAlignment::JUSTIFIED, L"Justified" },
            { (int64_t)TextAlignment::TRAILING, L"Trailing" }
        };
    }

    // To enable ClearType, the ignore alpha parameter in the component base must be set to true
    class Label : public Component, public KeyboardEventHandler
    {
        DEFINE_COMPONENT(Label, Component)
    public:
        ~Label() {}
    protected:
        void Init(std::wstring text = L"")
        {
            this->text = text;

            border.selectedColor = Color();

            Component::hoverText.ComputedFrom([](
                const std::wstring& text,
                const std::wstring& labelHoverText,
                bool showTruncatedText,
                bool textTruncated
            ) {
                if (!showTruncatedText)
                    return labelHoverText;
                else if (textTruncated)
                    return text;
                else
                    return std::wstring();
            }, this->text, hoverText, showTruncatedText, _textTruncated);
        }

    public:
        Value<std::wstring> text = Value<std::wstring>(L"", [=](std::wstring& currentValue, const std::wstring& text) {
            std::wstring newText = text;
            _textChangedEvent->InvokeAll(this, &newText);
            currentValue = newText;

            selectionStart = 0;
            selectionEnd = 0;

            _textDesc.WithText(currentValue);
            _CalculateLayout();
        });
        Value<TextAlignment> xTextAlign = Value<TextAlignment>(TextAlignment::LEADING, [=](TextAlignment& currentValue, const TextAlignment& alignment) {
            currentValue = alignment;
            _textDesc.WithTextAlignment(currentValue);
            _CalculateLayout();
        });
        Value<Alignment> yTextAlign = Value<Alignment>(Alignment::START, [=](Alignment& currentValue, const Alignment& alignment) {
            currentValue = alignment;
            InvokeRedraw();
        });
        Value<WordWrapping> wordWrapping = Value<WordWrapping>(WordWrapping::NO_WRAP, [=](WordWrapping& currentValue, const WordWrapping& wrap) {
            currentValue = wrap;
            _textDesc.WithWrap(currentValue);
            _CalculateLayout();
        });
        // If set to a non-empty string, the text will be truncated to fit within the boundaries.
        // 'cutoff' - The string appended to the end of truncated text (e.g. "trunca..." if 'cutoff' is "...").
        Value<std::wstring> cutoff = Value<std::wstring>(L"", [=](std::wstring& currentValue, const std::wstring& cutoff) {
            currentValue = cutoff;
            _CalculateLayout();
        });
        Value<RectF> padding = Value<RectF>({ 0, 0, 0, 0 }, [=](RectF& currentValue, const RectF& rect) {
            currentValue = rect;
            _CalculateLayout();
        });
        Value<std::wstring> hoverText = std::wstring(L"");
        Value<bool> showTruncatedText = false;
    private:
        Value<bool> _textTruncated = false;
    public:
        Value<std::wstring> font = Value<std::wstring>(L"Calibri", [=](std::wstring& currentValue, const std::wstring& font) {
            currentValue = font;
            _textDesc.WithFontFamily(currentValue);
            _CalculateLayout();
        });
        Value<float> fontSize = Value<float>(14.0f, [=](float& currentValue, const float& fontSize) {
            currentValue = fontSize;
            _textDesc.WithFontSize(currentValue);
            _CalculateLayout();
        });
        Value<FontWeight> fontWeight = Value<FontWeight>(FontWeight::NORMAL, [=](FontWeight& currentValue, const FontWeight& fontWeight) {
            currentValue = fontWeight;
            _textDesc.WithFontWeight(currentValue);
            _CalculateLayout();
        });
        Value<FontStyle> fontStyle = Value<FontStyle>(FontStyle::NORMAL, [=](FontStyle& currentValue, const FontStyle& fontStyle) {
            currentValue = fontStyle;
            _textDesc.WithFontStyle(currentValue);
            _CalculateLayout();
        });
        Value<FontStretch> fontStretch = Value<FontStretch>(FontStretch::NORMAL, [=](FontStretch& currentValue, const FontStretch& fontStretch) {
            currentValue = fontStretch;
            _textDesc.WithFontStretch(currentValue);
            _CalculateLayout();
        });
        Value<Color> fontColor = Value<Color>(Color(0xD0D0D0), [=](Color& currentValue, const Color& fontColor) {
            currentValue = fontColor;
            InvokeRedraw();
        });
        
        Value<bool> autoWidth = Value<bool>(false, [=](bool& currentValue, const bool& autoWidth) {
            currentValue = autoWidth;
            _CalculateLayout();
        });
        Value<bool> autoHeight = Value<bool>(false, [=](bool& currentValue, const bool& autoHeight) {
            currentValue = autoHeight;
            _CalculateLayout();
        });
        Value<std::optional<int>> minAutoWidth = Value<std::optional<int>>(std::nullopt, [=](std::optional<int>& currentValue, const std::optional<int>& width) {
            currentValue = width;
            _CalculateLayout();
        });
        Value<std::optional<int>> minAutoHeight = Value<std::optional<int>>(std::nullopt, [=](std::optional<int>& currentValue, const std::optional<int>& height) {
            currentValue = height;
            _CalculateLayout();
        });
        Value<std::optional<int>> maxAutoWidth = Value<std::optional<int>>(std::nullopt, [=](std::optional<int>& currentValue, const std::optional<int>& width) {
            currentValue = width;
            _CalculateLayout();
        });
        Value<std::optional<int>> maxAutoHeight = Value<std::optional<int>>(std::nullopt, [=](std::optional<int>& currentValue, const std::optional<int>& height) {
            currentValue = height;
            _CalculateLayout();
        });
        
        Value<bool> textSelectable = Value<bool>(false, [=](bool& currentValue, const bool& selectable) {
            currentValue = selectable;
            if (textSelectable)
            {
                cursorIcon = zwnd::CursorIcon::IBEAM;
                Component::selectable = true;
            }
            else
            {
                cursorIcon = zwnd::CursorIcon::ARROW;
                Component::selectable = false;
                _selecting = false;
                selectionStart = 0;
                selectionEnd = 0;
            }
            InvokeRedraw();
        });
        Value<size_t> selectionStart = Value<size_t>(0, [=](size_t& currentValue, const size_t& pos) {
            currentValue = pos;
            InvokeRedraw();
        });
        Value<size_t> selectionEnd = Value<size_t>(0, [=](size_t& currentValue, const size_t& pos) {
            currentValue = pos;
            InvokeRedraw();
        });

        float GetTextWidth()
        {
            auto metrics = GetMetrics();
            return metrics.width + padding->left + padding->right;
        }

        float GetTextHeight()
        {
            auto metrics = GetMetrics();
            return metrics.height + padding->top + padding->bottom;
        }

        TextMetrics GetMetrics();
        TextLineMetricsResult GetLineMetrics();
        TextHitTestResult HitTestPoint(PointF point);
        TextPositionHitTestResult HitTestTextPosition(size_t textPosition, bool isTrailingHit = false);
        TextRangeHitTestResult HitTestTextRange(size_t textPosition, size_t textLength);

        PointF MapComponentToTextLayoutCoordinates(PointF point)
        {
            return { point.x - padding->left, point.y - _TextTopPos() };
        }

        PointF MapTextLayoutToComponentCoordinates(PointF point)
        {
            return { point.x + padding->left, point.y + _TextTopPos() };
        }

    protected:
        float _TextTopPos()
        {
            auto metrics = GetMetrics();
            if (yTextAlign == Alignment::START)
                return padding->top;
            else if (yTextAlign == Alignment::CENTER)
                return padding->top + ((size_->height - padding->top - padding->bottom) - metrics.height) * 0.5f;
            else if (yTextAlign == Alignment::END)
                return padding->top + size_->height - metrics.height - padding->bottom;
            else
                return 0;
        }

    public:
        // Handler parameters:
        // - a pointer to the label object
        // - a reference to the new text string. This parameter can be modified
        EventSubscription<void, Label*, std::wstring*> SubscribeOnTextChanged(std::function<void(Label*, std::wstring*)> handler)
        {
            return _textChangedEvent->Subscribe(handler);
        }

    private:
        Size _currentLayoutSize = { 0, 0 };

        bool _selecting = false;

        EventEmitter<void, Label*, std::wstring*> _textChangedEvent;

        TextDesc _textDesc;

    protected:
        void _OnDraw(Graphics* g) override
        {
            // Get selected area
            TextRangeHitTestResult result;
            if (selectionStart > selectionEnd)
                result = HitTestTextRange(selectionEnd, selectionStart - selectionEnd);
            else if (selectionStart < selectionEnd)
                result = HitTestTextRange(selectionStart, selectionEnd - selectionStart);

            // Draw selection background
            if (!result.hitMetrics.empty())
            {
                for (auto& metric : result.hitMetrics)
                {
                    RectF rect{};
                    rect.left = metric.left;
                    rect.top = metric.top;
                    rect.right = rect.left + metric.width;
                    rect.bottom = rect.top + metric.height;
                    Color dodgerBlue = Color(0x1E90FF, 0.5f);
                    g->FillRectangle(rect, dodgerBlue);
                }
            }

            // Draw text
            if (!text->empty())
                g->DrawTextLayout(_textDesc, PointF{ padding->left, _TextTopPos() }, fontColor);
        }

        void _OnResize(Size size) override
        {
            if (size != _currentLayoutSize)
                _CalculateLayout(true);
        }

        EventContext _OnLeftPressed(Point point) override
        {
            if (textSelectable)
            {
                // Get click text position
                auto result = HitTestPoint(point.ToPointF());
                selectionStart = result.hitMetrics.textPosition;
                if (result.isTrailingHit)
                    selectionStart = selectionStart + 1;
                selectionEnd = selectionStart.Get();
                _selecting = true;

                InvokeRedraw();
            }
            return EventContext().Add(this, point);
        }

        EventContext _OnLeftReleased(std::optional<Point> point) override
        {
            _selecting = false;
            return EventContext().Add(this, point);
        }

        EventContext _OnMouseMove(Point point, Point deltaPos) override
        {
            if (deltaPos == Point{ 0, 0 })
                return EventContext().Add(this, point);

            if (_selecting)
            {
                auto result = HitTestPoint(point.ToPointF());
                size_t currentTextPosition = result.hitMetrics.textPosition;
                if (result.isTrailingHit)
                    currentTextPosition++;
                if (currentTextPosition != selectionEnd)
                {
                    selectionEnd = currentTextPosition;
                    InvokeRedraw();
                }
            }

            return EventContext().Add(this, point);
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
                if (selectionStart > selectionEnd)
                {
                    selStart = selectionEnd;
                    selLength = selectionStart - selectionEnd;
                }
                else
                {
                    selStart = selectionStart;
                    selLength = selectionEnd - selectionStart;
                }

                if (selLength != 0)
                {
                    std::wstring copyTextW = text->substr(selStart, selLength);
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

        void _CalculateLayout(bool ignoreAutoSizing = false)
        {
            float xPadding = padding->left + padding->right;
            float yPadding = padding->top + padding->bottom;
            float finalWidth = size_->width - xPadding;
            float finalHeight = size_->height - yPadding;
            if (finalWidth <= 0) finalWidth = 1.f;
            if (finalHeight <= 0) finalHeight = 1.f;

            if (autoWidth || autoHeight)
            {
                float newFinalWidth = finalWidth;
                float newFinalHeight = finalHeight;

                _textDesc.WithLayoutSize({ newFinalWidth, newFinalHeight }).WithWrap(WordWrapping::NO_WRAP);
                TextMetrics textMetrics = GetMetrics();

                if (autoWidth)
                {
                    newFinalWidth = std::ceilf(textMetrics.width);
                    if (minAutoWidth->has_value() && newFinalWidth < minAutoWidth->value() - xPadding)
                        newFinalWidth = (float)minAutoWidth->value() - xPadding;
                    if (maxAutoWidth->has_value() && newFinalWidth > maxAutoWidth->value() - xPadding)
                        newFinalWidth = (float)maxAutoWidth->value() - xPadding;
                }
                if (autoHeight)
                {
                    if (wordWrapping != WordWrapping::NO_WRAP)
                    {
                        _textDesc.WithLayoutSize({ newFinalWidth, newFinalHeight }).WithWrap(wordWrapping);
                        textMetrics = GetMetrics();
                    }
                    newFinalHeight = std::ceilf(textMetrics.height);
                    if (minAutoHeight->has_value() && newFinalHeight < minAutoHeight->value() - yPadding)
                        newFinalHeight = (float)minAutoHeight->value() - yPadding;
                    if (maxAutoHeight->has_value() && newFinalHeight > maxAutoHeight->value() - yPadding)
                        newFinalHeight = (float)maxAutoHeight->value() - yPadding;
                }

                finalWidth = newFinalWidth;
                finalHeight = newFinalHeight;
            }

            std::wstring finalText = text;
            size_t charactersCut = 0;

            _textDesc.WithLayoutSize({ finalWidth, finalHeight }).WithWrap(wordWrapping);
            while (true)
            {
                _textDesc.WithText(finalText);

                // If a cutoff is specified, truncate the text until it fits (including the cutoff sequence)
                if (!cutoff->empty())
                {
                    // OPTIMIZATION: Use binary search to speed up truncation of long strings

                    TextMetrics textMetrics = GetMetrics();
                    if (textMetrics.width > textMetrics.layoutWidth ||
                        (textMetrics.height > textMetrics.layoutHeight && textMetrics.lineCount > 1))
                    {
                        // Stop if the entire string is cut
                        if (charactersCut == text->length())
                            break;

                        charactersCut++;
                        finalText = text->substr(0, text->length() - charactersCut) + cutoff.Get();
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

            _textTruncated = charactersCut > 0;

            _currentLayoutSize = {
                int(finalWidth + padding->left + padding->right),
                int(finalHeight + padding->top + padding->bottom)
            };
            if (autoWidth || autoHeight)
            {
                selfSize_ = Size{
                    autoWidth ? _currentLayoutSize.width : 0,
                    autoHeight ? _currentLayoutSize.height : 0
                };
            }

            InvokeRedraw();
        }

    public:

        constexpr std::vector<std::pair<int64_t, std::wstring>> FontWeightValueProxySelectionValues()
        {
            return {
                { (int64_t)FontWeight::LIGHT, L"Light" },
                { (int64_t)FontWeight::NORMAL, L"Normal" },
                { (int64_t)FontWeight::MEDIUM, L"Medium" },
                { (int64_t)FontWeight::SEMI_BOLD, L"Semi bold" },
                { (int64_t)FontWeight::BOLD, L"Bold" }
            };
        }
        constexpr std::vector<std::pair<int64_t, std::wstring>> FontStyleValueProxySelectionValues()
        {
            return {
                { (int64_t)FontStyle::NORMAL, L"Normal" },
                { (int64_t)FontStyle::OBLIQUE, L"Oblique" },
                { (int64_t)FontStyle::ITALIC, L"Italic" }
            };
        }
        constexpr std::vector<std::pair<int64_t, std::wstring>> FontStretchValueProxySelectionValues()
        {
            return {
                { (int64_t)FontStretch::CONDENSED, L"Condensed" },
                { (int64_t)FontStretch::NORMAL, L"Normal" },
                { (int64_t)FontStretch::EXPANDED, L"Expanded" }
            };
        }
        constexpr std::vector<std::pair<int64_t, std::wstring>> WordWrappingValueProxySelectionValues()
        {
            return {
                { (int64_t)WordWrapping::NO_WRAP, L"No wrap" },
                { (int64_t)WordWrapping::WRAP, L"Wrap" },
                { (int64_t)WordWrapping::EMERGENCY_BREAK, L"Emergency break" },
                { (int64_t)WordWrapping::WHOLE_WORD, L"Whole word" },
                { (int64_t)WordWrapping::CHARACTER, L"Character" }
            };
        }

        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;

            values.push_back(ValueProxy::BasicTextValueProxy("text", std::make_any<Value<std::wstring>*>(&text)));
            values.push_back(ValueProxy::BasicEnumValueProxy<TextAlignment>("x text align", std::make_any<Value<TextAlignment>*>(&xTextAlign), TextAlignmentValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicEnumValueProxy<Alignment>("y text align", std::make_any<Value<Alignment>*>(&yTextAlign), AlignmentValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicBoolValueProxy("auto width", std::make_any<Value<bool>*>(&autoWidth)));
            values.push_back(ValueProxy::BasicBoolValueProxy("auto height", std::make_any<Value<bool>*>(&autoHeight)));
            values.push_back(ValueProxy::BasicOptionalIntValueProxy<int>("min auto width", std::make_any<Value<std::optional<int>>*>(&minAutoWidth), ValueProxy::Number(0)));
            values.push_back(ValueProxy::BasicOptionalIntValueProxy<int>("min auto height", std::make_any<Value<std::optional<int>>*>(&minAutoHeight), ValueProxy::Number(0)));
            values.push_back(ValueProxy::BasicOptionalIntValueProxy<int>("max auto width", std::make_any<Value<std::optional<int>>*>(&maxAutoWidth), ValueProxy::Number(0)));
            values.push_back(ValueProxy::BasicOptionalIntValueProxy<int>("max auto height", std::make_any<Value<std::optional<int>>*>(&maxAutoHeight), ValueProxy::Number(0)));
            values.push_back(RectF::LeftValueProxy("left padding", std::make_any<Value<RectF>*>(&padding), 3));
            values.push_back(RectF::TopValueProxy("top padding", std::make_any<Value<RectF>*>(&padding), 3));
            values.push_back(RectF::RightValueProxy("right padding", std::make_any<Value<RectF>*>(&padding), 3));
            values.push_back(RectF::BottomValueProxy("bottom padding", std::make_any<Value<RectF>*>(&padding), 3));
            values.push_back(ValueProxy::BasicEnumValueProxy<WordWrapping>("word wrapping", std::make_any<Value<WordWrapping>*>(&wordWrapping), WordWrappingValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicTextValueProxy("font", std::make_any<Value<std::wstring>*>(&font)));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("font size", std::make_any<Value<float>*>(&fontSize), 3));
            values.push_back(ValueProxy::BasicEnumValueProxy<FontWeight>("font weight", std::make_any<Value<FontWeight>*>(&fontWeight), FontWeightValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicEnumValueProxy<FontStyle>("font style", std::make_any<Value<FontStyle>*>(&fontStyle), FontStyleValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicEnumValueProxy<FontStretch>("font stretch", std::make_any<Value<FontStretch>*>(&fontStretch), FontStretchValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicColorValueProxy("font color", std::make_any<Value<Color>*>(&fontColor)));
            //values.push_back(TextRangeValueProxy("underline [start length]", std::make_any<Value<DWRITE_TEXT_RANGE>*>(&underline)));
            //values.push_back(TextRangeValueProxy("strikethrough [start length]", std::make_any<Value<DWRITE_TEXT_RANGE>*>(&strikethrough)));
            values.push_back(ValueProxy::BasicTextValueProxy("cutoff", std::make_any<Value<std::wstring>*>(&cutoff)));
            values.push_back(ValueProxy::BasicTextValueProxy("hover text", std::make_any<Value<std::wstring>*>(&hoverText)));
            values.push_back(ValueProxy::BasicBoolValueProxy("show truncated text", std::make_any<Value<bool>*>(&showTruncatedText)));
            values.push_back(ValueProxy::BasicBoolValueProxy("text selectable", std::make_any<Value<bool>*>(&textSelectable)));
            //values.push_back(ValueProxy::BasicIntValueProxy<size_t>("selection start", std::make_any<Value<size_t>*>(&selectionStart), ValueProxy::Number(0)));
            //values.push_back(ValueProxy::BasicIntValueProxy<size_t>("selection end", std::make_any<Value<size_t>*>(&selectionEnd), ValueProxy::Number(0)));

            auto data = Component::GetReflectionData();
            data.insert(data.begin(), { "Label", std::move(values) });
            return data;
        }
    };
}