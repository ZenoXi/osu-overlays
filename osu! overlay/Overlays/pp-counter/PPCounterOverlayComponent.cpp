#include "App.h"
#include "SharedContext.h"
#include "PPCounterOverlayComponent.h"
#include "PPCounterConfig.h"

#include "UICore/Components/Base/Label.h"
#include "UICore/Components/Base/Dummy.h"
#include "UICore/Helper/AnimationHelper.h"

#include "Shared/Components/OverlayLayoutSetup.h"

void zcom::PPCounterOverlayComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    Panel::Init();

    _overlay = overlay;
    _dataProviderView = std::make_unique<DataProviderView>(this);

    _configValueChangedEventSubscription = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configValueChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        ExecuteSynchronously([=]() {
            _UpdateAppearanceFromConfig();
            ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(PPCounterConfig::LAYOUT_STRING), this);
        });
    });
    _UpdateAppearanceFromConfig();
    ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(PPCounterConfig::LAYOUT_STRING), this);

    auto dataProviderDisabledLabel = Create<Label>(L"osu! data provider is not enabled, game data unavailable");
    dataProviderDisabledLabel->parentSize = { 1.0f, 1.0f };
    dataProviderDisabledLabel->size = { -3, -3 };
    dataProviderDisabledLabel->padding = { 20.0f, 0.0f, 20.0f, 0.0f };
    dataProviderDisabledLabel->xTextAlign = TextAlignment::CENTER;
    dataProviderDisabledLabel->yTextAlign = Alignment::CENTER;
    dataProviderDisabledLabel->wordWrapping = WordWrapping::WRAP;
    dataProviderDisabledLabel->border.cornerRadius = 12.0f;
    dataProviderDisabledLabel->backgroundColor.ComputedFrom([](Color color) { return color; }, _backgroundColor);
    dataProviderDisabledLabel->visible.ComputedFrom([](bool dataProviderRunning) { return !dataProviderRunning; }, _dataProviderView->running_);

    _dataPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    _dataPanel->parentSize = { 1.0f, 1.0f };
    _dataPanel->size = { -3, -3 };
    _dataPanel->border.cornerRadius = 12.0f;
    _dataPanel->backgroundColor.ComputedFrom([](Color color) { return color; }, _backgroundColor);
    _dataPanel->visible.ComputedFrom([](bool dataProviderRunning) { return dataProviderRunning; }, _dataProviderView->running_);
    _dataPanel->SubscribePreContentDraw([=](Component* item, Graphics* g) { _DrawStrainGraph(item, g); }).Detach();

    auto background = Create<Dummy>();
    background->parentSize = { 1.0f, 1.0f };
    background->size = { -3, -3 };
    background->xAlign = Alignment::END;
    background->yAlign = Alignment::END;
    background->border.cornerRadius = 12.0f;
    background->backgroundColor.ComputedFrom([](Color color) { return color; }, _backgroundAccentColor);
    background->zIndex = -2;

    _ppLabel = Create<Label>(L"0pp");
    _ppLabel->size = { 150, 0 };
    _ppLabel->parentSize = { 0.0f, 1.0f };
    _ppLabel->padding = { 20.0f, 0.0f, 0.0f, 0.0f };
    _ppLabel->xTextAlign = TextAlignment::CENTER;
    _ppLabel->yTextAlign = Alignment::CENTER;
    _ppLabel->font = L"Arial Rounded MT";
    _ppLabel->fontSize = 32.0f;
    _ppLabel->fontWeight = FontWeight::BOLD;
    _100CountLabel = Create<Label>(L"0");
    _100CountLabel->parentSize = { 0.0f, 1.0f };
    _100CountLabel->xTextAlign = TextAlignment::CENTER;
    _100CountLabel->yTextAlign = Alignment::CENTER;
    _100CountLabel->font = L"Arial Rounded MT";
    _100CountLabel->fontSize = 26.0f;
    _100CountLabel->fontWeight = FontWeight::BOLD;
    _100CountLabel->fontColor = Color(0xA5EDA7);
    _100CountLabel->SetProperty(FlexGrow());
    _50CountLabel = Create<Label>(L"0");
    _50CountLabel->parentSize = { 0.0f, 1.0f };
    _50CountLabel->xTextAlign = TextAlignment::CENTER;
    _50CountLabel->yTextAlign = Alignment::CENTER;
    _50CountLabel->font = L"Arial Rounded MT";
    _50CountLabel->fontSize = 26.0f;
    _50CountLabel->fontWeight = FontWeight::BOLD;
    _50CountLabel->fontColor = Color(0xDFC886);
    _50CountLabel->SetProperty(FlexGrow());
    _missCountLabel = Create<Label>(L"0");
    _missCountLabel->parentSize = { 0.0f, 1.0f };
    _missCountLabel->xTextAlign = TextAlignment::CENTER;
    _missCountLabel->yTextAlign = Alignment::CENTER;
    _missCountLabel->font = L"Arial Rounded MT";
    _missCountLabel->fontSize = 26.0f;
    _missCountLabel->fontWeight = FontWeight::BOLD;
    _missCountLabel->fontColor = Color(0xED6D69);
    _missCountLabel->SetProperty(FlexGrow());

    _dataPanel->AddItem(_ppLabel.get());
    _dataPanel->AddItem(_100CountLabel.get());
    _dataPanel->AddItem(_50CountLabel.get());
    _dataPanel->AddItem(_missCountLabel.get());

    AddItem(_dataPanel.get());
    AddItem(std::move(background));
    AddItem(std::move(dataProviderDisabledLabel));
}

void zcom::PPCounterOverlayComponent::_OnUpdate()
{
    Panel::_OnUpdate();

    std::optional<osu::GameState> stateOpt = _scene->GetApp()->Shared<SharedContext*>()->dataProvider.GetGameState();
    if (stateOpt)
    {
        osu::GameState state = stateOpt.value();
        _100CountLabel->text = std::to_wstring(state.play.hits.count100);
        _50CountLabel->text = std::to_wstring(state.play.hits.count50);
        _missCountLabel->text = std::to_wstring(state.play.hits.countMiss);
        _mapRate = state.play.mods.rate;

        if (ztime::Main() > (_lastGraphUpdate + _graphUpdateInterval))
        {
            _lastGraphUpdate = ztime::Main();
            _strains.clear();
            _xAxis.clear();
            _highestStrain = 0.0f;
            for (int i = 0; i < state.performance.graph.xaxis.size(); i++)
            {
                _strains.push_back(state.performance.graph.aim[i] + state.performance.graph.speed[i]);
                _xAxis.push_back(state.performance.graph.xaxis[i]);

                if (_strains.back() > _highestStrain)
                    _highestStrain = _strains.back();
            }
            _currentGameTime = state.beatmap.time.live.GetTime(MILLISECONDS);
            _lastObjectTime = state.beatmap.time.lastObject.GetTime(MILLISECONDS);
            _dataPanel->InvokeRedraw();
        }

        int pp = int(std::roundf(state.play.pp.current));
        if (pp != _targetPPValue)
        {
            _animatingPP = true;
            _ppChangeStart = ztime::Main();
            _startPPValue = _currentPPValue;
            _targetPPValue = pp;
        }

        if (_animatingPP)
        {
            float progress = (ztime::Main() - _ppChangeStart).GetDuration() / (float)_ppChangeDuration.GetDuration();
            if (progress >= 1.0f)
            {
                _animatingPP = false;
                _currentPPValue = _targetPPValue;
            }
            else
            {
                _currentPPValue = _startPPValue + (int)std::ceilf((_targetPPValue - _startPPValue) * zanim::EaseOutQuad(progress));
            }
        }

        _ppLabel->text = std::to_wstring(_currentPPValue) + L"pp";
    }
}

void zcom::PPCounterOverlayComponent::_UpdateAppearanceFromConfig()
{
    _backgroundColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(PPCounterConfig::BACKGROUND_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _backgroundAccentColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(PPCounterConfig::BACKGROUND_ACCENT_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _showDifficultyGraph = _scene->GetApp()->config.GetIntConfigValue(PPCounterConfig::SHOW_DIFFICULTY_GRAPH, Config::ADD_AND_SAVE_IF_MISSING);
    _difficultyGraphForegroundColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(PPCounterConfig::DIFFICULTY_GRAPH_FOREGROUND_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _difficultyGraphBackgroundColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(PPCounterConfig::DIFFICULTY_GRAPH_BACKGROUND_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    if (_dataPanel)
        _dataPanel->InvokeRedraw();
}

void zcom::PPCounterOverlayComponent::_DrawStrainGraph(Component* item, Graphics* g)
{
    if (_highestStrain <= 0.0f || !_showDifficultyGraph)
        return;

    Rect rect = g->GetTargetSourceClipRect();

    ID2D1PathGeometry* pPathGeometry;

    // Create path geometry
    g->GetD2DFactory()->CreatePathGeometry(&pPathGeometry);
    if (!pPathGeometry)
    {
        // TODO: Logging
        return;
    }

    ID2D1GeometrySink* pSink = NULL;
    pPathGeometry->Open(&pSink);

    pSink->SetFillMode(D2D1_FILL_MODE_WINDING);


    int64_t curMs = _currentGameTime < 0 ? 0 : _currentGameTime;
    int64_t endMs = _lastObjectTime;
    int w = rect.GetWidth();
    int h = rect.GetHeight();

    pSink->BeginFigure(D2D1::Point2F((float)rect.left, (float)rect.bottom), D2D1_FIGURE_BEGIN_FILLED);
    for (int i = 0; i < _strains.size(); i++)
    {
        float strain = _strains[i];
        if (strain < 0)
            strain = 0;
        float x = ((float)_xAxis[i] / endMs) * _mapRate * w;
        float y = strain / _highestStrain * (h * 0.5f);
        pSink->AddLine(D2D1::Point2F(rect.left + x, rect.bottom - y));
    }
    pSink->AddLine(D2D1::Point2F((float)rect.right, (float)rect.bottom));
    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

    pSink->Close();
    pSink->Release();

    //ID2D1LinearGradientBrush* brush;
    //g->GetRenderContext()->CreateSolidColorBrush(D2D1::ColorF(0x3F2E30), &brush);

    //ID2D1SolidColorBrush* brush;
    ////g->GetRenderContext()->CreateSolidColorBrush(D2D1::ColorF(0x2D191B), &brush);
    //g->GetRenderContext()->CreateSolidColorBrush(D2D1::ColorF(0x271315), &brush);
    //g->GetRenderContext()->FillGeometry(pPathGeometry, brush);
    //brush->Release();

    {
        Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> stopCollection = nullptr;
        Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> brush = nullptr;

        float x = (float)curMs / endMs;
        D2D1_GRADIENT_STOP gradientStops[2];
        //gradientStops[0] = { x, D2D1::ColorF(0x3F2E30) };
        //gradientStops[1] = { x, D2D1::ColorF(0x271315) };
        gradientStops[0] = { x, ColorToD2D1_COLOR_F(_difficultyGraphForegroundColor) };
        gradientStops[1] = { x, ColorToD2D1_COLOR_F(_difficultyGraphBackgroundColor) };

        g->GetRenderContext()->CreateGradientStopCollection(
            gradientStops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            stopCollection.GetAddressOf()
        );
        if (stopCollection)
        {
            g->GetRenderContext()->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F((float)rect.left, 0.0f),
                    D2D1::Point2F((float)rect.right, 0.0f)
                ),
                stopCollection.Get(),
                brush.GetAddressOf()
            );

            if (brush)
            {
                g->GetRenderContext()->FillGeometry(pPathGeometry, brush.Get());
            }
            else
            {
                // TODO: Logging
            }
        }
        else
        {
            // TODO: Logging
        }
    }


    pPathGeometry->Release();
}
