#include "TextRenderContext.h"

zcom::TextRenderContext::TextRenderContext()
{
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(p_DWriteFactory.GetAddressOf())
    );

    _fontContext = std::make_unique<FontContext>(p_DWriteFactory.Get());
}

void zcom::TextRenderContext::AddFontFromFile(const std::wstring& filePath)
{
    AddFontsFromFile({ filePath });
}

void zcom::TextRenderContext::AddFontsFromFile(const std::vector<std::wstring>& filePaths)
{
    _fontContext->AddFontFiles(filePaths);
}

IDWriteFontCollection* zcom::TextRenderContext::GetDWriteFontCollection()
{
    return _fontContext->GetFontCollection();
}

IDWriteTextLayout* zcom::TextRenderContext::GetLayoutFromTextDesc(TextDesc& desc)
{
    IDWriteTextLayout* layout = nullptr;

    zcom::TextLayout textLayout = desc.GetTextLayout();
    auto it = _textLayouts.find(textLayout);
    if (it != _textLayouts.end())
    {
        layout = it->second.Get();
    }
    else
    {
        IDWriteTextFormat* format = nullptr;

        zcom::TextFormat textFormat = desc.GetTextFormat();
        auto it = _textFormats.find(textFormat);
        if (it != _textFormats.end())
        {
            format = it->second.Get();
        }
        else
        {
            Microsoft::WRL::ComPtr<IDWriteTextFormat> formatPtr;
            HRESULT hr = p_DWriteFactory->CreateTextFormat(
                textFormat.GetFontFamily().c_str(),
                NULL,
                (DWRITE_FONT_WEIGHT)textFormat.GetFontWeight(),
                _AppFontStyleToDWriteFontStyle(textFormat.GetFontStyle()),
                (DWRITE_FONT_STRETCH)textFormat.GetFontStretch(),
                textFormat.GetFontSize(),
                L"en-us",
                formatPtr.GetAddressOf()
            );
            if (hr != S_OK)
            {
                // Retry with custom font collection
                hr = p_DWriteFactory->CreateTextFormat(
                    textFormat.GetFontFamily().c_str(),
                    _fontContext->GetFontCollection(),
                    (DWRITE_FONT_WEIGHT)textFormat.GetFontWeight(),
                    _AppFontStyleToDWriteFontStyle(textFormat.GetFontStyle()),
                    (DWRITE_FONT_STRETCH)textFormat.GetFontStretch(),
                    textFormat.GetFontSize(),
                    L"en-us",
                    formatPtr.GetAddressOf()
                );
            }
            if (hr != S_OK)
            {
                // TODO: Logging
                return nullptr;
            }

            format = formatPtr.Get();
            _textFormats[textFormat] = formatPtr;
        }

        Microsoft::WRL::ComPtr<IDWriteTextLayout> layoutPtr;
        HRESULT hr = p_DWriteFactory->CreateTextLayout(
            textLayout.GetText().c_str(),
            (UINT32)textLayout.GetText().length(),
            format,
            textLayout.GetLayoutSize().width,
            textLayout.GetLayoutSize().height,
            layoutPtr.GetAddressOf()
        );
        if (hr != S_OK)
        {
            // TODO: Logging
            return nullptr;
        }

        layoutPtr->SetWordWrapping(_AppWordWrappingToDWriteWordWrapping(textLayout.GetWrap()));
        layoutPtr->SetTextAlignment(_AppTextAlignmentToDWriteTextAlignment(textLayout.GetTextAlignment()));
        layout = layoutPtr.Get();
        _textLayouts[textLayout] = layoutPtr;
    }

    return layout;
}

zcom::TextMetrics zcom::TextRenderContext::GetLayoutTextMetrics(TextDesc& desc)
{
    auto layout = GetLayoutFromTextDesc(desc);

    DWRITE_TEXT_METRICS dwMetrics;
    layout->GetMetrics(&dwMetrics);

    TextMetrics metrics{};
    metrics.left = dwMetrics.left;
    metrics.top = dwMetrics.top;
    metrics.width = dwMetrics.width;
    metrics.widthIncludingTrailingWhitespace = dwMetrics.widthIncludingTrailingWhitespace;
    metrics.height = dwMetrics.height;
    metrics.layoutWidth = dwMetrics.layoutWidth;
    metrics.layoutHeight = dwMetrics.layoutHeight;
    metrics.maxBidiReorderingDepth = dwMetrics.maxBidiReorderingDepth;
    metrics.lineCount = dwMetrics.lineCount;
    return metrics;
}

zcom::TextLineMetricsResult zcom::TextRenderContext::GetLayoutLineMetrics(TextDesc& desc)
{
    auto layout = GetLayoutFromTextDesc(desc);

    UINT32 lineCount;
    layout->GetLineMetrics(nullptr, 0, &lineCount);
    if (lineCount == 0)
        return TextLineMetricsResult{};

    std::vector<DWRITE_LINE_METRICS> dwMetrics;
    dwMetrics.resize(lineCount);
    layout->GetLineMetrics(dwMetrics.data(), (UINT32)dwMetrics.size(), &lineCount);

    std::vector<TextLineMetrics> metrics;
    metrics.resize(lineCount);
    for (UINT32 i = 0; i < lineCount; i++)
    {
        metrics[i].length = dwMetrics[i].length;
        metrics[i].trailingWhitespaceLength = dwMetrics[i].trailingWhitespaceLength;
        metrics[i].newlineLength = dwMetrics[i].newlineLength;
        metrics[i].height = dwMetrics[i].height;
        metrics[i].baseline = dwMetrics[i].baseline;
        metrics[i].isTrimmed = dwMetrics[i].isTrimmed;
    }

    return TextLineMetricsResult{ metrics };
}

zcom::TextHitTestResult zcom::TextRenderContext::HitTestPoint(TextDesc& desc, PointF point)
{
    auto layout = GetLayoutFromTextDesc(desc);

    BOOL isTrailingHit;
    BOOL isInside;
    DWRITE_HIT_TEST_METRICS hitMetrics;
    layout->HitTestPoint(point.x, point.y, &isTrailingHit, &isInside, &hitMetrics);

    TextHitTestResult result{};
    result.isTrailingHit = (bool)isTrailingHit;
    result.isInside = (bool)isInside;
    result.hitMetrics = _DWriteHitTestMetricsToAppHitTestMetrics(hitMetrics);
    return result;
}

zcom::TextPositionHitTestResult zcom::TextRenderContext::HitTestTextPosition(TextDesc& desc, size_t textPosition, bool isTrailingHit)
{
    auto layout = GetLayoutFromTextDesc(desc);

    FLOAT posX;
    FLOAT posY;
    DWRITE_HIT_TEST_METRICS hitMetrics;
    layout->HitTestTextPosition((UINT32)textPosition, isTrailingHit, &posX, &posY, &hitMetrics);

    TextPositionHitTestResult result{};
    result.position = { posX, posY };
    result.hitMetrics = _DWriteHitTestMetricsToAppHitTestMetrics(hitMetrics);
    return result;
}

zcom::TextRangeHitTestResult zcom::TextRenderContext::HitTestTextRange(TextDesc& desc, size_t textPosition, size_t textLength)
{
    auto layout = GetLayoutFromTextDesc(desc);

    std::vector<DWRITE_HIT_TEST_METRICS> metricsArray;

    auto textMetrics = GetLayoutTextMetrics(desc);
    metricsArray.resize((size_t)textMetrics.lineCount * textMetrics.maxBidiReorderingDepth);

    while (true)
    {
        // Arbitrarily large limit
        if (metricsArray.size() > 10000000)
            return {};

        uint32_t actualCount;
        HRESULT hr = layout->HitTestTextRange((UINT32)textPosition, (UINT32)textLength, 0, 0, metricsArray.data(), (UINT32)metricsArray.size(), &actualCount);
        if (hr == E_NOT_SUFFICIENT_BUFFER)
        {
            metricsArray.resize(size_t(metricsArray.size() * 1.5) + 1);
            continue;
        }

        if (actualCount < metricsArray.size())
            metricsArray.resize(actualCount);

        break;
    }

    TextRangeHitTestResult result{};
    result.hitMetrics.resize(metricsArray.size());
    for (int i = 0; i < metricsArray.size(); i++)
        result.hitMetrics[i] = _DWriteHitTestMetricsToAppHitTestMetrics(metricsArray[i]);
    return result;
}

DWRITE_FONT_STYLE zcom::TextRenderContext::_AppFontStyleToDWriteFontStyle(zcom::FontStyle style)
{
    switch (style)
    {
        case zcom::FontStyle::NORMAL: return DWRITE_FONT_STYLE_NORMAL;
        case zcom::FontStyle::OBLIQUE: return DWRITE_FONT_STYLE_OBLIQUE;
        case zcom::FontStyle::ITALIC: return DWRITE_FONT_STYLE_ITALIC;
        default: return DWRITE_FONT_STYLE_NORMAL;
    }
}

DWRITE_WORD_WRAPPING zcom::TextRenderContext::_AppWordWrappingToDWriteWordWrapping(zcom::WordWrapping wrap)
{
    switch (wrap)
    {
        case zcom::WordWrapping::NO_WRAP: return DWRITE_WORD_WRAPPING_NO_WRAP;
        case zcom::WordWrapping::WRAP: return DWRITE_WORD_WRAPPING_WRAP;
        case zcom::WordWrapping::EMERGENCY_BREAK: return DWRITE_WORD_WRAPPING_EMERGENCY_BREAK;
        case zcom::WordWrapping::WHOLE_WORD: return DWRITE_WORD_WRAPPING_WHOLE_WORD;
        case zcom::WordWrapping::CHARACTER: return DWRITE_WORD_WRAPPING_CHARACTER;
        default: return DWRITE_WORD_WRAPPING_NO_WRAP;
    }
}

DWRITE_TEXT_ALIGNMENT zcom::TextRenderContext::_AppTextAlignmentToDWriteTextAlignment(zcom::TextAlignment alignment)
{
    switch (alignment)
    {
        case zcom::TextAlignment::LEADING: return DWRITE_TEXT_ALIGNMENT_LEADING;
        case zcom::TextAlignment::CENTER: return DWRITE_TEXT_ALIGNMENT_CENTER;
        case zcom::TextAlignment::JUSTIFIED: return DWRITE_TEXT_ALIGNMENT_JUSTIFIED;
        case zcom::TextAlignment::TRAILING: return DWRITE_TEXT_ALIGNMENT_TRAILING;
        default: return DWRITE_TEXT_ALIGNMENT_LEADING;
    }
}

zcom::TextHitTestMetrics zcom::TextRenderContext::_DWriteHitTestMetricsToAppHitTestMetrics(DWRITE_HIT_TEST_METRICS metrics)
{
    TextHitTestMetrics result{};
    result.textPosition = metrics.textPosition;
    result.length = metrics.length;
    result.left = metrics.left;
    result.top = metrics.top;
    result.width = metrics.width;
    result.height = metrics.height;
    result.bidiLevel = metrics.bidiLevel;
    result.isText = metrics.isText;
    result.isTrimmed = metrics.isTrimmed;
    return result;
}
