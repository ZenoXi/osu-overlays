#include "App.h"
#include "SharedContext.h"
#include "SmokeTrailSetupComponent.h"
#include "SmokeTrailConfig.h"

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Button.h"
#include "UICore/Components/Base/ColorSelector.h"
#include "UICore/Components/Base/NumberInput.h"
#include "UICore/Components/Base/KeySelector.h"

#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Components/NumberParameterInput.h"
#include "Shared/Styles/Styles.h"

void zcom::SmokeTrailSetupComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    ScrollPanel::Init();

    size = { 300, 0 };
    parentSize = { 0.0f, 1.0f };
    backgroundColor = Color(0x202020);
    yScrollbar.scrollable = true;
    yScrollbar.backgroundVisible = true;

    _overlay = overlay;
    _overlayView.emplace(overlay->Id(), this);

    auto flexPanel = Create<FlexPanel>(FlexDirection::DOWN);
    flexPanel->parentSize = { 1.0f, 0.0f };
    flexPanel->autoHeight = true;

    auto generalPanel = Create<FlexPanel>(FlexDirection::DOWN);
    generalPanel->parentSize = { 1.0f, 0.0f };
    generalPanel->autoHeight = true;

    auto titleRow = Create<FlexPanel>(FlexDirection::RIGHT);
    titleRow->parentSize = { 1.0f, 0.0f };
    titleRow->autoHeight = true;
    titleRow->padding = { 15, 15, 15, 15 };
    auto generalLabel = Create<Label>(L"Smoke trail");
    generalLabel->size = { 0, 30 };
    generalLabel->yTextAlign = Alignment::CENTER;
    generalLabel->fontSize = 20.0f;
    generalLabel->SetProperty(FlexGrow());
    auto enableButton = Create<Button>(L"Enable");
    enableButton->size = { 90, 30 };
    enableButton->SetComputedStyle("style", [](Component* item, bool overlayOpen) {
        Button* button = (Button*)item;
        if (!overlayOpen)
        {
            button->Label()->text = L"Enable";
            EnableButtonStyle::Apply(button);
        }
        else
        {
            button->Label()->text = L"Disable";
            DisableButtonStyle::Apply(button);
        }
    }, _overlayView->enabled_);
    enableButton->activation = ButtonActivation::RELEASE;
    enableButton->SubscribeOnActivated([=]() {
        if (!_overlayView->enabled_)
            _scene->GetApp()->Shared<SharedContext*>()->overlayManager.EnableOverlay(_overlay);
        else
            _scene->GetApp()->Shared<SharedContext*>()->overlayManager.DisableOverlay(_overlay->Id());
    }).Detach();
    titleRow->AddItem(std::move(generalLabel));
    titleRow->AddItem(std::move(enableButton));

    generalPanel->AddItem(std::move(titleRow));

    auto overlayLayoutSetupPanel = Create<OverlayLayoutSetup>(_scene->GetApp()->config.GetConfigValue(SmokeTrailConfig::LAYOUT_STRING));
    overlayLayoutSetupPanel->parentSize = { 1.0f, 0.0f };
    overlayLayoutSetupPanel->padding = { 0, 0, 0, 10 };
    overlayLayoutSetupPanel->autoHeight = true;
    overlayLayoutSetupPanel->SubscribeOnOverlayLayoutStringChanged([=](std::wstring layoutString) {
        _scene->GetApp()->config.SetValue(SmokeTrailConfig::LAYOUT_STRING.name, layoutString);
    }).Detach();


    auto performancePanel = Create<FlexPanel>(FlexDirection::DOWN);
    performancePanel->parentSize = { 1.0f, 0.0f };
    performancePanel->autoHeight = true;

    auto cellSizeInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Cell size", L"Size of the simulation grid cell, in pixels. Smaller values have a very large impact on performance",
        _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::CELL_SIZE, Config::ADD_IF_MISSING),
        1, 100, 1
    ));
    cellSizeInput->GetInput()->size.Assign(Width(60));
    cellSizeInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(SmokeTrailConfig::CELL_SIZE.name, value.getAsInteger());
    }).Detach();

    auto threadCountInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"CPU Thread count", L"How many CPU threads to use for the simulation when hardware acceleration is unavailable. If you have a CPU with many cores, increasing this setting should improve performance. Higher values offer diminishing permormance increases or even reduce performance",
        _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::THREAD_COUNT, Config::ADD_IF_MISSING),
        1, 64, 1
    ));
    threadCountInput->GetInput()->size.Assign(Width(60));
    threadCountInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(SmokeTrailConfig::THREAD_COUNT.name, value.getAsInteger());
    }).Detach();

    performancePanel->AddItem(Create<SectionHeader>(L"Performance"));
    performancePanel->AddItem(std::move(cellSizeInput));
    performancePanel->AddItem(std::move(threadCountInput));
    

    auto appearancePanel = Create<FlexPanel>(FlexDirection::DOWN);
    appearancePanel->parentSize = { 1.0f, 0.0f };
    appearancePanel->autoHeight = true;

    auto smokeColorColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    smokeColorColorRow->parentSize = { 1.0f, 0.0f };
    smokeColorColorRow->autoHeight = true;
    smokeColorColorRow->spacing = 10;
    smokeColorColorRow->padding = { 15, 0, 15, 10 };
    auto smokeColorColorInput = Create<ColorSelector>();
    smokeColorColorInput->UseConfigValue(SmokeTrailConfig::TRAIL_COLOR);
    smokeColorColorInput->size = { 60, 26 };
    smokeColorColorInput->border.cornerRadius = 2.0f;
    auto smokeColorColorLabel = Create<Label>(L"Trail color");
    smokeColorColorLabel->size = { 0, 26 };
    smokeColorColorLabel->yTextAlign = Alignment::CENTER;
    smokeColorColorLabel->SetProperty(FlexGrow());
    smokeColorColorRow->AddItem(std::move(smokeColorColorInput));
    smokeColorColorRow->AddItem(std::move(smokeColorColorLabel));
    
    auto trailWidthInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail width", L"How wide the smoke line is, in pixels",
        _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::TRAIL_WIDTH, Config::ADD_IF_MISSING),
        1, 100, 1
    ));
    trailWidthInput->GetInput()->size.Assign(Width(60));
    trailWidthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(SmokeTrailConfig::TRAIL_WIDTH.name, value.getAsInteger());
    }).Detach();
    
    auto trailEdgeFadeRangeInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail edge fade range", L"How wide the the fade out area of the smoke line is, in pixels",
        _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::TRAIL_EDGE_FADE_RANGE, Config::ADD_IF_MISSING),
        1, 100, 1
    ));
    trailEdgeFadeRangeInput->GetInput()->size.Assign(Width(60));
    trailEdgeFadeRangeInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(SmokeTrailConfig::TRAIL_EDGE_FADE_RANGE.name, value.getAsInteger());
    }).Detach();
    
    auto smokeDensityInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Trail density", L"The density of the smoke line. Density of 1 means fully saturated, but higher densities allow smoke to spread out more while remaining the same color",
        (float)_scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::TRAIL_DENSITY, Config::ADD_IF_MISSING),
        0.1f, 100.0f, 1.0f
    ));
    smokeDensityInput->GetInput()->size.Assign(Width(60));
    smokeDensityInput->GetInput()->precision = 1;
    smokeDensityInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeTrailConfig::TRAIL_DENSITY.name, value.getAsDouble());
    }).Detach();
    
    auto cursorWindWidthInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail wind width", L"The width of the area around your cursor that generates the wind. Values bigger than the smoke line width do nothing",
        _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::TRAIL_WIND_WIDTH, Config::ADD_IF_MISSING),
        1, 100, 1
    ));
    cursorWindWidthInput->GetInput()->size.Assign(Width(60));
    cursorWindWidthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(SmokeTrailConfig::TRAIL_WIND_WIDTH.name, value.getAsInteger());
    }).Detach();
    
    auto cursorWindSpeedInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Trail wind speed", L"Ratio between your cursor speed and generated wind speed. Large values can cause visual artifacts",
        (float)_scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::TRAIL_WIND_SPEED, Config::ADD_IF_MISSING),
        0.0f, 10.0f, 0.05f
    ));
    cursorWindSpeedInput->GetInput()->size.Assign(Width(60));
    cursorWindSpeedInput->GetInput()->precision = 2;
    cursorWindSpeedInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeTrailConfig::TRAIL_WIND_SPEED.name, value.getAsDouble());
    }).Detach();
    
    auto cursorTempInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Cursor temperature", L"How much heat your cursor generates. Heat makes surrounding smoke move upwards",
        (float)_scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::CURSOR_TEMP, Config::ADD_IF_MISSING),
        0.0f, 100.0f, 0.1f
    ));
    cursorTempInput->GetInput()->size.Assign(Width(60));
    cursorTempInput->GetInput()->precision = 1;
    cursorTempInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeTrailConfig::CURSOR_TEMP.name, value.getAsDouble());
    }).Detach();

    appearancePanel->AddItem(Create<SectionHeader>(L"Appearance"));
    appearancePanel->AddItem(std::move(smokeColorColorRow));
    appearancePanel->AddItem(std::move(trailWidthInput));
    appearancePanel->AddItem(std::move(trailEdgeFadeRangeInput));
    appearancePanel->AddItem(std::move(smokeDensityInput));
    appearancePanel->AddItem(std::move(cursorWindWidthInput));
    appearancePanel->AddItem(std::move(cursorWindSpeedInput));
    appearancePanel->AddItem(std::move(cursorTempInput));


    auto simulationPanel = Create<FlexPanel>(FlexDirection::DOWN);
    simulationPanel->parentSize = { 1.0f, 0.0f };
    simulationPanel->autoHeight = true;
    
    auto velocityDiffusionInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Velocity diffusion", L"The rate at which wind spreads out to nearby cells",
        (float)_scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::VELOCITY_DIFFUSION, Config::ADD_IF_MISSING),
        0.0f, 100.0f, 0.1f
    ));
    velocityDiffusionInput->GetInput()->size.Assign(Width(60));
    velocityDiffusionInput->GetInput()->precision = 1;
    velocityDiffusionInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeTrailConfig::VELOCITY_DIFFUSION.name, value.getAsDouble());
    }).Detach();
    
    auto densityDiffusionInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Density diffusion", L"The rate at which smoke spreads out to nearby cells",
        (float)_scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::DENSITY_DIFFUSION, Config::ADD_IF_MISSING),
        0.0f, 100.0f, 0.1f
    ));
    densityDiffusionInput->GetInput()->size.Assign(Width(60));
    densityDiffusionInput->GetInput()->precision = 1;
    densityDiffusionInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeTrailConfig::DENSITY_DIFFUSION.name, value.getAsDouble());
    }).Detach();
    
    auto temperatureDiffusionInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Temperature diffusion", L"The rate at which heat spreads out to nearby cells",
        (float)_scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::TEMPERATURE_DIFFUSION, Config::ADD_IF_MISSING),
        0.0f, 100.0f, 0.1f
    ));
    temperatureDiffusionInput->GetInput()->size.Assign(Width(60));
    temperatureDiffusionInput->GetInput()->precision = 1;
    temperatureDiffusionInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeTrailConfig::TEMPERATURE_DIFFUSION.name, value.getAsDouble());
    }).Detach();

    auto densityReductionRateInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Density reduction rate", L"How much smoke density is reduced in every cell every second. Controls how quickly the smoke disappears",
        (float)_scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::DENSITY_REDUCTION_RATE, Config::ADD_IF_MISSING),
        0.0f, 100.0f, 0.05f
    ));
    densityReductionRateInput->GetInput()->size.Assign(Width(60));
    densityReductionRateInput->GetInput()->precision = 2;
    densityReductionRateInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeTrailConfig::DENSITY_REDUCTION_RATE.name, value.getAsDouble());
    }).Detach();

    auto temperatureReductionRateInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Temperature reduction rate", L"How much temperature is reduced in every cell every second. Controls how quickly the smoke stops rising",
        (float)_scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::TEMPERATURE_REDUCTION_RATE, Config::ADD_IF_MISSING),
        0.0f, 100.0f, 0.05f
    ));
    temperatureReductionRateInput->GetInput()->size.Assign(Width(60));
    temperatureReductionRateInput->GetInput()->precision = 2;
    temperatureReductionRateInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeTrailConfig::TEMPERATURE_REDUCTION_RATE.name, value.getAsDouble());
    }).Detach();

    simulationPanel->AddItem(Create<SectionHeader>(L"Simulation"));
    simulationPanel->AddItem(std::move(velocityDiffusionInput));
    simulationPanel->AddItem(std::move(densityDiffusionInput));
    simulationPanel->AddItem(std::move(temperatureDiffusionInput));
    simulationPanel->AddItem(std::move(densityReductionRateInput));
    simulationPanel->AddItem(std::move(temperatureReductionRateInput));


    flexPanel->AddItem(std::move(generalPanel));
    flexPanel->AddItem(std::move(overlayLayoutSetupPanel));
    flexPanel->AddItem(std::move(performancePanel));
    flexPanel->AddItem(std::move(appearancePanel));
    flexPanel->AddItem(std::move(simulationPanel));

    AddItem(std::move(flexPanel));
}
