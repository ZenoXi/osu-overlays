#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "ComboBarScene.h"
#include "ComboBarConfig.h"
#include "FontLoader/FontLoader.h"
#include "Shared/Util/Color.h"

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

void zcom::ComboBarScene::Init(SceneOptionsBase* options)
{
    ComboBarSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const ComboBarSceneOptions*>(options);
    
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&_dwriteFactory)
    );

    // Code to get the font name to supply to CreateTextFormat
    //
    //IDWriteFontFamily* family;
    //fontCollection->GetFontFamily(0, &family);
    //IDWriteLocalizedStrings* strings;
    //family->GetFamilyNames(&strings);
    //std::wstring name;
    //name.resize(128);
    //strings->GetString(0, name.data(), name.size());

    //FontCollectionLoader::GetLoader();

    FontContext fontContext(_dwriteFactory);
    std::vector<std::wstring> filePaths = { L"Resources/Fonts/conthrax-sb.otf" };
    fontContext.CreateFontCollection(filePaths, &_dwriteFontCollection);
    _fontName = L"Conthrax";

    //SimpleTimer timer;
    _BuildFontSizeMap(maxFontSize, bottomGraphMargin - gapBetweenBarAndText);
    //std::cout << timer.MillisElapsed() << '\n';

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

    _basePanel->SetBorderWidth(7.0f);
    _basePanel->SetBorderColor(D2D1::ColorF(0xFF00FF, 0.5f));
    _basePanel->SetBackgroundColor(D2D1::ColorF(0, 1.0f / 255.0f));
    _basePanel->SubscribePostUpdate([&]() {
        _Update();
    }).Detach();
    _basePanel->SubscribePostDraw([&](Component* panel, Graphics g) {
        _Draw(panel, g);
    }).Detach();

    //_dataProvider = std::make_unique<osu::data::OsuDataProvider>();

    _UpdateParameters();
    _configValueChangedEventSubscription = _app->config.SubscribeOnConfigValueChanged();
    _configValueChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        if (changes)
        {
            bool colorChanged = changes.value().first == ComboBarConfig::BAR_COLOR.name
                || changes.value().first == ComboBarConfig::TEXT_COLOR.name
                || changes.value().first == ComboBarConfig::GRID_COLOR.name;
            if (colorChanged)
                _basePanel->ExecuteSynchronously([=]() { _UpdateParameters(); });
        }
    });

    _basePanel->SetBorderVisibility(true);
    _basePanel->ExecuteSynchronously([=]() {
        if (!_interactionModeActive)
            _basePanel->SetBorderVisibility(false);
    }, Duration(2, SECONDS));
}

void zcom::ComboBarScene::Uninit()
{
    _dwriteGridTextFormat->Release();
    _dwriteFontCollection->Release();
    _dwriteFactory->Release();
}

void zcom::ComboBarScene::_UpdateParameters()
{
    zutil::Color barColor = zutil::Color(_app->config.GetIntConfigValue(ComboBarConfig::BAR_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    zutil::Color textColor = zutil::Color(_app->config.GetIntConfigValue(ComboBarConfig::TEXT_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    zutil::Color gridColor = zutil::Color(_app->config.GetIntConfigValue(ComboBarConfig::GRID_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _barColor = D2D1::ColorF(barColor.ToIntNoAlpha(), barColor.a / 255.0f);
    _textColor = D2D1::ColorF(textColor.ToIntNoAlpha(), textColor.a / 255.0f);
    _gridColor = D2D1::ColorF(gridColor.ToIntNoAlpha(), gridColor.a / 255.0f);
    _basePanel->InvokeRedraw();
}

void zcom::ComboBarScene::_Update()
{
    if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) &&
        (GetAsyncKeyState(VK_LSHIFT) & 0x8000) &&
        (GetAsyncKeyState('Q') & 0x8000) &&
        (GetAsyncKeyState('W') & 0x8000))
    {
        if (!_interactionModeActive)
        {
            _interactionModeActive = true;
            _basePanel->SetBorderVisibility(true);
            _window->Backend().SetMouseInteraction(zwnd::MouseWindowInteraction::DEFAULT);
        }
    }
    else if (_interactionModeActive)
    {
        _interactionModeActive = false;
        _basePanel->SetBorderVisibility(false);
        _window->Backend().SetMouseInteraction(zwnd::MouseWindowInteraction::PASS_THROUGH);
    }

    std::optional<osu::GameState> stateOpt = _app->dataProvider.GetGameState();
    if (stateOpt)
    {
        bool gameStateEntered = false;
        osu::GameState state = stateOpt.value();
        if (state.menuState.state == 2)
        {
            if (!_inGameState)
            {
                _maxCombos.clear();
                _maxCombos.push_back(0);
                _comboAtPreviousCheck = 0;
                _inGameState = true;
                gameStateEntered = true;
                _basePanel->InvokeRedraw();
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
            int currentScore = state.gameplayState.score;
            if (currentScore < _scoreAtPreviousCheck)
            {
                _maxCombos.clear();
                _maxCombos.push_back(0);
                _comboAtPreviousCheck = 0;
                _basePanel->InvokeRedraw();
                std::cout << "Combos reset does to lower score [" << currentScore << " vs " << _scoreAtPreviousCheck << "]\n";
            }
            _scoreAtPreviousCheck = currentScore;

            int currentCombo = state.gameplayState.currentCombo;
            if (currentCombo < _comboAtPreviousCheck)
            {
                std::cout << "Combo added [" << currentCombo << " vs " << _comboAtPreviousCheck << "]\n";
                _maxCombos.push_back(currentCombo);
                _comboAtPreviousCheck = currentCombo;
                _basePanel->InvokeRedraw();
            }
            else if (currentCombo != _comboAtPreviousCheck)
            {
                // Basic sfeguard against possibly erroneous values
                if (gameStateEntered || currentCombo < _comboAtPreviousCheck + 10)
                {
                    _maxCombos.back() = currentCombo;
                    _basePanel->InvokeRedraw();
                }
                _comboAtPreviousCheck = currentCombo;
            }
        }
    }
}

void zcom::ComboBarScene::_Draw(Component* panel, Graphics g)
{
    //_maxCombos = { 123, 23, 15, 3, 81, 45, 192, 1, 12, 37, 69, 164, 83, 27 };

    int maxCombo = 0;
    for (int combo : _maxCombos)
        if (combo > maxCombo)
            maxCombo = combo;
    if (maxCombo == 0 && !_inGameState)
        return;

    float maxBarHeight = panel->GetHeight() - topGraphMargin - bottomGraphMargin;
    float barAndGapWidth = ((panel->GetWidth() - infoPanelWidth) / (float)_maxCombos.size());
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
    g.target->CreateSolidColorBrush(_barColor, &barBrush);
    g.target->CreateSolidColorBrush(_textColor, &textBrush);
    g.target->CreateSolidColorBrush(_gridColor, &gridBrush);
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
        g.target->DrawTextLayout(
            D2D1::Point2F(
                infoPanelWidth - metrics.layoutWidth - gapFromGridToLabel,
                highestBarTopYPos - (metrics.layoutHeight / 2) + offsetToAlignVertically
            ),
            highestComboLayout,
            textBrush.Get()
        );
        textBrush->SetOpacity(1.0f);

        // Draw lines
        gridBrush->SetOpacity(gridOpacity);
        g.target->DrawLine(
            D2D1::Point2F(infoPanelWidth, highestBarTopYPos),
            D2D1::Point2F(infoPanelWidth + totalBarWidth, highestBarTopYPos),
            gridBrush.Get()
        );
        g.target->DrawLine(
            D2D1::Point2F(infoPanelWidth, middleBarTopYPos),
            D2D1::Point2F(infoPanelWidth + totalBarWidth, middleBarTopYPos),
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
                (FLOAT)panel->GetHeight(),
                layoutHeight,
                &textLayout
            );
            if (textLayout)
            {
                textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                float sideTextXPos = infoPanelWidth - layoutHeight - (gridLabelWidth * std::sqrtf(transitionDelta));
                g.target->SetTransform(D2D1::Matrix3x2F::Rotation(-90.0f, D2D1::Point2F(sideTextXPos, (FLOAT)panel->GetHeight())));
                g.target->DrawTextLayout(D2D1::Point2F(sideTextXPos, (FLOAT)panel->GetHeight()), textLayout, textBrush.Get());
                g.target->SetTransform(D2D1::Matrix3x2F::Identity());

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
            barLeftSideXPos,
            topGraphMargin + maxBarHeight - heightOfMaxCombo * ratioToMaxCombo,
            barLeftSideXPos + barWidth,
            topGraphMargin + maxBarHeight
        );
        D2D1_ROUNDED_RECT barRectRounded{};
        barRectRounded.rect = barRect;
        barRectRounded.radiusX = barCornerRadius;
        barRectRounded.radiusY = barCornerRadius;
        g.target->FillRoundedRectangle(barRectRounded, barBrush.Get());

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
                g.target->SetTransform(D2D1::Matrix3x2F::Rotation(-90.0f, D2D1::Point2F(barLeftSideXPos, (FLOAT)panel->GetHeight())));
                textBrush->SetOpacity(textOpacity);
                g.target->DrawTextLayout(D2D1::Point2F(barLeftSideXPos, panel->GetHeight() + offsetToAlignVertically), textLayout, textBrush.Get());
                textBrush->SetOpacity(1.0f);
                g.target->SetTransform(D2D1::Matrix3x2F::Identity());
            }
            textLayout->Release();
        }
        else
        {
            // TODO: Logging
        }
    }
}

void zcom::ComboBarScene::_BuildFontSizeMap(const float maxFontSize, const float layoutWidth)
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