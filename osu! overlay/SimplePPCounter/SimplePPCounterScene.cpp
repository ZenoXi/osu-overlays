#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "SimplePPCounterScene.h"

void zcom::SimplePPCounterScene::Init(SceneOptionsBase* options)
{
    SimplePPCounterSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const SimplePPCounterSceneOptions*>(options);

    _ppLabel = Create<Label>(L"0");
    _ppLabel->AutomaticSize();
    _ppLabel->SetAlignment(Alignment::CENTER, Alignment::CENTER);
    _ppLabel->SetFont(L"Comic Sans MS");
    _ppLabel->SetFontSize(32.0f);
    _ppLabel->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD);
    _ppLabel->SetVerticalTextAlignment(Alignment::CENTER);
    _ppLabel->SetHorizontalTextAlignment(TextAlignment::CENTER);

    _basePanel->AddItem(_ppLabel.get());
    _basePanel->SetBorderWidth(7.0f);
    _basePanel->SetBackgroundColor(D2D1::ColorF(0, 1.0f / 255.0f));
    _basePanel->SubscribePostUpdate([=]() {
        _Update();
    }).Detach();

    //_dataProvider = std::make_unique<osu::data::OsuDataProvider>();
}

void zcom::SimplePPCounterScene::_Update()
{
    if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) &&
        (GetAsyncKeyState(VK_LSHIFT) & 0x8000) &&
        (GetAsyncKeyState('Q') & 0x8000) &&
        (GetAsyncKeyState('W') & 0x8000))
    {
        _basePanel->SetBorderVisibility(true);
        _window->Backend().SetMouseInteraction(zwnd::MouseWindowInteraction::DEFAULT);
    }
    else
    {
        _basePanel->SetBorderVisibility(false);
        _window->Backend().SetMouseInteraction(zwnd::MouseWindowInteraction::PASS_THROUGH);
    }

    std::optional<osu::GameState> stateOpt = _app->dataProvider.GetGameState();
    if (stateOpt)
    {
        osu::GameState state = stateOpt.value();
        //int currentPP = _dataProvider->GetCurrentState().gameplayState.currentPP;
        //int currentPP = _dataProvider->GetCurrentState().menuState.isChatEnabled;
        //std::optional<osu::data::OsuState> state = _app->dataProvider.GetGameState();
        //if (state)
        int currentPP = state.gameplayState.currentPP;
        if (currentPP != _currentPPValue)
        {
            _currentPPValue = currentPP;
            std::wostringstream ss(L"");
            ss << _currentPPValue;
            _ppLabel->SetText(ss.str());
        }
    }
}