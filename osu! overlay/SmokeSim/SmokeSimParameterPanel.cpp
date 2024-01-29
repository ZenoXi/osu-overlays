#include "App.h"
#include "Scenes/Scene.h"
#include "Scenes/DefaultNonClientAreaScene.h"
#include "Scenes/DefaultTitleBarScene.h"
#include "Scenes/Scene.h"
#include "SmokeSimParameterPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Button.h"
#include "Components/Base/EmptyPanel.h"
#include "SmokeSim/SmokeSimScene.h"
#include "Helper/StringHelper.h"

void zcom::SmokeSimParameterPanel::Init(SmokeSimType simType)
{
    ScrollPanel::Init();
    _simType = simType;

    // Load smoke sim options. This also creates the config file and(or) sets the default values if necessary
    _trail_cellSize = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.cellSize").value_or(_trail_cellSize.Default());
    _trail_threadCount = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.threadCount").value_or(_trail_threadCount.Default());
    _trail_fullMonitor = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.fullMonitor").value_or(_trail_fullMonitor.Default());
    _trail_width = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.width").value_or(_trail_width.Default());
    _trail_height = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.height").value_or(_trail_height.Default());
    _trail_xOffset = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.xOffset").value_or(_trail_xOffset.Default());
    _trail_yOffset = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.yOffset").value_or(_trail_yOffset.Default());
    _smoke_cellSize = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.cellSize").value_or(_smoke_cellSize.Default());
    _smoke_threadCount = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.threadCount").value_or(_smoke_threadCount.Default());
    _smoke_fullMonitor = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.fullMonitor").value_or(_smoke_fullMonitor.Default());
    _smoke_width = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.width").value_or(_smoke_width.Default());
    _smoke_height = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.height").value_or(_smoke_height.Default());
    _smoke_xOffset = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.xOffset").value_or(_smoke_xOffset.Default());
    _smoke_yOffset = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.yOffset").value_or(_smoke_yOffset.Default());
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.cellSize", _trail_cellSize.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.threadCount", _trail_threadCount.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.fullMonitor", _trail_fullMonitor.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.width", _trail_width.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.height", _trail_height.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.xOffset", _trail_xOffset.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.yOffset", _trail_yOffset.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.cellSize", _smoke_cellSize.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.threadCount", _smoke_threadCount.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.fullMonitor", _smoke_fullMonitor.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.width", _smoke_width.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.height", _smoke_height.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.xOffset", _smoke_xOffset.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.yOffset", _smoke_yOffset.Get(), false);
    SmokeSimScene::SimParams simParams;
    simParams.trailWidth = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.trailWidth").value_or(simParams.trailWidth.Default());
    simParams.trailEdgeFadeRange = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.trailEdgeFadeRange").value_or(simParams.trailEdgeFadeRange.Default());
    simParams.trailDensity = _scene->GetApp()->options.GetDoubleValue(L"smokesim.cursortrail.trailDensity").value_or(simParams.trailDensity.Default());
    simParams.trailWindWidth = _scene->GetApp()->options.GetIntValue(L"smokesim.cursortrail.trailWindWidth").value_or(simParams.trailWindWidth.Default());
    simParams.trailWindSpeed = _scene->GetApp()->options.GetDoubleValue(L"smokesim.cursortrail.trailWindSpeed").value_or(simParams.trailWindSpeed.Default());
    simParams.cursorTemp = _scene->GetApp()->options.GetDoubleValue(L"smokesim.cursortrail.cursorTemp").value_or(simParams.cursorTemp.Default());
    simParams.trailVelocityDiffusion = _scene->GetApp()->options.GetDoubleValue(L"smokesim.cursortrail.velocityDiffusion").value_or(simParams.trailVelocityDiffusion.Default());
    simParams.trailDensityDiffusion = _scene->GetApp()->options.GetDoubleValue(L"smokesim.cursortrail.densityDiffusion").value_or(simParams.trailDensityDiffusion.Default());
    simParams.trailTemperatureDiffusion = _scene->GetApp()->options.GetDoubleValue(L"smokesim.cursortrail.temperatureDiffusion").value_or(simParams.trailTemperatureDiffusion.Default());
    simParams.trailDensityReductionRate = _scene->GetApp()->options.GetDoubleValue(L"smokesim.cursortrail.densityReductionRate").value_or(simParams.trailDensityReductionRate.Default());
    simParams.trailTemperatureReductionRate = _scene->GetApp()->options.GetDoubleValue(L"smokesim.cursortrail.temperatureReductionRate").value_or(simParams.trailTemperatureReductionRate.Default());
    simParams.brushWidth = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.brushWidth").value_or(simParams.brushWidth.Default());
    simParams.brushEdgeFadeRange = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.brushEdgeFadeRange").value_or(simParams.brushEdgeFadeRange.Default());
    simParams.smokeDensity = _scene->GetApp()->options.GetDoubleValue(L"smokesim.enhancedsmoke.smokeDensity").value_or(simParams.smokeDensity.Default());
    simParams.cursorWindWidth = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.cursorWindWidth").value_or(simParams.cursorWindWidth.Default());
    simParams.cursorWindSpeed = _scene->GetApp()->options.GetDoubleValue(L"smokesim.enhancedsmoke.cursorWindSpeed").value_or(simParams.cursorWindSpeed.Default());
    simParams.slowdownPersistenceDurationMs = _scene->GetApp()->options.GetIntValue(L"smokesim.enhancedsmoke.slowdownPersistenceDuration").value_or(simParams.slowdownPersistenceDurationMs.Default());
    simParams.smokeVelocityDiffusion = _scene->GetApp()->options.GetDoubleValue(L"smokesim.enhancedsmoke.velocityDiffusion").value_or(simParams.smokeVelocityDiffusion.Default());
    simParams.smokeDensityDiffusion = _scene->GetApp()->options.GetDoubleValue(L"smokesim.enhancedsmoke.densityDiffusion").value_or(simParams.smokeDensityDiffusion.Default());
    simParams.smokeDensityReductionRate = _scene->GetApp()->options.GetDoubleValue(L"smokesim.enhancedsmoke.densityReductionRate").value_or(simParams.smokeDensityReductionRate.Default());
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.trailWidth", simParams.trailWidth.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.trailEdgeFadeRange", simParams.trailEdgeFadeRange.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.trailDensity", simParams.trailDensity.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.trailWindWidth", simParams.trailWindWidth.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.trailWindSpeed", simParams.trailWindSpeed.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.cursorTemp", simParams.cursorTemp.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.velocityDiffusion", simParams.trailVelocityDiffusion.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.densityDiffusion", simParams.trailDensityDiffusion.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.temperatureDiffusion", simParams.trailTemperatureDiffusion.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.densityReductionRate", simParams.trailDensityReductionRate.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.temperatureReductionRate", simParams.trailTemperatureReductionRate.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.brushWidth", simParams.brushWidth.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.brushEdgeFadeRange", simParams.brushEdgeFadeRange.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.smokeDensity", simParams.smokeDensity.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.cursorWindWidth", simParams.cursorWindWidth.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.cursorWindSpeed", simParams.cursorWindSpeed.Get(), false);
    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.slowdownPersistenceDuration", simParams.slowdownPersistenceDurationMs.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.velocityDiffusion", simParams.smokeVelocityDiffusion.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.densityDiffusion", simParams.smokeDensityDiffusion.Get(), false);
    _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.densityReductionRate", simParams.smokeDensityReductionRate.Get(), false);
    _scene->GetApp()->options.SaveOptions();

    if (simType == SmokeSimType::ENHANCED_SMOKE)
    {
        Handle<zwnd::Window> windowHandle = _scene->GetApp()->FindWindowByClassName(L"enhancedSmokeOverlay");
        if (windowHandle.Valid())
            _overlayWindowId = windowHandle->GetWindowId();
    }
    else if (simType == SmokeSimType::CURSOR_TRAIL)
    {
        Handle<zwnd::Window> windowHandle = _scene->GetApp()->FindWindowByClassName(L"cursorTrailOverlay");
        if (windowHandle.Valid())
            _overlayWindowId = windowHandle->GetWindowId();
    }

    SetBackgroundColor(D2D1::ColorF(0x202020));
    Scrollable(Scrollbar::VERTICAL, true);
    ScrollBackgroundVisible(Scrollbar::VERTICAL, true);

    auto flexPanel = Create<FlexPanel>(FlexDirection::DOWN);
    flexPanel->FillContainerWidth();

    auto generalPanel = Create<FlexPanel>(FlexDirection::DOWN);
    generalPanel->FillContainerWidth();

    auto titleRow = Create<FlexPanel>(FlexDirection::RIGHT);
    titleRow->FillContainerWidth();
    titleRow->SetPadding({ 15, 15, 15, 15 });
    auto generalLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Cursor trail" : L"Enhanced smoke");
    generalLabel->SetBaseHeight(30);
    generalLabel->SetVerticalTextAlignment(Alignment::CENTER);
    generalLabel->SetFontSize(20.0f);
    generalLabel->SetProperty(FlexGrow());
    auto enableButton = Create<Button>(L"Enable");
    enableButton->SetBaseSize(90, 30);
    enableButton->SetBorderVisibility(false);
    enableButton->Text()->SetFontColor(D2D1::ColorF(0xEAEAEA));
    enableButton->SetButtonColor(D2D1::ColorF(0x307020));
    enableButton->SetButtonHoverColor(D2D1::ColorF(0x309020));
    enableButton->SetButtonClickColor(D2D1::ColorF(0x308020));
    enableButton->SetCornerRounding(2.0f);
    enableButton->SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));
    enableButton->SetProperty(PROP_Shadow{});
    if (!_overlayWindowId)
    {
        enableButton->Text()->SetText(L"Enable");
        enableButton->SetButtonColor(D2D1::ColorF(0x307020));
        enableButton->SetButtonHoverColor(D2D1::ColorF(0x309020));
        enableButton->SetButtonClickColor(D2D1::ColorF(0x308020));
    }
    else
    {
        enableButton->Text()->SetText(L"Disable");
        enableButton->SetButtonColor(D2D1::ColorF(0x703020));
        enableButton->SetButtonHoverColor(D2D1::ColorF(0x903020));
        enableButton->SetButtonClickColor(D2D1::ColorF(0x803020));
    }
    enableButton->SetActivation(ButtonActivation::RELEASE);
    enableButton->SubscribeOnActivated([&, button = enableButton.get()]() {
        if (!_overlayWindowId)
        {
            _OpenOverlayWindow();
            if (_overlayWindowId)
            {
                button->Text()->SetText(L"Disable");
                button->SetButtonColor(D2D1::ColorF(0x703020));
                button->SetButtonHoverColor(D2D1::ColorF(0x903020));
                button->SetButtonClickColor(D2D1::ColorF(0x803020));
            }
        }
        else
        {
            Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_overlayWindowId.value());
            if (handle.Valid())
                handle->Close();
            _overlayWindowId = std::nullopt;

            button->Text()->SetText(L"Enable");
            button->SetButtonColor(D2D1::ColorF(0x307020));
            button->SetButtonHoverColor(D2D1::ColorF(0x309020));
            button->SetButtonClickColor(D2D1::ColorF(0x308020));
        }
        _UpdateActiveItems();
    }).Detach();

    titleRow->AddItem(std::move(generalLabel));
    titleRow->AddItem(std::move(enableButton));

    auto cellSizeRow = Create<FlexPanel>(FlexDirection::RIGHT);
    cellSizeRow->FillContainerWidth();
    cellSizeRow->SetSpacing(10);
    cellSizeRow->SetPadding({ 15, 0, 15, 10 });
    _cellSizeInput = Create<NumberInput>();
    _cellSizeInput->SetBaseSize(50, 26);
    _cellSizeInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_cellSize.Get() : _smoke_cellSize.Get()));
    _cellSizeInput->SetMinValue(NumberInputValue(1));
    _cellSizeInput->SetMaxValue(NumberInputValue(100));
    _cellSizeInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    _cellSizeInput->SetCornerRounding(2.0f);
    _cellSizeInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.cellSize", value.getAsInteger());
        else
            _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.cellSize", value.getAsInteger());
    }).Detach();
    auto cellSizeLabel = Create<Label>(L"Cell size, in pixels");
    cellSizeLabel->SetBaseHeight(26);
    cellSizeLabel->SetVerticalTextAlignment(Alignment::CENTER);
    cellSizeLabel->SetProperty(FlexGrow());
    cellSizeRow->AddItem(_cellSizeInput.get());
    cellSizeRow->AddItem(std::move(cellSizeLabel));

    auto threadCountRow = Create<FlexPanel>(FlexDirection::RIGHT);
    threadCountRow->FillContainerWidth();
    threadCountRow->SetSpacing(10);
    threadCountRow->SetPadding({ 15, 0, 15, 10 });
    _threadCountInput = Create<NumberInput>();
    _threadCountInput->SetBaseSize(50, 26);
    _threadCountInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_threadCount.Get() : _smoke_threadCount.Get()));
    _threadCountInput->SetMinValue(NumberInputValue(1));
    _threadCountInput->SetMaxValue(NumberInputValue(64));
    _threadCountInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    _threadCountInput->SetCornerRounding(2.0f);
    _threadCountInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.threadCount", value.getAsInteger());
        else
            _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.threadCount", value.getAsInteger());
    }).Detach();
    auto threadCountLabel = Create<Label>(L"Thread count");
    threadCountLabel->SetBaseHeight(26);
    threadCountLabel->SetVerticalTextAlignment(Alignment::CENTER);
    threadCountLabel->SetProperty(FlexGrow());
    threadCountRow->AddItem(_threadCountInput.get());
    threadCountRow->AddItem(std::move(threadCountLabel));

    auto fullMonitorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    fullMonitorRow->FillContainerWidth();
    fullMonitorRow->SetSpacing(10);
    fullMonitorRow->SetPadding({ 15, 0, 15, 10 });
    _fullMonitorCheckbox = Create<Checkbox>();
    _fullMonitorCheckbox->SetBaseSize(20, 20);
    _fullMonitorCheckbox->SetBackgroundColor(D2D1::ColorF(0x101010));
    _fullMonitorCheckbox->SetCornerRounding(2.0f);
    _fullMonitorCheckbox->SetVerticalAlignment(Alignment::CENTER);
    _fullMonitorCheckbox->Checked(simType == SmokeSimType::CURSOR_TRAIL ? _trail_fullMonitor.Get() : _smoke_fullMonitor.Get());
    _fullMonitorCheckbox->SubscribeOnStateChanged([=](bool state) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.fullMonitor", state);
        else
            _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.fullMonitor", state);
        _UpdateActiveItems();
    }).Detach();
    auto fullMonitorLabel = Create<Label>(L"Fill entire screen");
    fullMonitorLabel->SetBaseHeight(26);
    fullMonitorLabel->SetVerticalTextAlignment(Alignment::CENTER);
    fullMonitorLabel->SetProperty(FlexGrow());
    fullMonitorRow->AddItem(_fullMonitorCheckbox.get());
    fullMonitorRow->AddItem(std::move(fullMonitorLabel));

    auto layoutSection = Create<FlexPanel>(FlexDirection::RIGHT);
    layoutSection->FillContainerWidth();
    layoutSection->SetSpacing(10);
    layoutSection->SetPadding({ 15, 0, 15, 10 });
    {
        auto sizeCol = Create<FlexPanel>(FlexDirection::DOWN);
        sizeCol->SetWidthFixed(true);
        sizeCol->SetProperty(FlexGrow());
        sizeCol->SetSpacing(15);
        {
            auto widthRow = Create<FlexPanel>(FlexDirection::RIGHT);
            widthRow->FillContainerWidth();
            widthRow->SetSpacing(10);
            _widthInput = Create<NumberInput>();
            _widthInput->SetBaseSize(70, 26);
            _widthInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_width.Get() : _smoke_width.Get()));
            _widthInput->SetMinValue(NumberInputValue(10));
            _widthInput->SetMaxValue(NumberInputValue(10000));
            _widthInput->SetStepSize(NumberInputValue(10));
            _widthInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _widthInput->SetCornerRounding(2.0f);
            _widthInput->AddOnValueChanged([=](NumberInputValue value) {
                if (simType == SmokeSimType::CURSOR_TRAIL)
                    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.width", value.getAsInteger());
                else
                    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.width", value.getAsInteger());
            }).Detach();
            auto widthLabel = Create<Label>(L"Width");
            widthLabel->SetBaseHeight(26);
            widthLabel->SetVerticalTextAlignment(Alignment::CENTER);
            widthLabel->SetProperty(FlexGrow());
            widthRow->AddItem(_widthInput.get());
            widthRow->AddItem(std::move(widthLabel));
            sizeCol->AddItem(std::move(widthRow));
        } {
            auto heightRow = Create<FlexPanel>(FlexDirection::RIGHT);
            heightRow->FillContainerWidth();
            heightRow->SetSpacing(10);
            _heightInput = Create<NumberInput>();
            _heightInput->SetBaseSize(70, 26);
            _heightInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_height.Get() : _smoke_height.Get()));
            _heightInput->SetMinValue(NumberInputValue(10));
            _heightInput->SetMaxValue(NumberInputValue(10000));
            _heightInput->SetStepSize(NumberInputValue(10));
            _heightInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _heightInput->SetCornerRounding(2.0f);
            _heightInput->AddOnValueChanged([=](NumberInputValue value) {
                if (simType == SmokeSimType::CURSOR_TRAIL)
                    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.height", value.getAsInteger());
                else
                    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.height", value.getAsInteger());
            }).Detach();
            auto heightLabel = Create<Label>(L"Height");
            heightLabel->SetBaseHeight(26);
            heightLabel->SetVerticalTextAlignment(Alignment::CENTER);
            heightLabel->SetProperty(FlexGrow());
            heightRow->AddItem(_heightInput.get());
            heightRow->AddItem(std::move(heightLabel));
            sizeCol->AddItem(std::move(heightRow));
        }
        layoutSection->AddItem(std::move(sizeCol));
    } {
        auto offsetCol = Create<FlexPanel>(FlexDirection::DOWN);
        offsetCol->SetWidthFixed(true);
        offsetCol->SetProperty(FlexGrow());
        offsetCol->SetSpacing(15);
        {
            auto xOffsetRow = Create<FlexPanel>(FlexDirection::RIGHT);
            xOffsetRow->FillContainerWidth();
            xOffsetRow->SetSpacing(10);
            _xOffsetInput = Create<NumberInput>();
            _xOffsetInput->SetBaseSize(70, 26);
            _xOffsetInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_xOffset.Get() : _smoke_xOffset.Get()));
            _xOffsetInput->SetMinValue(NumberInputValue(-10000));
            _xOffsetInput->SetMaxValue(NumberInputValue(10000));
            _xOffsetInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _xOffsetInput->SetCornerRounding(2.0f);
            _xOffsetInput->AddOnValueChanged([=](NumberInputValue value) {
                if (simType == SmokeSimType::CURSOR_TRAIL)
                    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.xOffset", value.getAsInteger());
                else
                    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.xOffset", value.getAsInteger());
            }).Detach();
            auto xOffsetLabel = Create<Label>(L"X offset");
            xOffsetLabel->SetBaseHeight(26);
            xOffsetLabel->SetVerticalTextAlignment(Alignment::CENTER);
            xOffsetLabel->SetProperty(FlexGrow());
            xOffsetRow->AddItem(_xOffsetInput.get());
            xOffsetRow->AddItem(std::move(xOffsetLabel));
            offsetCol->AddItem(std::move(xOffsetRow));
        } {
            auto yOffsetRow = Create<FlexPanel>(FlexDirection::RIGHT);
            yOffsetRow->FillContainerWidth();
            yOffsetRow->SetSpacing(10);
            _yOffsetInput = Create<NumberInput>();
            _yOffsetInput->SetBaseSize(70, 26);
            _yOffsetInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_yOffset.Get() : _smoke_yOffset.Get()));
            _yOffsetInput->SetMinValue(NumberInputValue(-10000));
            _yOffsetInput->SetMaxValue(NumberInputValue(10000));
            _yOffsetInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _yOffsetInput->SetCornerRounding(2.0f);
            _yOffsetInput->AddOnValueChanged([=](NumberInputValue value) {
                if (simType == SmokeSimType::CURSOR_TRAIL)
                    _scene->GetApp()->options.SetIntValue(L"smokesim.cursortrail.yOffset", value.getAsInteger());
                else
                    _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.yOffset", value.getAsInteger());
            }).Detach();
            auto yOffsetLabel = Create<Label>(L"Y offset");
            yOffsetLabel->SetBaseHeight(26);
            yOffsetLabel->SetVerticalTextAlignment(Alignment::CENTER);
            yOffsetLabel->SetProperty(FlexGrow());
            yOffsetRow->AddItem(_yOffsetInput.get());
            yOffsetRow->AddItem(std::move(yOffsetLabel));
            offsetCol->AddItem(std::move(yOffsetRow));
        }
        layoutSection->AddItem(std::move(offsetCol));
    }

    generalPanel->AddItem(std::move(titleRow));
    generalPanel->AddItem(std::move(cellSizeRow));
    generalPanel->AddItem(std::move(threadCountRow));
    generalPanel->AddItem(std::move(fullMonitorRow));
    generalPanel->AddItem(std::move(layoutSection));
    flexPanel->AddItem(std::move(generalPanel));

    auto interactionPanel = Create<FlexPanel>(FlexDirection::DOWN);
    interactionPanel->FillContainerWidth();

    auto interactionLabel = Create<Label>(L"Interaction");
    interactionLabel->SetBaseHeight(50);
    interactionLabel->SetParentWidthPercent(1.0f);
    interactionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    interactionLabel->SetFontSize(20.0f);
    interactionLabel->SetMargins({ 15.0f, 0.0f, 15.0f, 0.0f });

    auto trailWidthRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailWidthRow->FillContainerWidth();
    trailWidthRow->SetSpacing(10);
    trailWidthRow->SetPadding({ 15, 0, 15, 10 });
    auto trailWidthInput = Create<NumberInput>();
    trailWidthInput->SetBaseSize(60, 26);
    trailWidthInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailWidth.Get() : simParams.brushWidth.Get()));
    trailWidthInput->SetMinValue(NumberInputValue(1));
    trailWidthInput->SetMaxValue(NumberInputValue(100));
    trailWidthInput->SetStepSize(NumberInputValue(1));
    trailWidthInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailWidthInput->SetCornerRounding(2.0f);
    trailWidthInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.trailWidth", value.getAsDouble());
        else
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.brushWidth", value.getAsDouble());
    }).Detach();
    auto trailWidthLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail width" : L"Brush width");
    trailWidthLabel->SetBaseHeight(26);
    trailWidthLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailWidthLabel->SetProperty(FlexGrow());
    trailWidthRow->AddItem(std::move(trailWidthInput));
    trailWidthRow->AddItem(std::move(trailWidthLabel));

    auto trailEdgeFadeRangeRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailEdgeFadeRangeRow->FillContainerWidth();
    trailEdgeFadeRangeRow->SetSpacing(10);
    trailEdgeFadeRangeRow->SetPadding({ 15, 0, 15, 10 });
    auto trailEdgeFadeRangeInput = Create<NumberInput>();
    trailEdgeFadeRangeInput->SetBaseSize(60, 26);
    trailEdgeFadeRangeInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailEdgeFadeRange.Get() : simParams.brushEdgeFadeRange.Get()));
    trailEdgeFadeRangeInput->SetMinValue(NumberInputValue(1));
    trailEdgeFadeRangeInput->SetMaxValue(NumberInputValue(100));
    trailEdgeFadeRangeInput->SetStepSize(NumberInputValue(1));
    trailEdgeFadeRangeInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailEdgeFadeRangeInput->SetCornerRounding(2.0f);
    trailEdgeFadeRangeInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.trailEdgeFadeRange", value.getAsDouble());
        else
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.brushEdgeFadeRange", value.getAsDouble());
    }).Detach();
    auto trailEdgeFadeRangeLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail edge fade range" : L"Brush edge fade range");
    trailEdgeFadeRangeLabel->SetBaseHeight(26);
    trailEdgeFadeRangeLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailEdgeFadeRangeLabel->SetProperty(FlexGrow());
    trailEdgeFadeRangeRow->AddItem(std::move(trailEdgeFadeRangeInput));
    trailEdgeFadeRangeRow->AddItem(std::move(trailEdgeFadeRangeLabel));

    auto trailDensityRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailDensityRow->FillContainerWidth();
    trailDensityRow->SetSpacing(10);
    trailDensityRow->SetPadding({ 15, 0, 15, 10 });
    auto trailDensityInput = Create<NumberInput>();
    trailDensityInput->SetBaseSize(60, 26);
    trailDensityInput->SetPrecision(1);
    trailDensityInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailDensity.Get() : simParams.smokeDensity.Get()));
    trailDensityInput->SetMinValue(NumberInputValue("0.1"));
    trailDensityInput->SetMaxValue(NumberInputValue("100"));
    trailDensityInput->SetStepSize(NumberInputValue("0.1"));
    trailDensityInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailDensityInput->SetCornerRounding(2.0f);
    trailDensityInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.trailDensity", value.getAsDouble());
        else
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.smokeDensity", value.getAsDouble());
    }).Detach();
    auto trailDensityLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail density" : L"Smoke density");
    trailDensityLabel->SetBaseHeight(26);
    trailDensityLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailDensityLabel->SetProperty(FlexGrow());
    trailDensityRow->AddItem(std::move(trailDensityInput));
    trailDensityRow->AddItem(std::move(trailDensityLabel));

    auto trailWindWidthRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailWindWidthRow->FillContainerWidth();
    trailWindWidthRow->SetSpacing(10);
    trailWindWidthRow->SetPadding({ 15, 0, 15, 10 });
    auto trailWindWidthInput = Create<NumberInput>();
    trailWindWidthInput->SetBaseSize(60, 26);
    trailWindWidthInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailWindWidth.Get() : simParams.cursorWindWidth.Get()));
    trailWindWidthInput->SetMinValue(NumberInputValue(1));
    trailWindWidthInput->SetMaxValue(NumberInputValue(100));
    trailWindWidthInput->SetStepSize(NumberInputValue(1));
    trailWindWidthInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailWindWidthInput->SetCornerRounding(2.0f);
    trailWindWidthInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.trailWindWidth", value.getAsDouble());
        else
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.cursorWindWidth", value.getAsDouble());
    }).Detach();
    auto trailWindWidthLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail wind width" : L"Cursor wind width");
    trailWindWidthLabel->SetBaseHeight(26);
    trailWindWidthLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailWindWidthLabel->SetProperty(FlexGrow());
    trailWindWidthRow->AddItem(std::move(trailWindWidthInput));
    trailWindWidthRow->AddItem(std::move(trailWindWidthLabel));

    auto trailWindSpeedRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailWindSpeedRow->FillContainerWidth();
    trailWindSpeedRow->SetSpacing(10);
    trailWindSpeedRow->SetPadding({ 15, 0, 15, 10 });
    auto trailWindSpeedInput = Create<NumberInput>();
    trailWindSpeedInput->SetBaseSize(60, 26);
    trailWindSpeedInput->SetPrecision(2);
    trailWindSpeedInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailWindSpeed.Get() : simParams.cursorWindSpeed.Get()));
    trailWindSpeedInput->SetMinValue(NumberInputValue("0.1"));
    trailWindSpeedInput->SetMaxValue(NumberInputValue("10"));
    trailWindSpeedInput->SetStepSize(NumberInputValue("0.05"));
    trailWindSpeedInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailWindSpeedInput->SetCornerRounding(2.0f);
    trailWindSpeedInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.trailWindSpeed", value.getAsDouble());
        else
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.cursorWindSpeed", value.getAsDouble());
    }).Detach();
    auto trailWindSpeedLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail wind speed" : L"Cursor wind speed");
    trailWindSpeedLabel->SetBaseHeight(26);
    trailWindSpeedLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailWindSpeedLabel->SetProperty(FlexGrow());
    trailWindSpeedRow->AddItem(std::move(trailWindSpeedInput));
    trailWindSpeedRow->AddItem(std::move(trailWindSpeedLabel));

    auto cursorTempRow = Create<FlexPanel>(FlexDirection::RIGHT);
    cursorTempRow->FillContainerWidth();
    cursorTempRow->SetSpacing(10);
    cursorTempRow->SetPadding({ 15, 0, 15, 10 });
    auto cursorTempInput = Create<NumberInput>();
    cursorTempInput->SetBaseSize(60, 26);
    cursorTempInput->SetPrecision(1);
    cursorTempInput->SetValue(NumberInputValue(simParams.cursorTemp.Get()));
    cursorTempInput->SetMinValue(NumberInputValue("0"));
    cursorTempInput->SetMaxValue(NumberInputValue("100"));
    cursorTempInput->SetStepSize(NumberInputValue("0.1"));
    cursorTempInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    cursorTempInput->SetCornerRounding(2.0f);
    cursorTempInput->AddOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.cursorTemp", value.getAsDouble());
    }).Detach();
    auto cursorTempLabel = Create<Label>(L"Cursor temperature");
    cursorTempLabel->SetBaseHeight(26);
    cursorTempLabel->SetVerticalTextAlignment(Alignment::CENTER);
    cursorTempLabel->SetProperty(FlexGrow());
    cursorTempRow->AddItem(std::move(cursorTempInput));
    cursorTempRow->AddItem(std::move(cursorTempLabel));

    auto slowdownPersistenceDurationRow = Create<FlexPanel>(FlexDirection::RIGHT);
    slowdownPersistenceDurationRow->FillContainerWidth();
    slowdownPersistenceDurationRow->SetSpacing(10);
    slowdownPersistenceDurationRow->SetPadding({ 15, 0, 15, 10 });
    auto slowdownPersistenceDurationInput = Create<NumberInput>();
    slowdownPersistenceDurationInput->SetBaseSize(60, 26);
    slowdownPersistenceDurationInput->SetValue(NumberInputValue(simParams.slowdownPersistenceDurationMs.Get()));
    slowdownPersistenceDurationInput->SetMinValue(NumberInputValue(0));
    slowdownPersistenceDurationInput->SetMaxValue(NumberInputValue(5000));
    slowdownPersistenceDurationInput->SetStepSize(NumberInputValue(50));
    slowdownPersistenceDurationInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    slowdownPersistenceDurationInput->SetCornerRounding(2.0f);
    slowdownPersistenceDurationInput->AddOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->options.SetIntValue(L"smokesim.enhancedsmoke.slowdownPersistenceDuration", value.getAsInteger());
    }).Detach();
    auto slowdownPersistenceDurationLabel = Create<Label>(L"Slowdown persistence duration");
    slowdownPersistenceDurationLabel->SetBaseHeight(26);
    slowdownPersistenceDurationLabel->SetVerticalTextAlignment(Alignment::CENTER);
    slowdownPersistenceDurationLabel->SetProperty(FlexGrow());
    slowdownPersistenceDurationRow->AddItem(std::move(slowdownPersistenceDurationInput));
    slowdownPersistenceDurationRow->AddItem(std::move(slowdownPersistenceDurationLabel));

    interactionPanel->AddItem(std::move(interactionLabel));
    interactionPanel->AddItem(std::move(trailWidthRow));
    interactionPanel->AddItem(std::move(trailEdgeFadeRangeRow));
    interactionPanel->AddItem(std::move(trailDensityRow));
    interactionPanel->AddItem(std::move(trailWindWidthRow));
    interactionPanel->AddItem(std::move(trailWindSpeedRow));
    if (simType == SmokeSimType::CURSOR_TRAIL)
        interactionPanel->AddItem(std::move(cursorTempRow));
    else
        interactionPanel->AddItem(std::move(slowdownPersistenceDurationRow));
    flexPanel->AddItem(std::move(interactionPanel));

    auto simulationPanel = Create<FlexPanel>(FlexDirection::DOWN);
    simulationPanel->FillContainerWidth();

    auto simulationLabel = Create<Label>(L"Simulation");
    simulationLabel->SetBaseHeight(50);
    simulationLabel->SetParentWidthPercent(1.0f);
    simulationLabel->SetVerticalTextAlignment(Alignment::CENTER);
    simulationLabel->SetFontSize(20.0f);
    simulationLabel->SetMargins({ 15.0f, 0.0f, 15.0f, 0.0f });

    auto velocityDiffusionRow = Create<FlexPanel>(FlexDirection::RIGHT);
    velocityDiffusionRow->FillContainerWidth();
    velocityDiffusionRow->SetSpacing(10);
    velocityDiffusionRow->SetPadding({ 15, 0, 15, 10 });
    auto velocityDiffusionInput = Create<NumberInput>();
    velocityDiffusionInput->SetBaseSize(60, 26);
    velocityDiffusionInput->SetPrecision(1);
    velocityDiffusionInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailVelocityDiffusion.Get() : simParams.smokeVelocityDiffusion.Get()));
    velocityDiffusionInput->SetMinValue(NumberInputValue("0.1"));
    velocityDiffusionInput->SetMaxValue(NumberInputValue("100"));
    velocityDiffusionInput->SetStepSize(NumberInputValue("0.1"));
    velocityDiffusionInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    velocityDiffusionInput->SetCornerRounding(2.0f);
    velocityDiffusionInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.velocityDiffusion", value.getAsDouble());
        else
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.velocityDiffusion", value.getAsDouble());
    }).Detach();
    auto velocityDiffusionLabel = Create<Label>(L"Velocity diffusion");
    velocityDiffusionLabel->SetBaseHeight(26);
    velocityDiffusionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    velocityDiffusionLabel->SetProperty(FlexGrow());
    velocityDiffusionRow->AddItem(std::move(velocityDiffusionInput));
    velocityDiffusionRow->AddItem(std::move(velocityDiffusionLabel));

    auto densityDiffusionRow = Create<FlexPanel>(FlexDirection::RIGHT);
    densityDiffusionRow->FillContainerWidth();
    densityDiffusionRow->SetSpacing(10);
    densityDiffusionRow->SetPadding({ 15, 0, 15, 10 });
    auto densityDiffusionInput = Create<NumberInput>();
    densityDiffusionInput->SetBaseSize(60, 26);
    densityDiffusionInput->SetPrecision(1);
    densityDiffusionInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailDensityDiffusion.Get() : simParams.smokeDensityDiffusion.Get()));
    densityDiffusionInput->SetMinValue(NumberInputValue("0.1"));
    densityDiffusionInput->SetMaxValue(NumberInputValue("100"));
    densityDiffusionInput->SetStepSize(NumberInputValue("0.1"));
    densityDiffusionInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    densityDiffusionInput->SetCornerRounding(2.0f);
    densityDiffusionInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.densityDiffusion", value.getAsDouble());
        else
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.densityDiffusion", value.getAsDouble());
    }).Detach();
    auto densityDiffusionLabel = Create<Label>(L"Density diffusion");
    densityDiffusionLabel->SetBaseHeight(26);
    densityDiffusionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    densityDiffusionLabel->SetProperty(FlexGrow());
    densityDiffusionRow->AddItem(std::move(densityDiffusionInput));
    densityDiffusionRow->AddItem(std::move(densityDiffusionLabel));

    auto temperatureDiffusionRow = Create<FlexPanel>(FlexDirection::RIGHT);
    temperatureDiffusionRow->FillContainerWidth();
    temperatureDiffusionRow->SetSpacing(10);
    temperatureDiffusionRow->SetPadding({ 15, 0, 15, 10 });
    auto temperatureDiffusionInput = Create<NumberInput>();
    temperatureDiffusionInput->SetBaseSize(60, 26);
    temperatureDiffusionInput->SetPrecision(1);
    temperatureDiffusionInput->SetValue(NumberInputValue(simParams.trailTemperatureDiffusion.Get()));
    temperatureDiffusionInput->SetMinValue(NumberInputValue("0.1"));
    temperatureDiffusionInput->SetMaxValue(NumberInputValue("100"));
    temperatureDiffusionInput->SetStepSize(NumberInputValue("0.1"));
    temperatureDiffusionInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    temperatureDiffusionInput->SetCornerRounding(2.0f);
    temperatureDiffusionInput->AddOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.temperatureDiffusion", value.getAsDouble());
    }).Detach();
    auto temperatureDiffusionLabel = Create<Label>(L"Temperature diffusion");
    temperatureDiffusionLabel->SetBaseHeight(26);
    temperatureDiffusionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    temperatureDiffusionLabel->SetProperty(FlexGrow());
    temperatureDiffusionRow->AddItem(std::move(temperatureDiffusionInput));
    temperatureDiffusionRow->AddItem(std::move(temperatureDiffusionLabel));

    auto densityReductionRateRow = Create<FlexPanel>(FlexDirection::RIGHT);
    densityReductionRateRow->FillContainerWidth();
    densityReductionRateRow->SetSpacing(10);
    densityReductionRateRow->SetPadding({ 15, 0, 15, 10 });
    auto densityReductionRateInput = Create<NumberInput>();
    densityReductionRateInput->SetBaseSize(60, 26);
    densityReductionRateInput->SetPrecision(2);
    densityReductionRateInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailDensityReductionRate.Get() : simParams.smokeDensityReductionRate.Get()));
    densityReductionRateInput->SetMinValue(NumberInputValue("0"));
    densityReductionRateInput->SetMaxValue(NumberInputValue("100"));
    densityReductionRateInput->SetStepSize(NumberInputValue("0.05"));
    densityReductionRateInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    densityReductionRateInput->SetCornerRounding(2.0f);
    densityReductionRateInput->AddOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.densityReductionRate", value.getAsDouble());
        else
            _scene->GetApp()->options.SetDoubleValue(L"smokesim.enhancedsmoke.densityReductionRate", value.getAsDouble());
    }).Detach();
    auto densityReductionRateLabel = Create<Label>(L"Density reduction rate");
    densityReductionRateLabel->SetBaseHeight(26);
    densityReductionRateLabel->SetVerticalTextAlignment(Alignment::CENTER);
    densityReductionRateLabel->SetProperty(FlexGrow());
    densityReductionRateRow->AddItem(std::move(densityReductionRateInput));
    densityReductionRateRow->AddItem(std::move(densityReductionRateLabel));

    auto temperatureReductionRateRow = Create<FlexPanel>(FlexDirection::RIGHT);
    temperatureReductionRateRow->FillContainerWidth();
    temperatureReductionRateRow->SetSpacing(10);
    temperatureReductionRateRow->SetPadding({ 15, 0, 15, 10 });
    auto temperatureReductionRateInput = Create<NumberInput>();
    temperatureReductionRateInput->SetBaseSize(60, 26);
    temperatureReductionRateInput->SetPrecision(2);
    temperatureReductionRateInput->SetValue(NumberInputValue(simParams.trailTemperatureReductionRate.Get()));
    temperatureReductionRateInput->SetMinValue(NumberInputValue("0"));
    temperatureReductionRateInput->SetMaxValue(NumberInputValue("100"));
    temperatureReductionRateInput->SetStepSize(NumberInputValue("0.05"));
    temperatureReductionRateInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    temperatureReductionRateInput->SetCornerRounding(2.0f);
    temperatureReductionRateInput->AddOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->options.SetDoubleValue(L"smokesim.cursortrail.temperatureReductionRate", value.getAsDouble());
    }).Detach();
    auto temperatureReductionRateLabel = Create<Label>(L"Temperature reduction rate");
    temperatureReductionRateLabel->SetBaseHeight(26);
    temperatureReductionRateLabel->SetVerticalTextAlignment(Alignment::CENTER);
    temperatureReductionRateLabel->SetProperty(FlexGrow());
    temperatureReductionRateRow->AddItem(std::move(temperatureReductionRateInput));
    temperatureReductionRateRow->AddItem(std::move(temperatureReductionRateLabel));

    simulationPanel->AddItem(std::move(simulationLabel));
    simulationPanel->AddItem(std::move(velocityDiffusionRow));
    simulationPanel->AddItem(std::move(densityDiffusionRow));
    if (simType == SmokeSimType::CURSOR_TRAIL)
        simulationPanel->AddItem(std::move(temperatureDiffusionRow));
    simulationPanel->AddItem(std::move(densityReductionRateRow));
    if (simType == SmokeSimType::CURSOR_TRAIL)
        simulationPanel->AddItem(std::move(temperatureReductionRateRow));
    flexPanel->AddItem(std::move(simulationPanel));

    AddItem(std::move(flexPanel));

    _UpdateActiveItems();
}

void zcom::SmokeSimParameterPanel::_OpenOverlayWindow()
{
    std::wstring wndClass = _simType == SmokeSimType::CURSOR_TRAIL ? L"cursorTrailOverlay" : L"enhancedSmokeOverlay";
    int width;
    int height;
    std::optional<int> xOffset = std::nullopt;
    std::optional<int> yOffset = std::nullopt;
    if (_fullMonitorCheckbox->Checked())
    {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        width = _widthInput->GetValue().getAsInteger();
        height = _heightInput->GetValue().getAsInteger();
        xOffset = _xOffsetInput->GetValue().getAsInteger();
        yOffset = _yOffsetInput->GetValue().getAsInteger();
    }

    auto props = zwnd::WindowProperties()
        .WindowClassName(wndClass)
        .InitialSize(width, height)
        .IgnoreTaskbarForPlacement()
        .TopMost()
        .DisableWindowAnimations()
        .DisableWindowActivation()
        .DisableMouseInteraction()
        .DisableFastTooltips();
    if (xOffset)
        props.InitialXOffset(xOffset.value());
    if (yOffset)
        props.InitialYOffset(yOffset.value());

    _overlayWindowId = _scene->GetApp()->CreateChildWindow(
        _scene->GetWindow()->GetWindowId(),
        props,
        [=](zwnd::Window* wnd) {
            // Remove window decorations
            DefaultNonClientAreaSceneOptions ncOpt;
            ncOpt.drawWindowShadow = false;
            ncOpt.drawWindowBorder = false;
            ncOpt.resizingBorderWidths = { 7, 7, 7, 7 };
            ncOpt.clientAreaMargins = { 0, 0, 0, 0 };
            wnd->LoadNonClientAreaScene<DefaultNonClientAreaScene>(&ncOpt);

            DefaultTitleBarSceneOptions tbOpt;
            tbOpt.showCloseButton = false;
            tbOpt.showMaximizeButton = false;
            tbOpt.showMinimizeButton = false;
            tbOpt.showTitle = false;
            tbOpt.showIcon = false;
            tbOpt.titleBarHeight = 0;
            // Make entire window act as caption to enable easy window moving
            tbOpt.captionHeight = 10000;
            wnd->LoadTitleBarScene<DefaultTitleBarScene>(&tbOpt);

            SmokeSimSceneOptions opt;
            opt.simType = _simType;
            opt.cellSize = _cellSizeInput->GetValue().getAsInteger();
            opt.maxThreads = _threadCountInput->GetValue().getAsInteger();
            wnd->LoadStartingScene<SmokeSimScene>(&opt);
        }
    );
}

void zcom::SmokeSimParameterPanel::_UpdateActiveItems()
{
    _cellSizeInput->SetActive(!_overlayWindowId);
    _threadCountInput->SetActive(!_overlayWindowId);
    _fullMonitorCheckbox->SetActive(!_overlayWindowId);
    _widthInput->SetActive(!_overlayWindowId && !_fullMonitorCheckbox->Checked());
    _heightInput->SetActive(!_overlayWindowId && !_fullMonitorCheckbox->Checked());
    _xOffsetInput->SetActive(!_overlayWindowId && !_fullMonitorCheckbox->Checked());
    _yOffsetInput->SetActive(!_overlayWindowId && !_fullMonitorCheckbox->Checked());
}