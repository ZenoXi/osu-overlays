#include "App.h"
#include "SharedContext.h"
#include "Window/Window.h"
#include "ComboCounterOverlayComponent.h"
#include "ComboCounterConfig.h"

#include "Shared/Components/OverlayLayoutSetup.h"

constexpr float maxBarWidth = 24.0f;
constexpr float maxGapWidth = 3.0f;
constexpr float gapToBarRatio = 0.125f;
constexpr float maxBarCornerRadius = 4.0f;
constexpr int comboForMaxHeight = 500;
constexpr float slopeExponential = 3.0f;
constexpr float topGraphMargin = 5.0f;
constexpr float bottomGraphMargin = 45.0f;
constexpr float gapBetweenBarAndText = 2.0f;
constexpr float maxFontSize = 14.0f;
constexpr float infoPanelWidth = 100.0f;
constexpr float gridTextFontSize = 12.0f;
constexpr float gapFromGridToLabel = 4.0f;

void zcom::ComboCounterOverlayComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    Panel::Init();

    _overlay = overlay;
    _dataProviderView = std::make_unique<DataProviderView>(this);

    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&_dwriteFactory)
    );

    _scene->GetWindow()->GetTextRenderContext()->AddFontFromFile(L"Resources/Fonts/conthrax-sb.otf");
    _dwriteFontCollection = _scene->GetWindow()->GetTextRenderContext()->GetDWriteFontCollection();
    _fontName = L"Conthrax";

    _BuildFontSizeMap(maxFontSize, bottomGraphMargin - gapBetweenBarAndText);

    _dwriteFactory->CreateTextFormat(
        _fontName.c_str(),
        _dwriteFontCollection,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        gridTextFontSize,
        L"en-US",
        &_dwriteGridTextFormat
    );

    _configValueChangedEventSubscription = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configValueChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        ExecuteSynchronously([=]() {
            _UpdateParameters();
            ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(ComboCounterConfig::LAYOUT_STRING), this);
        });
    });
    _UpdateParameters();
    ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(ComboCounterConfig::LAYOUT_STRING), this); 
}

zcom::ComboCounterOverlayComponent::~ComboCounterOverlayComponent()
{
    _dwriteGridTextFormat->Release();
    _dwriteFontCollection->Release();
    _dwriteFactory->Release();
}

void zcom::ComboCounterOverlayComponent::_UpdateParameters()
{
    _barColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(ComboCounterConfig::BAR_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _textColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(ComboCounterConfig::TEXT_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _gridColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(ComboCounterConfig::GRID_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    InvokeRedraw();
}

void zcom::ComboCounterOverlayComponent::_OnUpdate()
{
    std::optional<osu::GameState> stateOpt = _scene->GetApp()->Shared<SharedContext*>()->dataProvider.GetGameState();
    if (stateOpt)
    {
        bool gameStateEntered = false;
        osu::GameState state = stateOpt.value();
        if (state.state.number == 2)
        {
            if (!_inGameState)
            {
                _maxCombos.clear();
                _maxCombos.push_back(0);
                _comboAtPreviousCheck = 0;
                _inGameState = true;
                gameStateEntered = true;
                InvokeRedraw();
                std::cout << "Game state entered\n";
            }
        }
        else
        {
            if (_inGameState)
            {
                _inGameState = false;
                std::cout << "Game state left\n";
            }
        }

        if (_inGameState)
        {
            int currentScore = (int)state.play.score;
            if (currentScore < _scoreAtPreviousCheck)
            {
                _maxCombos.clear();
                _maxCombos.push_back(0);
                _comboAtPreviousCheck = 0;
                InvokeRedraw();
                std::cout << "Combos reset does to lower score [" << currentScore << " vs " << _scoreAtPreviousCheck << "]\n";
            }
            _scoreAtPreviousCheck = currentScore;

            int currentCombo = state.play.combo.current;
            if (currentCombo < _comboAtPreviousCheck)
            {
                std::cout << "Combo added [" << currentCombo << " vs " << _comboAtPreviousCheck << "]\n";
                _maxCombos.push_back(currentCombo);
                _comboAtPreviousCheck = currentCombo;
                InvokeRedraw();
            }
            else if (currentCombo != _comboAtPreviousCheck)
            {
                // Basic sfeguard against possibly erroneous values
                if (gameStateEntered || currentCombo < _comboAtPreviousCheck + 10)
                {
                    _maxCombos.back() = currentCombo;
                    InvokeRedraw();
                }
                _comboAtPreviousCheck = currentCombo;
            }
        }
    }
}

void zcom::ComboCounterOverlayComponent::_OnDraw(Graphics* g)
{
    PointF targetTopLeft = g->AdjustPointToTargetSpace(PointF{ 0.0f, 0.0f });

    int maxCombo = 0;
    for (int combo : _maxCombos)
        if (combo > maxCombo)
            maxCombo = combo;
    if (maxCombo == 0 && !_inGameState)
        return;

    float maxBarHeight = size_->height - topGraphMargin - bottomGraphMargin;
    float barAndGapWidth = ((size_->width - infoPanelWidth) / (float)_maxCombos.size());
    float barWidth = barAndGapWidth / (1.0f + gapToBarRatio);
    float gapWidth = barAndGapWidth - barWidth;
    if (barWidth > maxBarWidth)
        barWidth = maxBarWidth;
    if (barWidth < 1.0f)
        barWidth = 1.0f;
    if (gapWidth > maxGapWidth)
        gapWidth = maxGapWidth;
    if (gapWidth < 0.0f)
        gapWidth = 0.0f;
    float barCornerRadius = barWidth / 2.0f;
    if (barCornerRadius > maxBarCornerRadius)
        barCornerRadius = maxBarCornerRadius;

    float heightOfMaxCombo = 1.0f;
    if (maxCombo < comboForMaxHeight)
        heightOfMaxCombo = -std::powf((comboForMaxHeight - maxCombo) / (float)comboForMaxHeight, slopeExponential) + 1.0f;
    heightOfMaxCombo *= maxBarHeight;

    ComPtr<ID2D1SolidColorBrush> barBrush = nullptr;
    ComPtr<ID2D1SolidColorBrush> textBrush = nullptr;
    ComPtr<ID2D1SolidColorBrush> gridBrush = nullptr;
    g->GetRenderContext()->CreateSolidColorBrush(ColorToD2D1_COLOR_F(_barColor), &barBrush);
    g->GetRenderContext()->CreateSolidColorBrush(ColorToD2D1_COLOR_F(_textColor), &textBrush);
    g->GetRenderContext()->CreateSolidColorBrush(ColorToD2D1_COLOR_F(_gridColor), &gridBrush);
    if (!barBrush || !textBrush || !gridBrush)
    {
        // TODO: Logging
        return;
    }

    float lowestOpacity = 1.0f;

    // Fade out text when space for it becomes too small and fade in grid
    float textOpacity = 1.0f;
    float gridOpacity = 0.0f;
    float transitionDelta = 0.0f;
    if ((_fontSizeMap[0].textHeight - 4.0f) > barWidth)
    {
        float overflowAmount = (_fontSizeMap[0].textHeight - 4.0f) - barWidth;
        textOpacity = 1.0f - overflowAmount / gapWidth;
        if (textOpacity > 0.0f)
            textOpacity = std::powf(textOpacity, 2.0f); // Make transition smoother
        gridOpacity = overflowAmount / gapWidth;
        if (gridOpacity > 0.0f)
            gridOpacity = std::powf(gridOpacity, 2.0f); // Make transition smoother
        if (gridOpacity > 1.0f)
            gridOpacity = 1.0f;
        transitionDelta = overflowAmount / gapWidth;
        if (transitionDelta < 0.0f)
            transitionDelta = 0.0f;
        if (transitionDelta > 1.0f)
            transitionDelta = 1.0f;
    }

    float gridLabelWidth = 0.0f;

    // Draw grid lines
    if (gridOpacity > 0.0f && _maxCombos.size() > 1)
    {
        std::wstringstream ss;
        ss << maxCombo;
        IDWriteTextLayout* highestComboLayout = nullptr;
        _dwriteFactory->CreateTextLayout(
            ss.str().c_str(),
            (UINT32)ss.str().length(),
            _dwriteGridTextFormat,
            infoPanelWidth,
            gridTextFontSize * 2.0f,
            &highestComboLayout
        );
        highestComboLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
        DWRITE_TEXT_METRICS metrics;
        highestComboLayout->GetMetrics(&metrics);
        float offsetToAlignVertically = (metrics.layoutHeight - metrics.height) * 0.5f;
        // Set label width to position COMBOS text properly
        gridLabelWidth = metrics.width + gapFromGridToLabel;

        float highestBarTopYPos = topGraphMargin + maxBarHeight - heightOfMaxCombo;
        float middleBarTopYPos = topGraphMargin + maxBarHeight - (heightOfMaxCombo / 2);
        float totalBarWidth = (barWidth + gapWidth) * _maxCombos.size() - gapWidth;

        // Draw text
        textBrush->SetOpacity(gridOpacity);
        g->GetRenderContext()->DrawTextLayout(
            D2D1::Point2F(
                targetTopLeft.x + infoPanelWidth - metrics.layoutWidth - gapFromGridToLabel,
                targetTopLeft.y + highestBarTopYPos - (metrics.layoutHeight / 2) + offsetToAlignVertically
            ),
            highestComboLayout,
            textBrush.Get()
        );
        textBrush->SetOpacity(1.0f);

        // Draw lines
        gridBrush->SetOpacity(gridOpacity);
        g->GetRenderContext()->DrawLine(
            D2D1::Point2F(targetTopLeft.x + infoPanelWidth, targetTopLeft.y + highestBarTopYPos),
            D2D1::Point2F(targetTopLeft.x + infoPanelWidth + totalBarWidth, targetTopLeft.y + highestBarTopYPos),
            gridBrush.Get()
        );
        g->GetRenderContext()->DrawLine(
            D2D1::Point2F(targetTopLeft.x + infoPanelWidth, targetTopLeft.y + middleBarTopYPos),
            D2D1::Point2F(targetTopLeft.x + infoPanelWidth + totalBarWidth, targetTopLeft.y + middleBarTopYPos),
            gridBrush.Get()
        );
        gridBrush->SetOpacity(1.0f);

        highestComboLayout->Release();
    }

    { // Draw side text
        IDWriteTextFormat* textFormat = nullptr;
        _dwriteFactory->CreateTextFormat(
            _fontName.c_str(),
            _dwriteFontCollection,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            16.0f,
            L"en-US",
            &textFormat
        );
        if (textFormat)
        {
            float layoutHeight = 25.0f;
            IDWriteTextLayout* textLayout = nullptr;
            std::wstring text = L"COMBOS";
            _dwriteFactory->CreateTextLayout(
                text.c_str(),
                (UINT32)text.length(),
                textFormat,
                (FLOAT)size_->height,
                layoutHeight,
                &textLayout
            );
            if (textLayout)
            {
                textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                float sideTextXPos = infoPanelWidth - layoutHeight - (gridLabelWidth * std::sqrtf(transitionDelta));
                g->GetRenderContext()->SetTransform(D2D1::Matrix3x2F::Rotation(-90.0f, D2D1::Point2F(targetTopLeft.x + sideTextXPos, targetTopLeft.y + (FLOAT)size_->height)));
                g->GetRenderContext()->DrawTextLayout(D2D1::Point2F(targetTopLeft.x + sideTextXPos, targetTopLeft.y + (FLOAT)size_->height), textLayout, textBrush.Get());
                g->GetRenderContext()->SetTransform(D2D1::Matrix3x2F::Identity());

                textLayout->Release();
            }
            else
            {
                // TODO: Logging
            }
            textFormat->Release();
        }
        else
        {
            // TODO: Logging
        }
    }

    // Draw each bar 
    for (int i = 0; i < _maxCombos.size(); i++)
    {
        float barLeftSideXPos = infoPanelWidth + (barWidth + gapWidth) * i;

        float ratioToMaxCombo = _maxCombos[i] / float(maxCombo);
        D2D1_RECT_F barRect = D2D1::RectF(
            targetTopLeft.x + barLeftSideXPos,
            targetTopLeft.y + topGraphMargin + maxBarHeight - heightOfMaxCombo * ratioToMaxCombo,
            targetTopLeft.x + barLeftSideXPos + barWidth,
            targetTopLeft.y + topGraphMargin + maxBarHeight
        );
        D2D1_ROUNDED_RECT barRectRounded{};
        barRectRounded.rect = barRect;
        barRectRounded.radiusX = barCornerRadius;
        barRectRounded.radiusY = barCornerRadius;
        g->GetRenderContext()->FillRoundedRectangle(barRectRounded, barBrush.Get());

        // Bar combo amount text
        std::wstringstream ss;
        ss << _maxCombos[i];
        FontSizeInfo fontInfo = _fontSizeMap[ss.str().length() - 1];

        IDWriteTextLayout* textLayout = nullptr;
        _dwriteFactory->CreateTextLayout(
            ss.str().c_str(),
            (UINT32)ss.str().length(),
            fontInfo.textFormat.Get(),
            bottomGraphMargin - gapBetweenBarAndText,
            barWidth,
            &textLayout
        );
        if (textLayout)
        {
            textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
            float offsetToAlignVertically = (barWidth - fontInfo.textHeight) * 0.5f;

            if (textOpacity > 0.0f)
            {
                g->GetRenderContext()->SetTransform(D2D1::Matrix3x2F::Rotation(-90.0f, D2D1::Point2F(targetTopLeft.x + barLeftSideXPos, targetTopLeft.y + (FLOAT)size_->height)));
                textBrush->SetOpacity(textOpacity);
                g->GetRenderContext()->DrawTextLayout(D2D1::Point2F(targetTopLeft.x + barLeftSideXPos, targetTopLeft.y + size_->height + offsetToAlignVertically), textLayout, textBrush.Get());
                textBrush->SetOpacity(1.0f);
                g->GetRenderContext()->SetTransform(D2D1::Matrix3x2F::Identity());
            }
            textLayout->Release();
        }
        else
        {
            // TODO: Logging
        }
    }
}

void zcom::ComboCounterOverlayComponent::_BuildFontSizeMap(const float maxFontSize, const float layoutWidth)
{
    // Find widest and tallest digit
    int widestDigit = 0;
    float widestDigitWidth = 0.0f;
    int tallestDigit = 0;
    float tallestDigitHeight = 0.0f;

    IDWriteTextFormat* textFormat = nullptr;
    _dwriteFactory->CreateTextFormat(
        _fontName.c_str(),
        _dwriteFontCollection,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        maxFontSize,
        L"en-US",
        &textFormat
    );
    if (!textFormat)
    {
        // TODO: Logging
        return;
    }

    for (int digit = 0; digit <= 9; digit++)
    {
        IDWriteTextLayout* textLayout = nullptr;
        std::wstringstream ss;
        ss << digit;
        _dwriteFactory->CreateTextLayout(
            ss.str().c_str(),
            (UINT32)ss.str().length(),
            textFormat,
            1000,
            1000,
            &textLayout
        );
        DWRITE_TEXT_METRICS metrics;
        textLayout->GetMetrics(&metrics);
        if (metrics.width > widestDigitWidth)
        {
            widestDigitWidth = metrics.width;
            widestDigit = digit;
        }
        if (metrics.height > tallestDigitHeight)
        {
            tallestDigitHeight = metrics.height;
            widestDigit = digit;
        }
        textLayout->Release();
    }
    textFormat->Release();

    // Build map up to 10 digits, as anything more would not fit into a 32-bit int
    for (int i = 0; i < 10; i++)
    {
        _fontSizeMap[i].textFormat.Reset();

        int digitCount = i + 1;

        std::wstringstream ss;
        for (int i = 0; i < digitCount; i++)
            ss << widestDigit;
        std::wstring text = ss.str();

        float fontSize = maxFontSize;
        while (fontSize > 0.0f)
        {
            IDWriteTextFormat* format = nullptr;
            _dwriteFactory->CreateTextFormat(
                _fontName.c_str(),
                _dwriteFontCollection,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                fontSize,
                L"en-US",
                &format
            );
            if (format)
            {
                IDWriteTextLayout* widestLayout = nullptr;
                _dwriteFactory->CreateTextLayout(
                    text.c_str(),
                    (UINT32)text.length(),
                    format,
                    1000,
                    1000,
                    &widestLayout
                );
                if (widestLayout)
                {
                    DWRITE_TEXT_METRICS widthMetrics;
                    widestLayout->GetMetrics(&widthMetrics);
                    widestLayout->Release();

                    if (widthMetrics.width <= layoutWidth)
                    {
                        _fontSizeMap[i].fontSize = fontSize;
                        _fontSizeMap[i].textWidth = widthMetrics.width;

                        IDWriteTextLayout* tallestLayout = nullptr;
                        std::wostringstream sso;
                        sso << tallestDigit;
                        _dwriteFactory->CreateTextLayout(
                            sso.str().c_str(),
                            (UINT32)sso.str().length(),
                            format,
                            1000,
                            1000,
                            &tallestLayout
                        );
                        if (tallestLayout)
                        {
                            DWRITE_TEXT_METRICS heightMetrics;
                            tallestLayout->GetMetrics(&heightMetrics);
                            tallestLayout->Release();
                            _fontSizeMap[i].textHeight = heightMetrics.height;
                        }
                        else
                        {
                            // Random value
                            _fontSizeMap[i].textHeight = 10;

                            // TODO: Logging
                        }
                        _fontSizeMap[i].textFormat = format;
                        break;
                    }
                    fontSize -= 0.5f;
                }
                else
                {
                    // TODO: Logging
                }
                format->Release();
            }
            else
            {
                // TODO: Logging
            }
        }
    }
}