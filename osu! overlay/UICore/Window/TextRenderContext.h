#pragma once

#include "DirectX.h"
#include "UICore/Components/ComHelper.h"
#include "UICore/Fonts/FontLoader.h"
#include "Text.h"

namespace zcom
{
    struct TextHitTestMetrics
    {
        uint32_t textPosition;
        uint32_t length;
        float left;
        float top;
        float width;
        float height;
        uint32_t bidiLevel;
        bool isText;
        bool isTrimmed;
    };

    struct TextMetrics
    {
        float left;
        float top;
        float width;
        float widthIncludingTrailingWhitespace;
        float height;
        float layoutWidth;
        float layoutHeight;
        uint32_t maxBidiReorderingDepth;
        uint32_t lineCount;
    };

    struct TextLineMetrics
    {
        uint32_t length;
        uint32_t trailingWhitespaceLength;
        uint32_t newlineLength;
        float height;
        float baseline;
        bool isTrimmed;
    };

    struct TextLineMetricsResult
    {
        std::vector<TextLineMetrics> lineMetrics;
    };

    struct TextPositionHitTestResult
    {
        PointF position;
        TextHitTestMetrics hitMetrics;
    };

    struct TextRangeHitTestResult
    {
        std::vector<TextHitTestMetrics> hitMetrics;
    };

    struct TextHitTestResult
    {
        bool isTrailingHit;
        bool isInside;
        TextHitTestMetrics hitMetrics;
    };

    class TextRenderContext
    {
        std::unordered_map<TextFormat, Microsoft::WRL::ComPtr<IDWriteTextFormat>> _textFormats;
        std::unordered_map<TextLayout, Microsoft::WRL::ComPtr<IDWriteTextLayout>> _textLayouts;
        Microsoft::WRL::ComPtr<IDWriteFactory> p_DWriteFactory;
        std::unique_ptr<FontContext> _fontContext;

    public:
        TextRenderContext();

        void AddFontFromFile(const std::wstring& filePath);
        void AddFontsFromFile(const std::vector<std::wstring>& filePaths);
        IDWriteFontCollection* GetDWriteFontCollection();

        IDWriteTextLayout* GetLayoutFromTextDesc(TextDesc& desc);
        TextMetrics GetLayoutTextMetrics(TextDesc& desc);
        TextLineMetricsResult GetLayoutLineMetrics(TextDesc& desc);
        TextHitTestResult HitTestPoint(TextDesc& desc, PointF point);
        TextPositionHitTestResult HitTestTextPosition(TextDesc& desc, size_t textPosition, bool isTrailingHit = false);
        TextRangeHitTestResult HitTestTextRange(TextDesc& desc, size_t textPosition, size_t textLength);

    private:
        DWRITE_FONT_STYLE _AppFontStyleToDWriteFontStyle(FontStyle style);
        DWRITE_WORD_WRAPPING _AppWordWrappingToDWriteWordWrapping(WordWrapping wrap);
        DWRITE_TEXT_ALIGNMENT _AppTextAlignmentToDWriteTextAlignment(TextAlignment alignment);
        TextHitTestMetrics _DWriteHitTestMetricsToAppHitTestMetrics(DWRITE_HIT_TEST_METRICS metrics);
    };

}