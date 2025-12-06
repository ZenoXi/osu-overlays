#include "App.h"
#include "SharedContext.h"
#include "URCounterOverlayComponent.h"
#include "URCounterConfig.h"

#include "Shared/Components/OverlayLayoutSetup.h"

void zcom::URCounterOverlayComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    Label::Init();

    _overlay = overlay;
    _dataProviderView = std::make_unique<DataProviderView>(this);

    _configValueChangedEventSubscription = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configValueChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        ExecuteSynchronously([=]() {
            ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(URCounterConfig::LAYOUT_STRING), this);
        });
    });
    ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(URCounterConfig::LAYOUT_STRING), this);

    xTextAlign = TextAlignment::CENTER;
    yTextAlign = Alignment::CENTER;
    fontSize = 24.0f;
    font = L"Arial Rounded MT";
    fontWeight = FontWeight::BOLD;
}

void zcom::URCounterOverlayComponent::_OnUpdate()
{
    Label::_OnUpdate();

    std::optional<osu::GameState> stateOpt = _scene->GetApp()->Shared<SharedContext*>()->dataProvider.GetGameState();
    if (stateOpt && _dataProviderView->running_)
    {
        osu::GameState state = stateOpt.value();
        text = std::to_wstring((int)state.play.unstableRate) + L"ur";
    }
    else
    {
        text = L"No data";
    }
}
