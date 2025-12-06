#pragma once

#include "UICore/Components/ComHelper.h"

namespace zcom
{
    enum class FontWeight : int
    {
        LIGHT = 300,
        NORMAL = 400,
        MEDIUM = 500,
        SEMI_BOLD = 600,
        BOLD = 700
    };

    enum class FontStyle
    {
        NORMAL,
        OBLIQUE,
        ITALIC
    };

    enum class FontStretch : int
    {
        CONDENSED = 3,
        NORMAL = 5,
        EXPANDED = 7
    };

    enum class TextAlignment
    {
        LEADING,
        CENTER,
        JUSTIFIED,
        TRAILING
    };

    enum class WordWrapping
    {
        NO_WRAP,
        WRAP,
        EMERGENCY_BREAK,
        WHOLE_WORD,
        CHARACTER
    };

    class TextFormat
    {
        friend class TextDesc;

        std::wstring _fontFamily = L"Calibri";
        float _fontSize = 14.0f;
        int _fontWeight = (int)FontWeight::NORMAL;
        FontStyle _fontStyle = FontStyle::NORMAL;
        int _fontStretch = (int)FontStretch::NORMAL;

        std::size_t _hash = 0;

    public:
        TextFormat() {}

        const std::wstring& GetFontFamily() const { return _fontFamily; }
        float GetFontSize() const { return _fontSize; }
        int GetFontWeight() const { return _fontWeight; }
        FontStyle GetFontStyle() const { return _fontStyle; }
        int GetFontStretch() const { return _fontStretch; }

        std::size_t GetHash() const { return _hash; }

        bool operator==(const TextFormat& other) const
        {
            return _fontFamily == other._fontFamily
                && _fontSize == other._fontSize
                && _fontWeight == other._fontWeight
                && _fontStyle == other._fontStyle
                && _fontStretch == other._fontStretch;
        }
    };

    class TextLayout
    {
        friend class TextDesc;

        std::wstring _text;
        zcom::SizeF _layoutSize = zcom::SizeF{};
        TextAlignment _textAlignment = TextAlignment::LEADING;
        WordWrapping _wrap = WordWrapping::NO_WRAP;

        TextFormat _format;

        std::size_t _hash = 0;

    public:
        TextLayout() {}

        const std::wstring& GetText() const { return _text; }
        zcom::SizeF GetLayoutSize() const { return _layoutSize; }
        TextAlignment GetTextAlignment() const { return _textAlignment; }
        WordWrapping GetWrap() const { return _wrap; }

        TextFormat GetFormat() const { return _format; }

        std::size_t GetHash() const { return _hash; }

        bool operator==(const TextLayout& other) const
        {
            return _text == other._text
                && _layoutSize == other._layoutSize
                && _textAlignment == other._textAlignment
                && _wrap == other._wrap
                && _format == other._format;
        }
    };

    class TextDesc
    {
        std::wstring _fontFamily = L"Calibri";
        float _fontSize = 14.0f;
        int _fontWeight = (int)FontWeight::NORMAL;
        FontStyle _fontStyle = FontStyle::NORMAL;
        int _fontStretch = (int)FontStretch::NORMAL;

        std::wstring _text;
        zcom::SizeF _layoutSize;
        TextAlignment _textAlignment = TextAlignment::LEADING;
        WordWrapping _wrap = WordWrapping::NO_WRAP;

        bool _formatHashEvaluated = false;
        std::size_t _formatHash = 0;
        bool _layoutHashEvaluated = false;
        std::size_t _layoutHash = 0;

    public:
        TextDesc() : _text(L""), _layoutSize(zcom::SizeF{}) {}
        TextDesc(const std::wstring& text, zcom::SizeF layoutSize) : _text(text), _layoutSize(layoutSize) {}

        TextDesc& WithFontFamily(const std::wstring& fontFamily)    { _fontFamily = fontFamily;         _formatHashEvaluated = false; _layoutHashEvaluated = false; return *this; }
        TextDesc& WithFontSize(float fontSize)                      { _fontSize = fontSize;             _formatHashEvaluated = false; _layoutHashEvaluated = false; return *this; }
        TextDesc& WithFontWeight(int fontWeight)                    { _fontWeight = fontWeight;         _formatHashEvaluated = false; _layoutHashEvaluated = false; return *this; }
        TextDesc& WithFontWeight(FontWeight fontWeight)             { _fontWeight = (int)fontWeight;    _formatHashEvaluated = false; _layoutHashEvaluated = false; return *this; }
        TextDesc& WithFontStyle(FontStyle fontStyle)                { _fontStyle = fontStyle;           _formatHashEvaluated = false; _layoutHashEvaluated = false; return *this; }
        TextDesc& WithFontStretch(int fontStretch)                  { _fontStretch = fontStretch;       _formatHashEvaluated = false; _layoutHashEvaluated = false; return *this; }
        TextDesc& WithFontStretch(FontStretch fontStretch)          { _fontStretch = (int)fontStretch;  _formatHashEvaluated = false; _layoutHashEvaluated = false; return *this; }

        TextDesc& WithText(const std::wstring& text)                { _text = text;                     _layoutHashEvaluated = false; return *this; }
        TextDesc& WithLayoutSize(zcom::SizeF layoutSize)            { _layoutSize = layoutSize;         _layoutHashEvaluated = false; return *this; }
        TextDesc& WithTextAlignment(TextAlignment textAlignment)    { _textAlignment = textAlignment;   _layoutHashEvaluated = false; return *this; }
        TextDesc& WithWrap(WordWrapping wrap)                       { _wrap = wrap;                     _layoutHashEvaluated = false; return *this; }

        TextFormat GetTextFormat()
        {
            if (!_formatHashEvaluated)
                _EvaluateFormatHash();

            TextFormat format;
            format._fontFamily = _fontFamily;
            format._fontSize = _fontSize;
            format._fontWeight = _fontWeight;
            format._fontStyle = _fontStyle;
            format._fontStretch = _fontStretch;
            format._hash = _formatHash;
            return format;
        }
        TextLayout GetTextLayout()
        {
            if (!_layoutHashEvaluated)
                _EvaluateLayoutHash();

            TextLayout layout;
            layout._text = _text;
            layout._layoutSize = _layoutSize;
            layout._textAlignment = _textAlignment;
            layout._wrap = _wrap;
            layout._format = GetTextFormat();
            layout._hash = _layoutHash;
            return layout;
        }

    private:
        void _EvaluateFormatHash()
        {
            _formatHash = 17;
            _formatHash = _formatHash * 31 + std::hash<std::wstring>()(_fontFamily);
            _formatHash = _formatHash * 31 + std::hash<float>()(_fontSize);
            _formatHash = _formatHash * 31 + std::hash<int>()(_fontWeight);
            _formatHash = _formatHash * 31 + std::hash<int>()((int)_fontStyle);
            _formatHash = _formatHash * 31 + std::hash<int>()(_fontStretch);
            _formatHashEvaluated = true;
        }

        void _EvaluateLayoutHash()
        {
            if (!_formatHashEvaluated)
                _EvaluateFormatHash();

            _layoutHash = _formatHash;
            _layoutHash = _layoutHash * 31 + std::hash<std::wstring>()(_text);
            _layoutHash = _layoutHash * 31 + std::hash<float>()(_layoutSize.width);
            _layoutHash = _layoutHash * 31 + std::hash<float>()(_layoutSize.height);
            _layoutHash = _layoutHash * 31 + std::hash<int>()((int)_textAlignment);
            _layoutHash = _layoutHash * 31 + std::hash<int>()((int)_wrap);
            _layoutHashEvaluated = true;
        }
    };
}

template <>
struct std::hash<zcom::TextFormat>
{
    std::size_t operator()(const zcom::TextFormat& obj) const
    {
        return obj.GetHash();
    }
};

template <>
struct std::hash<zcom::TextLayout>
{
    std::size_t operator()(const zcom::TextLayout& obj) const
    {
        return obj.GetHash();
    }
};