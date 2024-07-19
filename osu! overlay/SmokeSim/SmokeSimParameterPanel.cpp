#include "App.h"
#include "Scenes/Scene.h"
#include "Scenes/DefaultNonClientAreaScene.h"
#include "Scenes/DefaultTitleBarScene.h"
#include "Scenes/Scene.h"
#include "SmokeSimParameterPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Button.h"
#include "Components/Base/Dummy.h"
#include "Components/Base/KeySelector.h"
#include "SmokeSimScene.h"
#include "SmokeSimConfig.h"
#include "Shared/Scenes/ColorSelectorScene.h"
#include "Shared/Util/Color.h"
#include "Helper/StringHelper.h"

void zcom::SmokeSimParameterPanel::Init(SmokeSimType simType)
{
    ScrollPanel::Init();
    _simType = simType;

    // Load smoke sim config. This also creates the config file and sets the default values where necessary
    _trail_cellSize = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::CURSOR_TRAIL_CELL_SIZE, Config::ADD_IF_MISSING);
    _trail_threadCount = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::CURSOR_TRAIL_THREAD_COUNT, Config::ADD_IF_MISSING);
    _trail_fullMonitor = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::CURSOR_TRAIL_FULL_MONITOR, Config::ADD_IF_MISSING);
    _trail_width = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::CURSOR_TRAIL_WIDTH, Config::ADD_IF_MISSING);
    _trail_height = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::CURSOR_TRAIL_HEIGHT, Config::ADD_IF_MISSING);
    _trail_xOffset = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::CURSOR_TRAIL_X_OFFSET, Config::ADD_IF_MISSING);
    _trail_yOffset = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::CURSOR_TRAIL_Y_OFFSET, Config::ADD_IF_MISSING);
    _smoke_cellSize = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::ENHANCED_SMOKE_CELL_SIZE, Config::ADD_IF_MISSING);
    _smoke_threadCount = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::ENHANCED_SMOKE_THREAD_COUNT, Config::ADD_IF_MISSING);
    _smoke_fullMonitor = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::ENHANCED_SMOKE_FULL_MONITOR, Config::ADD_IF_MISSING);
    _smoke_width = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::ENHANCED_SMOKE_WIDTH, Config::ADD_IF_MISSING);
    _smoke_height = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::ENHANCED_SMOKE_HEIGHT, Config::ADD_IF_MISSING);
    _smoke_xOffset = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::ENHANCED_SMOKE_X_OFFSET, Config::ADD_IF_MISSING);
    _smoke_yOffset = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::ENHANCED_SMOKE_Y_OFFSET, Config::ADD_IF_MISSING);
    SmokeSimParams simParams = {};
    simParams.trailColor = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::TRAIL_COLOR, Config::ADD_IF_MISSING);
    simParams.trailWidth = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::TRAIL_WIDTH, Config::ADD_IF_MISSING);
    simParams.trailEdgeFadeRange = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::TRAIL_EDGE_FADE_RANGE, Config::ADD_IF_MISSING);
    simParams.trailDensity = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::TRAIL_DENSITY, Config::ADD_IF_MISSING);
    simParams.trailWindWidth = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::TRAIL_WIND_WIDTH, Config::ADD_IF_MISSING);
    simParams.trailWindSpeed = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::TRAIL_WIND_SPEED, Config::ADD_IF_MISSING);
    simParams.cursorTemp = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::CURSOR_TEMP, Config::ADD_IF_MISSING);
    simParams.trailVelocityDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::TRAIL_VELOCITY_DIFFUSION, Config::ADD_IF_MISSING);
    simParams.trailDensityDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::TRAIL_DENSITY_DIFFUSION, Config::ADD_IF_MISSING);
    simParams.trailTemperatureDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::TRAIL_TEMPERATURE_DIFFUSION, Config::ADD_IF_MISSING);
    simParams.trailDensityReductionRate = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::TRAIL_DENSITY_REDUCTION_RATE, Config::ADD_IF_MISSING);
    simParams.trailTemperatureReductionRate = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::TRAIL_TEMPERATURE_REDUCTION_RATE, Config::ADD_IF_MISSING);
    simParams.smokeColor = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::SMOKE_COLOR, Config::ADD_IF_MISSING);
    simParams.brushWidth = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::BRUSH_WIDTH, Config::ADD_IF_MISSING);
    simParams.brushEdgeFadeRange = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::BRUSH_EDGE_FADE_RANGE, Config::ADD_IF_MISSING);
    simParams.smokeDensity = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::SMOKE_DENSITY, Config::ADD_IF_MISSING);
    simParams.cursorWindWidth = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::CURSOR_WIND_WIDTH, Config::ADD_IF_MISSING);
    simParams.cursorWindSpeed = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::CURSOR_WIND_SPEED, Config::ADD_IF_MISSING);
    simParams.slowdownPersistenceDurationMs = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::SLOWDOWN_PERSISTENCE_DURATION, Config::ADD_IF_MISSING);
    simParams.smokeVelocityDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::SMOKE_VELOCITY_DIFFUSION, Config::ADD_IF_MISSING);
    simParams.smokeDensityDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::SMOKE_DENSITY_DIFFUSION, Config::ADD_IF_MISSING);
    simParams.smokeDensityReductionRate = _scene->GetApp()->config.GetDoubleConfigValue(SmokeSimConfig::SMOKE_DENSITY_REDUCTION_RATE, Config::ADD_IF_MISSING);
    simParams.smokeKeyCode = _scene->GetApp()->config.GetIntConfigValue(SmokeSimConfig::SMOKE_KEY_CODE, Config::ADD_IF_MISSING);
    _scene->GetApp()->config.SaveConfig();

    if (simType == SmokeSimType::ENHANCED_SMOKE)
    {
        Handle<zwnd::Window> overlayWindowHandle = _scene->GetApp()->FindWindowByClassName(SmokeSimConfig::ENHANCED_SMOKE_OVERLAY_WINDOW_NAME);
        if (overlayWindowHandle.Valid())
            _overlayWindowId = overlayWindowHandle->GetWindowId();
        Handle<zwnd::Window> colorSelectorWindowHandle = _scene->GetApp()->FindWindowByClassName(SmokeSimConfig::ENHANCED_SMOKE_COLOR_SELECTOR_WINDOW_NAME);
        if (colorSelectorWindowHandle.Valid())
            _colorSelectorWindowId = colorSelectorWindowHandle->GetWindowId();
    }
    else if (simType == SmokeSimType::CURSOR_TRAIL)
    {
        Handle<zwnd::Window> overlayWindowHandle = _scene->GetApp()->FindWindowByClassName(SmokeSimConfig::CURSOR_TRAIL_OVERLAY_WINDOW_NAME);
        if (overlayWindowHandle.Valid())
            _overlayWindowId = overlayWindowHandle->GetWindowId();
        Handle<zwnd::Window> colorSelectorWindowHandle = _scene->GetApp()->FindWindowByClassName(SmokeSimConfig::CURSOR_TRAIL_COLOR_SELECTOR_WINDOW_NAME);
        if (colorSelectorWindowHandle.Valid())
            _colorSelectorWindowId = colorSelectorWindowHandle->GetWindowId();
    }

    _windowClosedEventSubscription = _scene->GetApp()->SubscribeOnWindowClosed([=](zwnd::WindowId id) {
        ExecuteSynchronously([=] {
            if (_colorSelectorWindowId.has_value() && _colorSelectorWindowId.value() == id)
                _colorSelectorWindowId = std::nullopt;
        });
    });

    _configChangedEventSubscription = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        if (changes)
        {
            bool colorChanged;
            if (_simType == SmokeSimType::CURSOR_TRAIL)
                colorChanged = changes.value().first == SmokeSimConfig::TRAIL_COLOR.name;
            else
                colorChanged = changes.value().first == SmokeSimConfig::SMOKE_COLOR.name;

            if (colorChanged)
                ExecuteSynchronously([=]() { _UpdateColorInput(); });
        }
    });

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
    enableButton->Label()->SetFontColor(D2D1::ColorF(0xEAEAEA));
    enableButton->SetButtonColor(D2D1::ColorF(0x307020));
    enableButton->SetButtonHoverColor(D2D1::ColorF(0x309020));
    enableButton->SetButtonClickColor(D2D1::ColorF(0x308020));
    enableButton->SetCornerRounding(2.0f);
    enableButton->SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));
    enableButton->SetProperty(PROP_Shadow{});
    if (!_overlayWindowId)
    {
        enableButton->Label()->SetText(L"Enable");
        enableButton->SetButtonColor(D2D1::ColorF(0x307020));
        enableButton->SetButtonHoverColor(D2D1::ColorF(0x309020));
        enableButton->SetButtonClickColor(D2D1::ColorF(0x308020));
    }
    else
    {
        enableButton->Label()->SetText(L"Disable");
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
                button->Label()->SetText(L"Disable");
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

            button->Label()->SetText(L"Enable");
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
    _cellSizeInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_cellSize : _smoke_cellSize));
    _cellSizeInput->SetMinValue(NumberInputValue(1));
    _cellSizeInput->SetMaxValue(NumberInputValue(100));
    _cellSizeInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    _cellSizeInput->SetCornerRounding(2.0f);
    _cellSizeInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetIntValue(SmokeSimConfig::CURSOR_TRAIL_CELL_SIZE.name, value.getAsInteger());
        else
            _scene->GetApp()->config.SetIntValue(SmokeSimConfig::ENHANCED_SMOKE_CELL_SIZE.name, value.getAsInteger());
    }).Detach();
    auto cellSizeLabel = Create<Label>(L"Cell size, in pixels");
    cellSizeLabel->SetBaseHeight(26);
    cellSizeLabel->SetVerticalTextAlignment(Alignment::CENTER);
    cellSizeLabel->SetProperty(FlexGrow());
    cellSizeLabel->SetHoverText(L"Size of the simulation grid cell. Smaller values have a very large impact on performance");
    cellSizeRow->AddItem(_cellSizeInput.get());
    cellSizeRow->AddItem(std::move(cellSizeLabel));

    auto threadCountRow = Create<FlexPanel>(FlexDirection::RIGHT);
    threadCountRow->FillContainerWidth();
    threadCountRow->SetSpacing(10);
    threadCountRow->SetPadding({ 15, 0, 15, 10 });
    _threadCountInput = Create<NumberInput>();
    _threadCountInput->SetBaseSize(50, 26);
    _threadCountInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_threadCount : _smoke_threadCount));
    _threadCountInput->SetMinValue(NumberInputValue(1));
    _threadCountInput->SetMaxValue(NumberInputValue(64));
    _threadCountInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    _threadCountInput->SetCornerRounding(2.0f);
    _threadCountInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetIntValue(SmokeSimConfig::CURSOR_TRAIL_THREAD_COUNT.name, value.getAsInteger());
        else
            _scene->GetApp()->config.SetIntValue(SmokeSimConfig::ENHANCED_SMOKE_THREAD_COUNT.name, value.getAsInteger());
    }).Detach();
    auto threadCountLabel = Create<Label>(L"CPU Thread count");
    threadCountLabel->SetBaseHeight(26);
    threadCountLabel->SetVerticalTextAlignment(Alignment::CENTER);
    threadCountLabel->SetProperty(FlexGrow());
    threadCountLabel->SetHoverText(L"How many CPU threads to use for the simulation when hardware acceleration is unavailable. If you have a CPU with many cores, increasing this setting should improve performance. Higher values offer diminishing permormance increases or even reduce performance");
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
    _fullMonitorCheckbox->Checked(simType == SmokeSimType::CURSOR_TRAIL ? _trail_fullMonitor : _smoke_fullMonitor);
    _fullMonitorCheckbox->SubscribeOnStateChanged([=](bool state) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetIntValue(SmokeSimConfig::CURSOR_TRAIL_FULL_MONITOR.name, state);
        else
            _scene->GetApp()->config.SetIntValue(SmokeSimConfig::ENHANCED_SMOKE_FULL_MONITOR.name, state);
        _UpdateActiveItems();
    }).Detach();
    auto fullMonitorLabel = Create<Label>(L"Fill entire screen");
    fullMonitorLabel->SetBaseHeight(26);
    fullMonitorLabel->SetVerticalTextAlignment(Alignment::CENTER);
    fullMonitorLabel->SetProperty(FlexGrow());
    fullMonitorLabel->SetHoverText(L"When checked, the overlay will use the entire area of your primary monitor (should match your osu! window). When unchecked, you can manually position the overlay window. Size and position of the overlay window is shown briefly with a red outline after enabling the overlay");
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
        sizeCol->SetSpacing(10);
        {
            auto widthRow = Create<FlexPanel>(FlexDirection::RIGHT);
            widthRow->FillContainerWidth();
            widthRow->SetSpacing(10);
            _widthInput = Create<NumberInput>();
            _widthInput->SetBaseSize(70, 26);
            _widthInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_width : _smoke_width));
            _widthInput->SetMinValue(NumberInputValue(10));
            _widthInput->SetMaxValue(NumberInputValue(10000));
            _widthInput->SetStepSize(NumberInputValue(10));
            _widthInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _widthInput->SetCornerRounding(2.0f);
            _widthInput->SubscribeOnValueChanged([=](NumberInputValue value) {
                if (simType == SmokeSimType::CURSOR_TRAIL)
                    _scene->GetApp()->config.SetIntValue(SmokeSimConfig::CURSOR_TRAIL_WIDTH.name, value.getAsInteger());
                else
                    _scene->GetApp()->config.SetIntValue(SmokeSimConfig::ENHANCED_SMOKE_WIDTH.name, value.getAsInteger());
            }).Detach();
            auto widthLabel = Create<Label>(L"Width");
            widthLabel->SetBaseHeight(26);
            widthLabel->SetVerticalTextAlignment(Alignment::CENTER);
            widthLabel->SetProperty(FlexGrow());
            widthLabel->SetHoverText(L"Width of the overlay, in pixels");
            widthRow->AddItem(_widthInput.get());
            widthRow->AddItem(std::move(widthLabel));
            sizeCol->AddItem(std::move(widthRow));
        } {
            auto heightRow = Create<FlexPanel>(FlexDirection::RIGHT);
            heightRow->FillContainerWidth();
            heightRow->SetSpacing(10);
            _heightInput = Create<NumberInput>();
            _heightInput->SetBaseSize(70, 26);
            _heightInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_height : _smoke_height));
            _heightInput->SetMinValue(NumberInputValue(10));
            _heightInput->SetMaxValue(NumberInputValue(10000));
            _heightInput->SetStepSize(NumberInputValue(10));
            _heightInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _heightInput->SetCornerRounding(2.0f);
            _heightInput->SubscribeOnValueChanged([=](NumberInputValue value) {
                if (simType == SmokeSimType::CURSOR_TRAIL)
                    _scene->GetApp()->config.SetIntValue(SmokeSimConfig::CURSOR_TRAIL_HEIGHT.name, value.getAsInteger());
                else
                    _scene->GetApp()->config.SetIntValue(SmokeSimConfig::ENHANCED_SMOKE_HEIGHT.name, value.getAsInteger());
            }).Detach();
            auto heightLabel = Create<Label>(L"Height");
            heightLabel->SetBaseHeight(26);
            heightLabel->SetVerticalTextAlignment(Alignment::CENTER);
            heightLabel->SetProperty(FlexGrow());
            heightLabel->SetHoverText(L"Height of the overlay, in pixels");
            heightRow->AddItem(_heightInput.get());
            heightRow->AddItem(std::move(heightLabel));
            sizeCol->AddItem(std::move(heightRow));
        }
        layoutSection->AddItem(std::move(sizeCol));
    } {
        auto offsetCol = Create<FlexPanel>(FlexDirection::DOWN);
        offsetCol->SetWidthFixed(true);
        offsetCol->SetProperty(FlexGrow());
        offsetCol->SetSpacing(10);
        {
            auto xOffsetRow = Create<FlexPanel>(FlexDirection::RIGHT);
            xOffsetRow->FillContainerWidth();
            xOffsetRow->SetSpacing(10);
            _xOffsetInput = Create<NumberInput>();
            _xOffsetInput->SetBaseSize(70, 26);
            _xOffsetInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_xOffset : _smoke_xOffset));
            _xOffsetInput->SetMinValue(NumberInputValue(-10000));
            _xOffsetInput->SetMaxValue(NumberInputValue(10000));
            _xOffsetInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _xOffsetInput->SetCornerRounding(2.0f);
            _xOffsetInput->SubscribeOnValueChanged([=](NumberInputValue value) {
                if (simType == SmokeSimType::CURSOR_TRAIL)
                    _scene->GetApp()->config.SetIntValue(SmokeSimConfig::CURSOR_TRAIL_X_OFFSET.name, value.getAsInteger());
                else
                    _scene->GetApp()->config.SetIntValue(SmokeSimConfig::ENHANCED_SMOKE_X_OFFSET.name, value.getAsInteger());
            }).Detach();
            auto xOffsetLabel = Create<Label>(L"X offset");
            xOffsetLabel->SetBaseHeight(26);
            xOffsetLabel->SetVerticalTextAlignment(Alignment::CENTER);
            xOffsetLabel->SetProperty(FlexGrow());
            xOffsetLabel->SetHoverText(L"How much to shift the overlay horizontally, in pixels");
            xOffsetRow->AddItem(_xOffsetInput.get());
            xOffsetRow->AddItem(std::move(xOffsetLabel));
            offsetCol->AddItem(std::move(xOffsetRow));
        } {
            auto yOffsetRow = Create<FlexPanel>(FlexDirection::RIGHT);
            yOffsetRow->FillContainerWidth();
            yOffsetRow->SetSpacing(10);
            _yOffsetInput = Create<NumberInput>();
            _yOffsetInput->SetBaseSize(70, 26);
            _yOffsetInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? _trail_yOffset : _smoke_yOffset));
            _yOffsetInput->SetMinValue(NumberInputValue(-10000));
            _yOffsetInput->SetMaxValue(NumberInputValue(10000));
            _yOffsetInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _yOffsetInput->SetCornerRounding(2.0f);
            _yOffsetInput->SubscribeOnValueChanged([=](NumberInputValue value) {
                if (simType == SmokeSimType::CURSOR_TRAIL)
                    _scene->GetApp()->config.SetIntValue(SmokeSimConfig::CURSOR_TRAIL_Y_OFFSET.name, value.getAsInteger());
                else
                    _scene->GetApp()->config.SetIntValue(SmokeSimConfig::ENHANCED_SMOKE_Y_OFFSET.name, value.getAsInteger());
            }).Detach();
            auto yOffsetLabel = Create<Label>(L"Y offset");
            yOffsetLabel->SetBaseHeight(26);
            yOffsetLabel->SetVerticalTextAlignment(Alignment::CENTER);
            yOffsetLabel->SetProperty(FlexGrow());
            yOffsetLabel->SetHoverText(L"How much to shift the overlay vertically, in pixels");
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
    interactionLabel->SetPadding({ 15.0f, 0.0f, 15.0f, 0.0f });

    auto keySelectorRowPanel = Create<Panel>();
    keySelectorRowPanel->SetParentWidthPercent(1.0f);
    keySelectorRowPanel->SetBaseHeight(26 + 10);
    keySelectorRowPanel->SetPadding({ 15, 0, 15, 10 });
    auto keySelectorKeyCodeLabel = Create<Label>(KeySelector::KeyCodeNameMap.at(simParams.smokeKeyCode));
    keySelectorKeyCodeLabel->SetBaseSize(80, 26);
    keySelectorKeyCodeLabel->SetPadding({ 5.0f, 0.0f, 0.0f, 0.0f });
    keySelectorKeyCodeLabel->SetVerticalTextAlignment(zcom::Alignment::CENTER);
    keySelectorKeyCodeLabel->SetZIndex(10);
    keySelectorKeyCodeLabel->SetInteractable(false);
    auto keySelectorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    keySelectorRow->FillContainerWidth();
    keySelectorRow->SetSpacing(10);
    auto keySelectorInput = Create<KeySelector>((BYTE)simParams.smokeKeyCode);
    keySelectorInput->SetBaseSize(80, 26);
    keySelectorInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    keySelectorInput->SetCornerRounding(2.0f);
    keySelectorInput->SubscribeOnKeySelected([=](BYTE value) {
        _scene->GetApp()->config.SetIntValue(SmokeSimConfig::SMOKE_KEY_CODE.name, value);
    }).Detach();
    keySelectorInput->SubscribeOnSelected([=, keySelectorKeyCodeLabel = keySelectorKeyCodeLabel.get()](Component*, bool) {
        keySelectorKeyCodeLabel->SetText(L"Press key");
        keySelectorKeyCodeLabel->SetFontStyle(DWRITE_FONT_STYLE_ITALIC);
    }).Detach();
    keySelectorInput->SubscribeOnDeselected([=, keySelectorKeyCodeLabel = keySelectorKeyCodeLabel.get()](Component* item) {
        keySelectorKeyCodeLabel->SetText(KeySelector::KeyCodeNameMap.at(((KeySelector*)item)->GetCurrentKey()));
        keySelectorKeyCodeLabel->SetFontStyle(DWRITE_FONT_STYLE_NORMAL);
    }).Detach();
    auto keySelectorLabel = Create<Label>(L"Smoke key");
    keySelectorLabel->SetBaseHeight(26);
    keySelectorLabel->SetVerticalTextAlignment(Alignment::CENTER);
    keySelectorLabel->SetProperty(FlexGrow());
    keySelectorRow->AddItem(std::move(keySelectorInput));
    keySelectorRow->AddItem(std::move(keySelectorLabel));
    keySelectorRowPanel->AddItem(std::move(keySelectorKeyCodeLabel));
    keySelectorRowPanel->AddItem(std::move(keySelectorRow));

    auto slowdownPersistenceDurationRow = Create<FlexPanel>(FlexDirection::RIGHT);
    slowdownPersistenceDurationRow->FillContainerWidth();
    slowdownPersistenceDurationRow->SetSpacing(10);
    slowdownPersistenceDurationRow->SetPadding({ 15, 0, 15, 10 });
    auto slowdownPersistenceDurationInput = Create<NumberInput>();
    slowdownPersistenceDurationInput->SetBaseSize(60, 26);
    slowdownPersistenceDurationInput->SetValue(NumberInputValue(simParams.slowdownPersistenceDurationMs));
    slowdownPersistenceDurationInput->SetMinValue(NumberInputValue(0));
    slowdownPersistenceDurationInput->SetMaxValue(NumberInputValue(5000));
    slowdownPersistenceDurationInput->SetStepSize(NumberInputValue(50));
    slowdownPersistenceDurationInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    slowdownPersistenceDurationInput->SetCornerRounding(2.0f);
    slowdownPersistenceDurationInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(SmokeSimConfig::SLOWDOWN_PERSISTENCE_DURATION.name, value.getAsInteger());
    }).Detach();
    auto slowdownPersistenceDurationLabel = Create<Label>(L"Slowdown persistence duration");
    slowdownPersistenceDurationLabel->SetBaseHeight(26);
    slowdownPersistenceDurationLabel->SetVerticalTextAlignment(Alignment::CENTER);
    slowdownPersistenceDurationLabel->SetProperty(FlexGrow());
    slowdownPersistenceDurationLabel->SetHoverText(L"How long after releasing the smoke key the simulation remains slowed down, measured in milliseconds. Small values make it harder to fully draw what you want without messing up the existing smoke");
    slowdownPersistenceDurationRow->AddItem(std::move(slowdownPersistenceDurationInput));
    slowdownPersistenceDurationRow->AddItem(std::move(slowdownPersistenceDurationLabel));

    if (simType == SmokeSimType::ENHANCED_SMOKE)
    {
        interactionPanel->AddItem(std::move(interactionLabel));
        interactionPanel->AddItem(std::move(keySelectorRowPanel));
        interactionPanel->AddItem(std::move(slowdownPersistenceDurationRow));
        flexPanel->AddItem(std::move(interactionPanel));
    }

    auto appearancePanel = Create<FlexPanel>(FlexDirection::DOWN);
    appearancePanel->FillContainerWidth();

    auto appearanceLabel = Create<Label>(L"Appearance");
    appearanceLabel->SetBaseHeight(50);
    appearanceLabel->SetParentWidthPercent(1.0f);
    appearanceLabel->SetVerticalTextAlignment(Alignment::CENTER);
    appearanceLabel->SetFontSize(20.0f);
    appearanceLabel->SetPadding({ 15.0f, 0.0f, 15.0f, 0.0f });

    auto trailColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailColorRow->FillContainerWidth();
    trailColorRow->SetSpacing(10);
    trailColorRow->SetPadding({ 15, 0, 15, 10 });
    auto trailColorInput = Create<Dummy>();
    trailColorInput->SetBaseSize(60, 26);
    trailColorInput->SetCornerRounding(2.0f);
    trailColorInput->SubscribeOnLeftReleased([=](Component*, int, int) {
        if (!_colorSelectorWindowId)
            _OpenColorSelector();
    }).Detach();
    auto trailColorLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail color" : L"Smoke color");
    trailColorLabel->SetBaseHeight(26);
    trailColorLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailColorLabel->SetProperty(FlexGrow());
    _colorInput = trailColorInput.get();
    trailColorRow->AddItem(std::move(trailColorInput));
    trailColorRow->AddItem(std::move(trailColorLabel));

    auto trailWidthRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailWidthRow->FillContainerWidth();
    trailWidthRow->SetSpacing(10);
    trailWidthRow->SetPadding({ 15, 0, 15, 10 });
    auto trailWidthInput = Create<NumberInput>();
    trailWidthInput->SetBaseSize(60, 26);
    trailWidthInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailWidth : simParams.brushWidth));
    trailWidthInput->SetMinValue(NumberInputValue(1));
    trailWidthInput->SetMaxValue(NumberInputValue(100));
    trailWidthInput->SetStepSize(NumberInputValue(1));
    trailWidthInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailWidthInput->SetCornerRounding(2.0f);
    trailWidthInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_WIDTH.name, value.getAsDouble());
        else
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::BRUSH_WIDTH.name, value.getAsDouble());
    }).Detach();
    auto trailWidthLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail width" : L"Brush width");
    trailWidthLabel->SetBaseHeight(26);
    trailWidthLabel->AutomaticWidth();
    trailWidthLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailWidthLabel->SetProperty(FlexGrow());
    trailWidthLabel->SetHoverText(L"How wide the smoke line is, in pixels");
    trailWidthRow->AddItem(std::move(trailWidthInput));
    trailWidthRow->AddItem(std::move(trailWidthLabel));

    auto trailEdgeFadeRangeRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailEdgeFadeRangeRow->FillContainerWidth();
    trailEdgeFadeRangeRow->SetSpacing(10);
    trailEdgeFadeRangeRow->SetPadding({ 15, 0, 15, 10 });
    auto trailEdgeFadeRangeInput = Create<NumberInput>();
    trailEdgeFadeRangeInput->SetBaseSize(60, 26);
    trailEdgeFadeRangeInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailEdgeFadeRange : simParams.brushEdgeFadeRange));
    trailEdgeFadeRangeInput->SetMinValue(NumberInputValue(1));
    trailEdgeFadeRangeInput->SetMaxValue(NumberInputValue(100));
    trailEdgeFadeRangeInput->SetStepSize(NumberInputValue(1));
    trailEdgeFadeRangeInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailEdgeFadeRangeInput->SetCornerRounding(2.0f);
    trailEdgeFadeRangeInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_EDGE_FADE_RANGE.name, value.getAsDouble());
        else
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::BRUSH_EDGE_FADE_RANGE.name, value.getAsDouble());
    }).Detach();
    auto trailEdgeFadeRangeLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail edge fade range" : L"Brush edge fade range");
    trailEdgeFadeRangeLabel->SetBaseHeight(26);
    trailEdgeFadeRangeLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailEdgeFadeRangeLabel->SetProperty(FlexGrow());
    trailEdgeFadeRangeLabel->SetHoverText(L"How wide the the fade out area of the smoke line is, in pixels");
    trailEdgeFadeRangeRow->AddItem(std::move(trailEdgeFadeRangeInput));
    trailEdgeFadeRangeRow->AddItem(std::move(trailEdgeFadeRangeLabel));

    auto trailDensityRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailDensityRow->FillContainerWidth();
    trailDensityRow->SetSpacing(10);
    trailDensityRow->SetPadding({ 15, 0, 15, 10 });
    auto trailDensityInput = Create<NumberInput>();
    trailDensityInput->SetBaseSize(60, 26);
    trailDensityInput->SetPrecision(1);
    trailDensityInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailDensity : simParams.smokeDensity));
    trailDensityInput->SetMinValue(NumberInputValue("0.1"));
    trailDensityInput->SetMaxValue(NumberInputValue("100"));
    trailDensityInput->SetStepSize(NumberInputValue("0.1"));
    trailDensityInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailDensityInput->SetCornerRounding(2.0f);
    trailDensityInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_DENSITY.name, value.getAsDouble());
        else
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::SMOKE_DENSITY.name, value.getAsDouble());
    }).Detach();
    auto trailDensityLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail density" : L"Smoke density");
    trailDensityLabel->SetBaseHeight(26);
    trailDensityLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailDensityLabel->SetProperty(FlexGrow());
    trailDensityLabel->SetHoverText(L"The density of the smoke line. Density of 1 means fully saturated, but higher densities allow smoke to spread out more while remaining the same color");
    trailDensityRow->AddItem(std::move(trailDensityInput));
    trailDensityRow->AddItem(std::move(trailDensityLabel));

    auto trailWindWidthRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailWindWidthRow->FillContainerWidth();
    trailWindWidthRow->SetSpacing(10);
    trailWindWidthRow->SetPadding({ 15, 0, 15, 10 });
    auto trailWindWidthInput = Create<NumberInput>();
    trailWindWidthInput->SetBaseSize(60, 26);
    trailWindWidthInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailWindWidth : simParams.cursorWindWidth));
    trailWindWidthInput->SetMinValue(NumberInputValue(1));
    trailWindWidthInput->SetMaxValue(NumberInputValue(100));
    trailWindWidthInput->SetStepSize(NumberInputValue(1));
    trailWindWidthInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailWindWidthInput->SetCornerRounding(2.0f);
    trailWindWidthInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_WIND_WIDTH.name, value.getAsDouble());
        else
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::CURSOR_WIND_WIDTH.name, value.getAsDouble());
    }).Detach();
    auto trailWindWidthLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail wind width" : L"Cursor wind width");
    trailWindWidthLabel->SetBaseHeight(26);
    trailWindWidthLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailWindWidthLabel->SetProperty(FlexGrow());
    trailWindWidthLabel->SetHoverText(L"The width of the area around your cursor that generates the wind. Values bigger than the smoke line width do nothing");
    trailWindWidthRow->AddItem(std::move(trailWindWidthInput));
    trailWindWidthRow->AddItem(std::move(trailWindWidthLabel));

    auto trailWindSpeedRow = Create<FlexPanel>(FlexDirection::RIGHT);
    trailWindSpeedRow->FillContainerWidth();
    trailWindSpeedRow->SetSpacing(10);
    trailWindSpeedRow->SetPadding({ 15, 0, 15, 10 });
    auto trailWindSpeedInput = Create<NumberInput>();
    trailWindSpeedInput->SetBaseSize(60, 26);
    trailWindSpeedInput->SetPrecision(2);
    trailWindSpeedInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailWindSpeed : simParams.cursorWindSpeed));
    trailWindSpeedInput->SetMinValue(NumberInputValue("0"));
    trailWindSpeedInput->SetMaxValue(NumberInputValue("10"));
    trailWindSpeedInput->SetStepSize(NumberInputValue("0.05"));
    trailWindSpeedInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    trailWindSpeedInput->SetCornerRounding(2.0f);
    trailWindSpeedInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_WIND_SPEED.name, value.getAsDouble());
        else
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::CURSOR_WIND_SPEED.name, value.getAsDouble());
    }).Detach();
    auto trailWindSpeedLabel = Create<Label>(simType == SmokeSimType::CURSOR_TRAIL ? L"Trail wind speed" : L"Cursor wind speed");
    trailWindSpeedLabel->SetBaseHeight(26);
    trailWindSpeedLabel->SetVerticalTextAlignment(Alignment::CENTER);
    trailWindSpeedLabel->SetProperty(FlexGrow());
    trailWindSpeedLabel->SetHoverText(L"Ratio between your cursor speed and generated wind speed. Large values can cause visual artifacts");
    trailWindSpeedRow->AddItem(std::move(trailWindSpeedInput));
    trailWindSpeedRow->AddItem(std::move(trailWindSpeedLabel));

    auto cursorTempRow = Create<FlexPanel>(FlexDirection::RIGHT);
    cursorTempRow->FillContainerWidth();
    cursorTempRow->SetSpacing(10);
    cursorTempRow->SetPadding({ 15, 0, 15, 10 });
    auto cursorTempInput = Create<NumberInput>();
    cursorTempInput->SetBaseSize(60, 26);
    cursorTempInput->SetPrecision(1);
    cursorTempInput->SetValue(NumberInputValue(simParams.cursorTemp));
    cursorTempInput->SetMinValue(NumberInputValue("0"));
    cursorTempInput->SetMaxValue(NumberInputValue("100"));
    cursorTempInput->SetStepSize(NumberInputValue("0.1"));
    cursorTempInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    cursorTempInput->SetCornerRounding(2.0f);
    cursorTempInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::CURSOR_TEMP.name, value.getAsDouble());
    }).Detach();
    auto cursorTempLabel = Create<Label>(L"Cursor temperature");
    cursorTempLabel->SetBaseHeight(26);
    cursorTempLabel->SetVerticalTextAlignment(Alignment::CENTER);
    cursorTempLabel->SetProperty(FlexGrow());
    cursorTempLabel->SetHoverText(L"How much heat your cursor generates. Heat makes surrounding smoke move upwards");
    cursorTempRow->AddItem(std::move(cursorTempInput));
    cursorTempRow->AddItem(std::move(cursorTempLabel));

    appearancePanel->AddItem(std::move(appearanceLabel));
    appearancePanel->AddItem(std::move(trailColorRow));
    appearancePanel->AddItem(std::move(trailWidthRow));
    appearancePanel->AddItem(std::move(trailEdgeFadeRangeRow));
    appearancePanel->AddItem(std::move(trailDensityRow));
    appearancePanel->AddItem(std::move(trailWindWidthRow));
    appearancePanel->AddItem(std::move(trailWindSpeedRow));
    if (simType == SmokeSimType::CURSOR_TRAIL)
        appearancePanel->AddItem(std::move(cursorTempRow));
    flexPanel->AddItem(std::move(appearancePanel));

    auto simulationPanel = Create<FlexPanel>(FlexDirection::DOWN);
    simulationPanel->FillContainerWidth();

    auto simulationLabel = Create<Label>(L"Simulation");
    simulationLabel->SetBaseHeight(50);
    simulationLabel->SetParentWidthPercent(1.0f);
    simulationLabel->SetVerticalTextAlignment(Alignment::CENTER);
    simulationLabel->SetFontSize(20.0f);
    simulationLabel->SetPadding({ 15.0f, 0.0f, 15.0f, 0.0f });

    auto velocityDiffusionRow = Create<FlexPanel>(FlexDirection::RIGHT);
    velocityDiffusionRow->FillContainerWidth();
    velocityDiffusionRow->SetSpacing(10);
    velocityDiffusionRow->SetPadding({ 15, 0, 15, 10 });
    auto velocityDiffusionInput = Create<NumberInput>();
    velocityDiffusionInput->SetBaseSize(60, 26);
    velocityDiffusionInput->SetPrecision(1);
    velocityDiffusionInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailVelocityDiffusion : simParams.smokeVelocityDiffusion));
    velocityDiffusionInput->SetMinValue(NumberInputValue("0"));
    velocityDiffusionInput->SetMaxValue(NumberInputValue("100"));
    velocityDiffusionInput->SetStepSize(NumberInputValue("0.1"));
    velocityDiffusionInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    velocityDiffusionInput->SetCornerRounding(2.0f);
    velocityDiffusionInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_VELOCITY_DIFFUSION.name, value.getAsDouble());
        else
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::SMOKE_VELOCITY_DIFFUSION.name, value.getAsDouble());
    }).Detach();
    auto velocityDiffusionLabel = Create<Label>(L"Velocity diffusion");
    velocityDiffusionLabel->SetBaseHeight(26);
    velocityDiffusionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    velocityDiffusionLabel->SetProperty(FlexGrow());
    velocityDiffusionLabel->SetHoverText(L"The rate at which wind spreads out to nearby cells");
    velocityDiffusionRow->AddItem(std::move(velocityDiffusionInput));
    velocityDiffusionRow->AddItem(std::move(velocityDiffusionLabel));

    auto densityDiffusionRow = Create<FlexPanel>(FlexDirection::RIGHT);
    densityDiffusionRow->FillContainerWidth();
    densityDiffusionRow->SetSpacing(10);
    densityDiffusionRow->SetPadding({ 15, 0, 15, 10 });
    auto densityDiffusionInput = Create<NumberInput>();
    densityDiffusionInput->SetBaseSize(60, 26);
    densityDiffusionInput->SetPrecision(1);
    densityDiffusionInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailDensityDiffusion : simParams.smokeDensityDiffusion));
    densityDiffusionInput->SetMinValue(NumberInputValue("0"));
    densityDiffusionInput->SetMaxValue(NumberInputValue("100"));
    densityDiffusionInput->SetStepSize(NumberInputValue("0.1"));
    densityDiffusionInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    densityDiffusionInput->SetCornerRounding(2.0f);
    densityDiffusionInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_DENSITY_DIFFUSION.name, value.getAsDouble());
        else
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::SMOKE_DENSITY_DIFFUSION.name, value.getAsDouble());
    }).Detach();
    auto densityDiffusionLabel = Create<Label>(L"Density diffusion");
    densityDiffusionLabel->SetBaseHeight(26);
    densityDiffusionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    densityDiffusionLabel->SetProperty(FlexGrow());
    densityDiffusionLabel->SetHoverText(L"The rate at which smoke spreads out to nearby cells");
    densityDiffusionRow->AddItem(std::move(densityDiffusionInput));
    densityDiffusionRow->AddItem(std::move(densityDiffusionLabel));

    auto temperatureDiffusionRow = Create<FlexPanel>(FlexDirection::RIGHT);
    temperatureDiffusionRow->FillContainerWidth();
    temperatureDiffusionRow->SetSpacing(10);
    temperatureDiffusionRow->SetPadding({ 15, 0, 15, 10 });
    auto temperatureDiffusionInput = Create<NumberInput>();
    temperatureDiffusionInput->SetBaseSize(60, 26);
    temperatureDiffusionInput->SetPrecision(1);
    temperatureDiffusionInput->SetValue(NumberInputValue(simParams.trailTemperatureDiffusion));
    temperatureDiffusionInput->SetMinValue(NumberInputValue("0"));
    temperatureDiffusionInput->SetMaxValue(NumberInputValue("100"));
    temperatureDiffusionInput->SetStepSize(NumberInputValue("0.1"));
    temperatureDiffusionInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    temperatureDiffusionInput->SetCornerRounding(2.0f);
    temperatureDiffusionInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_TEMPERATURE_DIFFUSION.name, value.getAsDouble());
    }).Detach();
    auto temperatureDiffusionLabel = Create<Label>(L"Temperature diffusion");
    temperatureDiffusionLabel->SetBaseHeight(26);
    temperatureDiffusionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    temperatureDiffusionLabel->SetProperty(FlexGrow());
    temperatureDiffusionLabel->SetHoverText(L"The rate at which heat spreads out to nearby cells");
    temperatureDiffusionRow->AddItem(std::move(temperatureDiffusionInput));
    temperatureDiffusionRow->AddItem(std::move(temperatureDiffusionLabel));

    auto densityReductionRateRow = Create<FlexPanel>(FlexDirection::RIGHT);
    densityReductionRateRow->FillContainerWidth();
    densityReductionRateRow->SetSpacing(10);
    densityReductionRateRow->SetPadding({ 15, 0, 15, 10 });
    auto densityReductionRateInput = Create<NumberInput>();
    densityReductionRateInput->SetBaseSize(60, 26);
    densityReductionRateInput->SetPrecision(2);
    densityReductionRateInput->SetValue(NumberInputValue(simType == SmokeSimType::CURSOR_TRAIL ? simParams.trailDensityReductionRate : simParams.smokeDensityReductionRate));
    densityReductionRateInput->SetMinValue(NumberInputValue("0"));
    densityReductionRateInput->SetMaxValue(NumberInputValue("100"));
    densityReductionRateInput->SetStepSize(NumberInputValue("0.05"));
    densityReductionRateInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    densityReductionRateInput->SetCornerRounding(2.0f);
    densityReductionRateInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        if (simType == SmokeSimType::CURSOR_TRAIL)
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_DENSITY_REDUCTION_RATE.name, value.getAsDouble());
        else
            _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::SMOKE_DENSITY_REDUCTION_RATE.name, value.getAsDouble());
    }).Detach();
    auto densityReductionRateLabel = Create<Label>(L"Density reduction rate");
    densityReductionRateLabel->SetBaseHeight(26);
    densityReductionRateLabel->SetVerticalTextAlignment(Alignment::CENTER);
    densityReductionRateLabel->SetProperty(FlexGrow());
    densityReductionRateLabel->SetHoverText(L"How much smoke density is reduced in every cell every second. Controls how quickly the smoke disappears");
    densityReductionRateRow->AddItem(std::move(densityReductionRateInput));
    densityReductionRateRow->AddItem(std::move(densityReductionRateLabel));

    auto temperatureReductionRateRow = Create<FlexPanel>(FlexDirection::RIGHT);
    temperatureReductionRateRow->FillContainerWidth();
    temperatureReductionRateRow->SetSpacing(10);
    temperatureReductionRateRow->SetPadding({ 15, 0, 15, 10 });
    auto temperatureReductionRateInput = Create<NumberInput>();
    temperatureReductionRateInput->SetBaseSize(60, 26);
    temperatureReductionRateInput->SetPrecision(2);
    temperatureReductionRateInput->SetValue(NumberInputValue(simParams.trailTemperatureReductionRate));
    temperatureReductionRateInput->SetMinValue(NumberInputValue("0"));
    temperatureReductionRateInput->SetMaxValue(NumberInputValue("100"));
    temperatureReductionRateInput->SetStepSize(NumberInputValue("0.05"));
    temperatureReductionRateInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    temperatureReductionRateInput->SetCornerRounding(2.0f);
    temperatureReductionRateInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(SmokeSimConfig::TRAIL_TEMPERATURE_REDUCTION_RATE.name, value.getAsDouble());
    }).Detach();
    auto temperatureReductionRateLabel = Create<Label>(L"Temperature reduction rate");
    temperatureReductionRateLabel->SetBaseHeight(26);
    temperatureReductionRateLabel->SetVerticalTextAlignment(Alignment::CENTER);
    temperatureReductionRateLabel->SetProperty(FlexGrow());
    temperatureReductionRateLabel->SetHoverText(L"How much temperature is reduced in every cell every second. Controls how quickly the smoke stops rising");
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
    _UpdateColorInput();
}

void zcom::SmokeSimParameterPanel::_UpdateColorInput()
{
    std::optional<int64_t> value = _scene->GetApp()->config.GetIntValue(_simType == SmokeSimType::CURSOR_TRAIL ? SmokeSimConfig::TRAIL_COLOR.name : SmokeSimConfig::SMOKE_COLOR.name);
    zutil::Color color = zutil::Color(value ? (int)value.value() : 0xFF000000);
    _colorInput->SetBackgroundColor(D2D1::ColorF(color.ToIntNoAlpha(), color.a / 255.0f));
}

void zcom::SmokeSimParameterPanel::_OpenOverlayWindow()
{
    std::wstring wndClass = _simType == SmokeSimType::CURSOR_TRAIL ? SmokeSimConfig::CURSOR_TRAIL_OVERLAY_WINDOW_NAME : SmokeSimConfig::ENHANCED_SMOKE_OVERLAY_WINDOW_NAME;
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
        width = (int)_widthInput->GetValue().getAsInteger();
        height = (int)_heightInput->GetValue().getAsInteger();
        xOffset = (int)_xOffsetInput->GetValue().getAsInteger();
        yOffset = (int)_yOffsetInput->GetValue().getAsInteger();
    }

    zwnd::WindowProperties props = zwnd::WindowProperties()
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

    _overlayWindowId = _scene->GetApp()->CreateTopWindow(
        props,
        [=](zwnd::Window* wnd) {
            // Remove window decorations
            DefaultNonClientAreaSceneOptions ncOpt;
            ncOpt.drawWindowShadow = false;
            ncOpt.drawWindowBorder = false;
            ncOpt.resizingBorderWidths = { 0, 0, 0, 0 };
            ncOpt.clientAreaMargins = { 0, 0, 0, 0 };
            wnd->LoadNonClientAreaScene<DefaultNonClientAreaScene>(&ncOpt);

            SmokeSimSceneOptions opt;
            opt.simType = _simType;
            opt.cellSize = (int)_cellSizeInput->GetValue().getAsInteger();
            opt.maxThreads = (int)_threadCountInput->GetValue().getAsInteger();
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

void zcom::SmokeSimParameterPanel::_OpenColorSelector()
{
    std::wstring wndClass = _simType == SmokeSimType::CURSOR_TRAIL ? SmokeSimConfig::CURSOR_TRAIL_COLOR_SELECTOR_WINDOW_NAME : SmokeSimConfig::ENHANCED_SMOKE_COLOR_SELECTOR_WINDOW_NAME;
    std::wstring windowTitle = _simType == SmokeSimType::CURSOR_TRAIL ? L"Trail color" : L"Smoke color";
    ConfigValue<int> configValue = _simType == SmokeSimType::CURSOR_TRAIL ? SmokeSimConfig::TRAIL_COLOR : SmokeSimConfig::SMOKE_COLOR;
    int width = 300;
    int height = 214;
    RECT mainWindowRect = _scene->GetWindow()->Backend().GetWindowRectangle();
    int mainWindowCenterX = (mainWindowRect.left + mainWindowRect.right) / 2;
    int mainWindowCenterY = (mainWindowRect.top + mainWindowRect.bottom) / 2;

    zwnd::WindowProperties props = zwnd::WindowProperties()
        .WindowClassName(wndClass)
        .InitialSize(width, height)
        .InitialOffset(mainWindowCenterX - width / 2, mainWindowCenterY - height / 2)
        .FixedSize()
        //.DisableWindowActivation()
        .DisableMaximizing()
        .DisableMinimizing()
        .DisableFastTooltips();

    _colorSelectorWindowId = _scene->GetApp()->CreateChildWindow(
        _scene->GetWindow()->GetWindowId(),
        props,
        [=](zwnd::Window* wnd) {
            wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
            wnd->resourceManager.InitAllImages();

            // Remove resizing border
            DefaultNonClientAreaSceneOptions ncOpt;
            ncOpt.resizingBorderWidths = { 0, 0, 0, 0 };
            wnd->LoadNonClientAreaScene<DefaultNonClientAreaScene>(&ncOpt);

            // Remove unnecessary caption elements
            DefaultTitleBarSceneOptions tbOpt;
            tbOpt.showMaximizeButton = false;
            tbOpt.showMinimizeButton = false;
            tbOpt.showIcon = false;
            tbOpt.windowTitle = windowTitle;
            tbOpt.useCleartype = false;
            wnd->LoadTitleBarScene<DefaultTitleBarScene>(&tbOpt);

            ColorSelectorSceneOptions opt;
            opt.configValue = configValue;
            wnd->LoadStartingScene<ColorSelectorScene>(&opt);
        }
    );
}