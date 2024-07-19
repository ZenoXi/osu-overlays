#include "App.h"
#include "Scenes/Scene.h"
#include "Scenes/DefaultNonClientAreaScene.h"
#include "Scenes/DefaultTitleBarScene.h"
#include "Scenes/Scene.h"
#include "CursorTrailParameterPanel.h"
#include "CursorTrailConfig.h"
#include "CursorTrailScene.h"
#include "Helper/StringHelper.h"
#include "Shared/Styles/Styles.h"
#include "Shared/Scenes/ColorSelectorScene.h"
#include "Shared/Components/NumberParameterInput.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Util/Streams.h"

void zcom::CursorTrailParameterPanel::Init()
{
    ScrollPanel::Init();

    Handle<zwnd::Window> windowHandle = _scene->GetApp()->FindWindowByClassName(CursorTrailConfig::OVERLAY_WINDOW_NAME);
    if (windowHandle.Valid())
        _overlayWindowId = windowHandle->GetWindowId();
    Handle<zwnd::Window> headColorSelectorWindowHandle = _scene->GetApp()->FindWindowByClassName(CursorTrailConfig::HEAD_COLOR_SELECTOR_WINDOW_NAME);
    if (headColorSelectorWindowHandle.Valid())
        _headColorSelectorWindowId = headColorSelectorWindowHandle->GetWindowId();

    SetBackgroundColor(D2D1::ColorF(0x202020));
    Scrollable(Scrollbar::VERTICAL, true);
    ScrollBackgroundVisible(Scrollbar::VERTICAL, true);

    _CreateInsertionMarker();

    auto flexPanel = Create<FlexPanel>(FlexDirection::DOWN);
    flexPanel->FillContainerWidth();
    flexPanel->AddItem(_SetUpGeneralPanel());
    flexPanel->AddItem(_SetUpAppearancePanel());
    flexPanel->AddItem(_SetUpColorsPanel());
    flexPanel->SubscribePostMouseMove([=](Component* panel, std::vector<EventTargets::Params> targets, int x, int y, int, int) {
        for (auto& target : targets)
        {
            if (target.target->HasTag("color_selector"))
            {
                for (int i = 0; i < _currentPalette.size(); i++)
                {
                    if (_currentPalette[i].selectorItem.get() == target.target)
                    {
                        _hoveredColorSelectorIndex = i;
                        break;
                    }
                }

                int panelWindowY = panel->GetWindowY();
                int itemWindowY = target.target->GetWindowY();
                if (target.y < target.target->GetHeight() / 2)
                {
                    _hoveredColorSelectorIsTopHalf = true;
                    _insertionMarker->SetVerticalOffsetPixels(itemWindowY - panelWindowY - _insertionMarker->GetHeight() / 2);
                }
                else
                {
                    _hoveredColorSelectorIsTopHalf = false;
                    _insertionMarker->SetVerticalOffsetPixels(itemWindowY - panelWindowY - _insertionMarker->GetHeight() / 2 + target.target->GetHeight());
                }
                _insertionMarker->SetVisible(true);
                return;
            }
            else if (target.target == _insertionMarker.get())
            {
                return;
            }
        }
        _insertionMarker->SetVisible(false);
    }).Detach();
    // OnLeave handler set on outer-most component, since if it is set on flexPanel, every time the insertionMarker is shown,
    // the mouse enters the marker and MouseLeave on the flexPanel is invoked (since insertionMarker is not a child of flexPanel)
    // resulting in rapid toggling between visible and hidden
    SubscribeOnMouseLeave([=](Component*) {
        _insertionMarker->SetVisible(false);
    }).Detach();
    AddItem(std::move(flexPanel));

    _LoadPalette();
    _BuildItemsForPalette();
    _OnGradientStopSelected(0);
    _ReorderColorList();

    _OnColorChanged();
    _UpdateActiveItems();
    _UpdateColorInput();
    _UpdateButtonsBasedOnOverlayState();

    _configChangedEventSubscription = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        if (changes)
        {
            if (changes.value().first == CursorTrailConfig::HEAD_COLOR.name)
                ExecuteSynchronously([=]() { _UpdateColorInput(); });
        }
    });

    SubscribePostUpdate([=]() {
        if (!_configSaved && ztime::Main() > _lastSaveTime + Duration(250, MILLISECONDS))
        {
            _configSaved = true;
            _scene->GetApp()->config.SaveConfig();
        }
    }).Detach();
}

void zcom::CursorTrailParameterPanel::_CreateInsertionMarker()
{
    _insertionMarker = Create<FlexPanel>(FlexDirection::RIGHT);
    _insertionMarker->FillContainerWidth();
    _insertionMarker->SetBaseSize(-10, 20);
    _insertionMarker->SetHorizontalAlignment(Alignment::CENTER);
    _insertionMarker->SetPadding({ 0, 0, 11, 0 });
    _insertionMarker->SetItemAlignment(Alignment::CENTER);
    _insertionMarker->SetVerticalOffsetPixels(636);
    _insertionMarker->SetZIndex(1);
    _insertionMarker->SetProperty(PROP_Shadow().WithBlurStandardDeviation(1.5f));
    _insertionMarker->EnableMouseEventFallthrough();
    auto insertionMarkerHead = Create<Button>();
    RoundedLiftedButtonStyle::Apply(insertionMarkerHead.get());
    insertionMarkerHead->SetBaseSize(20, 20);
    insertionMarkerHead->SetVerticalAlignment(Alignment::CENTER);
    insertionMarkerHead->SetActivation(ButtonActivation::PRESS);
    insertionMarkerHead->SetButtonColor(D2D1::ColorF(0x383838));
    insertionMarkerHead->SetButtonHoverColor(D2D1::ColorF(0x484848));
    insertionMarkerHead->SetButtonClickColor(D2D1::ColorF(0x303030));
    insertionMarkerHead->SetButtonImageAll(_scene->GetWindow()->resourceManager.GetImage("insertion_marker_icon"));
    insertionMarkerHead->ButtonImage()->SetPlacement(ImagePlacement::CENTER);
    insertionMarkerHead->UseImageParamsForAll(insertionMarkerHead->ButtonImage());
    insertionMarkerHead->ButtonImage()->SetTintColor(D2D1::ColorF(0xA0A0A0));
    insertionMarkerHead->ButtonHoverImage()->SetTintColor(D2D1::ColorF(0xD0D0D0));
    insertionMarkerHead->ButtonClickImage()->SetTintColor(D2D1::ColorF(0xD0D0D0));
    auto insertionMarkerTransition = Create<Image>(_scene->GetWindow()->resourceManager.GetImage("wall_to_2px_pipe_transition_right"));
    insertionMarkerTransition->SetBaseSize(2, 8);
    insertionMarkerTransition->SetTintColor(D2D1::ColorF(0x383838));
    auto insertionMarkerSeparator = Create<Dummy>();
    insertionMarkerSeparator->SetBaseHeight(2);
    insertionMarkerSeparator->SetProperty(FlexGrow());
    insertionMarkerSeparator->SetBackgroundColor(D2D1::ColorF(0x383838));

    insertionMarkerHead->SubscribeOnMouseEnterArea([=, transition = insertionMarkerTransition.get(), separator = insertionMarkerSeparator.get()](Component* item) {
        _SetInsertionMarkerColors((Button*)item, transition, separator);
    }).Detach();
    insertionMarkerHead->SubscribeOnMouseLeaveArea([=, transition = insertionMarkerTransition.get(), separator = insertionMarkerSeparator.get()](Component* item) {
        _SetInsertionMarkerColors((Button*)item, transition, separator);
    }).Detach();
    insertionMarkerHead->SubscribeOnLeftPressed([=, transition = insertionMarkerTransition.get(), separator = insertionMarkerSeparator.get()](Component* item, int, int) {
        _SetInsertionMarkerColors((Button*)item, transition, separator);
    }).Detach();
    insertionMarkerHead->SubscribeOnLeftReleased([=, transition = insertionMarkerTransition.get(), separator = insertionMarkerSeparator.get()](Component* item, int, int) {
        _SetInsertionMarkerColors((Button*)item, transition, separator);
    }).Detach();
    insertionMarkerHead->SubscribeOnActivated([=]() {
        if (_hoveredColorSelectorIndex == 0 && _hoveredColorSelectorIsTopHalf)
        {
            _GradientStop& hoveredStop = _currentPalette[_hoveredColorSelectorIndex];
            _GradientStop newStop = { hoveredStop.color, hoveredStop.position / 2 };
            _CreateInnerGradientStopComponents(newStop);
            _currentPalette.insert(_currentPalette.begin(), std::move(newStop));
            _OnGradientStopSelected(0);
        }
        else if (_hoveredColorSelectorIndex == _currentPalette.size() - 1 && !_hoveredColorSelectorIsTopHalf)
        {
            _GradientStop& hoveredStop = _currentPalette[_hoveredColorSelectorIndex];
            _GradientStop newStop = { hoveredStop.color, (hoveredStop.position + 1.0f) / 2 };
            _CreateInnerGradientStopComponents(newStop);
            _currentPalette.push_back(std::move(newStop));
            _OnGradientStopSelected((int)_currentPalette.size() - 1);
        }
        else
        {
            _GradientStop& hoveredStop = _currentPalette[_hoveredColorSelectorIndex];
            if (_hoveredColorSelectorIsTopHalf)
            {
                _GradientStop& topStop = _currentPalette[(size_t)_hoveredColorSelectorIndex - 1];
                _GradientStop newStop = {
                    zutil::Color(
                        uint8_t(((uint32_t)topStop.color.r + hoveredStop.color.r) / 2),
                        uint8_t(((uint32_t)topStop.color.g + hoveredStop.color.g) / 2),
                        uint8_t(((uint32_t)topStop.color.b + hoveredStop.color.b) / 2),
                        uint8_t(((uint32_t)topStop.color.a + hoveredStop.color.a) / 2)
                    ),
                    (topStop.position + hoveredStop.position) / 2
                };
                _CreateInnerGradientStopComponents(newStop);
                _currentPalette.insert(_currentPalette.begin() + _hoveredColorSelectorIndex, std::move(newStop));
                _OnGradientStopSelected(_hoveredColorSelectorIndex);
            }
            else
            {
                _GradientStop& bottomStop = _currentPalette[(size_t)_hoveredColorSelectorIndex + 1];
                _GradientStop newStop = {
                    zutil::Color(
                        uint8_t(((uint32_t)bottomStop.color.r + hoveredStop.color.r) / 2),
                        uint8_t(((uint32_t)bottomStop.color.g + hoveredStop.color.g) / 2),
                        uint8_t(((uint32_t)bottomStop.color.b + hoveredStop.color.b) / 2),
                        uint8_t(((uint32_t)bottomStop.color.a + hoveredStop.color.a) / 2)
                    ),
                    (bottomStop.position + hoveredStop.position) / 2
                };
                _CreateInnerGradientStopComponents(newStop);
                _currentPalette.insert(_currentPalette.begin() + _hoveredColorSelectorIndex + 1, std::move(newStop));
                _OnGradientStopSelected(_hoveredColorSelectorIndex + 1);
            }
        }
        _SavePalette();
        _ReorderColorList();
    }).Detach();

    _insertionMarker->AddItem(std::move(insertionMarkerHead));
    _insertionMarker->AddItem(std::move(insertionMarkerTransition));
    _insertionMarker->AddItem(std::move(insertionMarkerSeparator));
    AddItem(_insertionMarker.get());
}

void zcom::CursorTrailParameterPanel::_SetInsertionMarkerColors(Button* head, Image* transition, Dummy* separator)
{
    if (head->GetMouseInsideArea())
    {
        _insertionMarker->SetProperty(PROP_Shadow().WithBlurStandardDeviation(3.0f));
        if (head->GetMouseLeftClicked())
        {
            transition->SetTintColor(D2D1::ColorF(0x303030));
            separator->SetBackgroundColor(D2D1::ColorF(0x303030));
        }
        else
        {
            transition->SetTintColor(D2D1::ColorF(0x484848));
            separator->SetBackgroundColor(D2D1::ColorF(0x484848));
        }
    }
    else
    {
        _insertionMarker->SetProperty(PROP_Shadow().WithBlurStandardDeviation(1.5f));
        transition->SetTintColor(D2D1::ColorF(0x383838));
        separator->SetBackgroundColor(D2D1::ColorF(0x383838));
    }
}

std::unique_ptr<zcom::FlexPanel> zcom::CursorTrailParameterPanel::_SetUpGeneralPanel()
{
    auto generalPanel = Create<FlexPanel>(FlexDirection::DOWN);
    generalPanel->FillContainerWidth();

    auto titleRow = Create<FlexPanel>(FlexDirection::RIGHT);
    titleRow->FillContainerWidth();
    titleRow->SetPadding({ 15, 15, 15, 15 });
    auto generalLabel = Create<Label>(L"Cursor trail");
    generalLabel->SetBaseHeight(30);
    generalLabel->SetVerticalTextAlignment(Alignment::CENTER);
    generalLabel->SetFontSize(20.0f);
    generalLabel->SetProperty(FlexGrow());
    _enableButton = Create<Button>(L"Enable");
    _enableButton->SetBaseSize(90, 30);
    EnableButtonStyle::Apply(_enableButton.get());
    _enableButton->SubscribeOnActivated([=]() {
        if (!_overlayWindowId)
            _OpenOverlayWindow();
        else
            _CloseOverlayWindow();
        _UpdateActiveItems();
        _UpdateButtonsBasedOnOverlayState();
    }).Detach();
    titleRow->AddItem(std::move(generalLabel));
    titleRow->AddItem(_enableButton.get());
    
    auto fullMonitorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    fullMonitorRow->FillContainerWidth();
    fullMonitorRow->SetSpacing(10);
    fullMonitorRow->SetPadding({ 15, 0, 15, 10 });
    _fullMonitorCheckbox = Create<Checkbox>();
    _fullMonitorCheckbox->SetBaseSize(20, 20);
    _fullMonitorCheckbox->SetBackgroundColor(D2D1::ColorF(0x101010));
    _fullMonitorCheckbox->SetCornerRounding(2.0f);
    _fullMonitorCheckbox->SetVerticalAlignment(Alignment::CENTER);
    _fullMonitorCheckbox->Checked(_scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::FULL_MONITOR, Config::ADD_IF_MISSING));
    _fullMonitorCheckbox->SubscribeOnStateChanged([=](bool state) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::FULL_MONITOR.name, state);
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
    layoutSection->SetPadding({ 15, 0, 15, 15 });
    {
        auto sizeCol = Create<FlexPanel>(FlexDirection::DOWN);
        sizeCol->SetWidthFixed(true);
        sizeCol->SetProperty(FlexGrow());
        sizeCol->SetSpacing(10);
        {
            auto widthInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
                L"Width", L"Initial width of the overlay, in pixels",
                _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::INITIAL_WIDTH, Config::ADD_IF_MISSING),
                300, 10000, 10
            ));
            widthInput->SetPadding({ 0, 0, 0, 0 });
            widthInput->GetInput()->SetBaseSize(70, 26);
            widthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(CursorTrailConfig::INITIAL_WIDTH.name, value.getAsInteger());
            }).Detach();
            _widthInput = widthInput->GetInput();
            sizeCol->AddItem(std::move(widthInput));
        } {
            auto heightInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
                L"Height", L"Initial height of the overlay, in pixels",
                _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::INITIAL_HEIGHT, Config::ADD_IF_MISSING),
                300, 10000, 10
            ));
            heightInput->SetPadding({ 0, 0, 0, 0 });
            heightInput->GetInput()->SetBaseSize(70, 26);
            heightInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(CursorTrailConfig::INITIAL_HEIGHT.name, value.getAsInteger());
            }).Detach();
            _heightInput = heightInput->GetInput();
            sizeCol->AddItem(std::move(heightInput));
        }
        layoutSection->AddItem(std::move(sizeCol));
    } {
        auto offsetCol = Create<FlexPanel>(FlexDirection::DOWN);
        offsetCol->SetWidthFixed(true);
        offsetCol->SetProperty(FlexGrow());
        offsetCol->SetSpacing(10);
        {
            auto xOffsetInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
                L"X offset", L"Initial horizontal shift of the overlay, in pixels",
                _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::INITIAL_X_OFFSET, Config::ADD_IF_MISSING),
                -10000, 10000, 1
            ));
            xOffsetInput->SetPadding({ 0, 0, 0, 0 });
            xOffsetInput->GetInput()->SetBaseSize(70, 26);
            xOffsetInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(CursorTrailConfig::INITIAL_X_OFFSET.name, value.getAsInteger());
            }).Detach();
            _xOffsetInput = xOffsetInput->GetInput();
            offsetCol->AddItem(std::move(xOffsetInput));
        } {
            auto yOffsetInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
                L"Y offset", L"Initial vertical shift of the overlay, in pixels",
                _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::INITIAL_Y_OFFSET, Config::ADD_IF_MISSING),
                -10000, 10000, 1
            ));
            yOffsetInput->SetPadding({ 0, 0, 0, 0 });
            yOffsetInput->GetInput()->SetBaseSize(70, 26);
            yOffsetInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(CursorTrailConfig::INITIAL_Y_OFFSET.name, value.getAsInteger());
            }).Detach();
            _yOffsetInput = yOffsetInput->GetInput();
            offsetCol->AddItem(std::move(yOffsetInput));
        }
        layoutSection->AddItem(std::move(offsetCol));
    }

    generalPanel->AddItem(std::move(titleRow));
    generalPanel->AddItem(std::move(fullMonitorRow));
    generalPanel->AddItem(std::move(layoutSection));
    return generalPanel;
}

std::unique_ptr<zcom::FlexPanel> zcom::CursorTrailParameterPanel::_SetUpAppearancePanel()
{
    auto appearancePanel = Create<FlexPanel>(FlexDirection::DOWN);
    appearancePanel->FillContainerWidth();

    auto trailWidthInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail width", L"How thick the trail is, in pixels",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_WIDTH, Config::ADD_IF_MISSING),
        1, 1000, 1
    ));
    trailWidthInput->GetInput()->SetBaseWidth(70);
    trailWidthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::TRAIL_WIDTH.name, value.getAsInteger());
    }).Detach();
    
    auto headSizeInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail head size", L"How big the head of the trail is, in pixels",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::HEAD_SIZE, Config::ADD_IF_MISSING),
        1, 1000, 1
    ));
    headSizeInput->GetInput()->SetBaseWidth(70);
    headSizeInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::HEAD_SIZE.name, value.getAsInteger());
    }).Detach();
    
    auto trailEdgeWidthInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Trail edge width", L"How wide the fade out area on the edge of the trail is, in pixels",
        _scene->GetApp()->config.GetDoubleConfigValue(CursorTrailConfig::TRAIL_EDGE_WIDTH, Config::ADD_IF_MISSING),
        0.1f, 1000.0f, 0.1f
    ));
    trailEdgeWidthInput->GetInput()->SetBaseWidth(70);
    trailEdgeWidthInput->GetInput()->SetPrecision(1);
    trailEdgeWidthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(CursorTrailConfig::TRAIL_EDGE_WIDTH.name, value.getAsDouble());
    }).Detach();
    
    auto headEdgeWidthInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Head edge width", L"How wide the fade out area on the edge of the trail head is, in pixels",
        _scene->GetApp()->config.GetDoubleConfigValue(CursorTrailConfig::HEAD_EDGE_WIDTH, Config::ADD_IF_MISSING),
        0.1f, 1000.0f, 0.1f
    ));
    headEdgeWidthInput->GetInput()->SetBaseWidth(70);
    headEdgeWidthInput->GetInput()->SetPrecision(1);
    headEdgeWidthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(CursorTrailConfig::HEAD_EDGE_WIDTH.name, value.getAsDouble());
    }).Detach();
    
    auto trailLifetimeInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail lifetime", L"How is the trail visible for, in milliseconds",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_LIFETIME, Config::ADD_IF_MISSING),
        1, 10000, 50
    ));
    trailLifetimeInput->GetInput()->SetBaseWidth(70);
    trailLifetimeInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::TRAIL_LIFETIME.name, value.getAsInteger());
    }).Detach();
    
    auto colorCycleDurationInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Color cycle duration", L"How long it takes to cycle through all of the colors, in milliseconds",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::COLOR_CYCLE_DURATION, Config::ADD_IF_MISSING),
        1, 100000, 500
    ));
    colorCycleDurationInput->GetInput()->SetBaseWidth(70);
    colorCycleDurationInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::COLOR_CYCLE_DURATION.name, value.getAsInteger());
    }).Detach();
    
    auto iconSpinDurationInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Icon spin duration", L"How long it takes for the cursor icon to make a full rotation, in milliseconds",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::ICON_SPIN_DURATION, Config::ADD_IF_MISSING),
        1, 100000, 500
    ));
    iconSpinDurationInput->GetInput()->SetBaseWidth(70);
    iconSpinDurationInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::ICON_SPIN_DURATION.name, value.getAsInteger());
    }).Detach();

    auto trailResolutionInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail resolution", L"How many points to calculate between each cursor position. Higher values make the trail smoother. Might impact performance",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_RESOLUTION, Config::ADD_IF_MISSING),
        1, 100, 1
    ));
    trailResolutionInput->GetInput()->SetBaseWidth(70);
    trailResolutionInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::TRAIL_RESOLUTION.name, value.getAsInteger());
    }).Detach();

    appearancePanel->AddItem(std::move(Create<SectionHeader>(L"Appearance")));
    appearancePanel->AddItem(std::move(trailWidthInput));
    appearancePanel->AddItem(std::move(headSizeInput));
    appearancePanel->AddItem(std::move(trailEdgeWidthInput));
    appearancePanel->AddItem(std::move(headEdgeWidthInput));
    appearancePanel->AddItem(std::move(trailLifetimeInput));
    appearancePanel->AddItem(std::move(colorCycleDurationInput));
    appearancePanel->AddItem(std::move(iconSpinDurationInput));
    appearancePanel->AddItem(std::move(trailResolutionInput));
    return appearancePanel;
}

std::unique_ptr<zcom::FlexPanel> zcom::CursorTrailParameterPanel::_SetUpColorsPanel()
{
    auto colorsPanel = Create<FlexPanel>(FlexDirection::DOWN);
    colorsPanel->FillContainerWidth();
    colorsPanel->SetPadding({ 0, 0, 0, 15 });

    auto headColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    headColorRow->FillContainerWidth();
    headColorRow->SetSpacing(10);
    headColorRow->SetPadding({ 15, 0, 15, 10 });
    _headColorInput = Create<Dummy>();
    _headColorInput->SetBaseSize(60, 26);
    _headColorInput->SetCornerRounding(2.0f);
    _headColorInput->SubscribeOnLeftReleased([=](Component*, int, int) {
        if (!_headColorSelectorWindowId)
        {
            _headColorSelectorWindowId = _OpenColorSelector(L"Bar color", CursorTrailConfig::HEAD_COLOR_SELECTOR_WINDOW_NAME, CursorTrailConfig::HEAD_COLOR);
        }
        else
        {
            Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_headColorSelectorWindowId.value());
            if (handle.Valid())
                handle->Backend().Focus();
        }
    }).Detach();
    auto headColorLabel = Create<Label>(L"Trail head color");
    headColorLabel->SetBaseHeight(26);
    headColorLabel->SetVerticalTextAlignment(Alignment::CENTER);
    headColorLabel->SetProperty(FlexGrow());
    headColorRow->AddItem(_headColorInput.get());
    headColorRow->AddItem(std::move(headColorLabel));

    auto trailColorsLabel = Create<Label>(L"Trail customization");
    trailColorsLabel->SetBaseSize(160, 20);
    trailColorsLabel->SetHorizontalOffsetPixels(15);
    auto trailColorsSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(trailColorsSeparator.get(), 15);
    trailColorsSeparator->SetBackgroundColor(D2D1::ColorF(0x383838));
    trailColorsSeparator->SetProperty(FlexMarginBefore(-4));
    trailColorsSeparator->SetProperty(FlexMarginAfter(4));

    _paletteSlider = Create<Slider>();
    _paletteSlider->SetParentWidthPercent(1.0f);
    _paletteSlider->SetBaseHeight(30);
    _paletteSlider->SetSliderBodyOffset(15, 15);
    _paletteSlider->SetInteractionAreaMargins({ 10, 4, 10, 4 });
    _paletteSlider->SetAnchorOffset(-2);
    _paletteSlider->SetProperty(PROP_Shadow());
    _paletteSlider->SetEatScrollEvents(true);

    auto paletteSliderBody = Create<Dummy>();
    paletteSliderBody->SetParentWidthPercent(1.0f);
    paletteSliderBody->SetBaseSize(-30, 18);
    paletteSliderBody->SetAlignment(Alignment::CENTER, Alignment::CENTER);
    paletteSliderBody->SubscribePostDraw([=](Component* item, Graphics g) {
        D2D1_ROUNDED_RECT rrect = D2D1::RoundedRect(D2D1::RectF(0, 0, (FLOAT)item->GetWidth(), (FLOAT)item->GetHeight()), 3.0f, 3.0f);

        auto patternBrush = _CreateCheckeredPatternBrush(g, D2D1::ColorF(0x404040));
        if (patternBrush)
        {
            g.target->Clear(D2D1::ColorF(0x303030));
            g.target->FillRoundedRectangle(rrect, patternBrush);
            patternBrush->Release();
        }
        else
        {
            // TODO: Logging
        }

        auto brush = _CreateGradientBrush(item, g, _currentPalette);
        if (brush)
        {
            g.target->FillRoundedRectangle(rrect, brush);
            brush->Release();
        }
        else
        {
            // TODO: Logging
        }
    }).Detach();
    auto paletteSliderAnchor = Create<Dummy>();
    paletteSliderAnchor->SetBaseSize(5, 24);
    paletteSliderAnchor->SetVerticalAlignment(Alignment::CENTER);
    paletteSliderAnchor->SubscribePostDraw([=](Component* item, Graphics g) {
        if (_paletteSliderHovered)
        {
            g.target->DrawBitmap(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_top_hovered"), D2D1::RectF(0, 0, (FLOAT)item->GetWidth(), 6));
            g.target->DrawBitmap(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_bottom_hovered"), D2D1::RectF(0, (FLOAT)item->GetHeight() - 6, (FLOAT)item->GetWidth(), (FLOAT)item->GetHeight()));
        }
        else
        {
            g.target->DrawBitmap(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_top"), D2D1::RectF(0, 0, (FLOAT)item->GetWidth(), 6));
            g.target->DrawBitmap(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_bottom"), D2D1::RectF(0, (FLOAT)item->GetHeight() - 6, (FLOAT)item->GetWidth(), (FLOAT)item->GetHeight()));
        }
    }).Detach();
    _paletteSlider->SubscribeOnEnterInteractionArea([=](Slider* slider) {
        _paletteSliderHovered = true;
        slider->GetAnchorComponent()->InvokeRedraw();
    }).Detach();
    _paletteSlider->SubscribeOnLeaveInteractionArea([=](Slider* slider) {
        _paletteSliderHovered = false;
        slider->GetAnchorComponent()->InvokeRedraw();
    }).Detach();
    _paletteSlider->SubscribeOnValueChanged([=](Slider* item, float* value) {
        _positionInput->SetValue(NumberInputValue(*value), false);
        _OnColorPositionChanged(*value);
    }).Detach();
    _paletteSlider->SubscribeOnWheelUp([=](Component*, int, int) {
        _positionInput->StepUp();
    }).Detach();
    _paletteSlider->SubscribeOnWheelDown([=](Component*, int, int) {
        _positionInput->StepDown();
    }).Detach();
    _paletteSlider->SetBodyComponent(std::move(paletteSliderBody));
    _paletteSlider->SetAnchorComponent(std::move(paletteSliderAnchor));

    auto gradientStopCustomizationPanel = Create<FlexPanel>(FlexDirection::DOWN);
    gradientStopCustomizationPanel->FillContainerWidth();
    gradientStopCustomizationPanel->SetSpacing(5);
    gradientStopCustomizationPanel->SetPadding({ 0, 10, 0, 10 });
    gradientStopCustomizationPanel->SetBaseWidth(-30);
    gradientStopCustomizationPanel->SetHorizontalAlignment(Alignment::CENTER);
    gradientStopCustomizationPanel->SetCornerRounding(3.0f);
    gradientStopCustomizationPanel->SetBackgroundColor(D2D1::ColorF(0x282828));
    gradientStopCustomizationPanel->SetProperty(PROP_Shadow());
    gradientStopCustomizationPanel->SetProperty(FlexMarginBefore(-2));
    gradientStopCustomizationPanel->SetZIndex(0);

    _redColorInputGroup = Create<ColorSliderInputGroup>(L"Red:");
    _redColorInputGroup->FillContainerWidth();
    _redColorInputGroup->SetPadding({ 10, 0, 10, 0 });
    _redColorInputGroup->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics g) {
        std::vector<_GradientStop> stops;
        stops.push_back({ _currentColor.WithR(0).WithA(255), 0.0f });
        stops.push_back({ _currentColor.WithR(255).WithA(255), 1.0f });
        auto brush = _CreateGradientBrush(item, g, stops);
        if (brush)
        {
            g.target->FillRectangle(D2D1::RectF(0, 0, (FLOAT)item->GetWidth(), (FLOAT)item->GetHeight()), brush);
            brush->Release();
        }
        else
        {
            // TODO: Logging
        }
    }).Detach();
    _redColorInputGroup->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _currentColor.r = (uint8_t)value.getAsInteger();
        _OnColorChanged();
    }).Detach();
    _greenColorInputGroup = Create<ColorSliderInputGroup>(L"Green:");
    _greenColorInputGroup->FillContainerWidth();
    _greenColorInputGroup->SetPadding({ 10, 0, 10, 0 });
    _greenColorInputGroup->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics g) {
        std::vector<_GradientStop> stops;
        stops.push_back({ _currentColor.WithG(0).WithA(255), 0.0f });
        stops.push_back({ _currentColor.WithG(255).WithA(255), 1.0f });
        auto brush = _CreateGradientBrush(item, g, stops);
        if (brush)
        {
            g.target->FillRectangle(D2D1::RectF(0, 0, (FLOAT)item->GetWidth(), (FLOAT)item->GetHeight()), brush);
            brush->Release();
        }
        else
        {
            // TODO: Logging
        }
    }).Detach();
    _greenColorInputGroup->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _currentColor.g = (uint8_t)value.getAsInteger();
        _OnColorChanged();
    }).Detach();
    _blueColorInputGroup = Create<ColorSliderInputGroup>(L"Blue:");
    _blueColorInputGroup->FillContainerWidth();
    _blueColorInputGroup->SetPadding({ 10, 0, 10, 0 });
    _blueColorInputGroup->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics g) {
        std::vector<_GradientStop> stops;
        stops.push_back({ _currentColor.WithB(0).WithA(255), 0.0f });
        stops.push_back({ _currentColor.WithB(255).WithA(255), 1.0f });
        auto brush = _CreateGradientBrush(item, g, stops);
        if (brush)
        {
            g.target->FillRectangle(D2D1::RectF(0, 0, (FLOAT)item->GetWidth(), (FLOAT)item->GetHeight()), brush);
            brush->Release();
        }
        else
        {
            // TODO: Logging
        }
    }).Detach();
    _blueColorInputGroup->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _currentColor.b = (uint8_t)value.getAsInteger();
        _OnColorChanged();
    }).Detach();
    _opacityInputGroup = Create<ColorSliderInputGroup>(L"Opacity:");
    _opacityInputGroup->FillContainerWidth();
    _opacityInputGroup->SetPadding({ 10, 0, 10, 0 });
    _opacityInputGroup->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics g) {
        D2D1_RECT_F rect = D2D1::RectF(0, 0, (FLOAT)item->GetWidth(), (FLOAT)item->GetHeight());

        auto patternBrush = _CreateCheckeredPatternBrush(g, D2D1::ColorF(0x404040));
        if (patternBrush)
        {
            g.target->Clear(D2D1::ColorF(0x303030));
            g.target->FillRectangle(rect, patternBrush);
            patternBrush->Release();
        }
        else
        {
            // TODO: Logging
        }

        std::vector<_GradientStop> stops;
        stops.push_back({ _currentColor.WithA(0), 0.0f });
        stops.push_back({ _currentColor.WithA(255), 1.0f });
        auto brush = _CreateGradientBrush(item, g, stops);
        if (brush)
        {
            g.target->FillRectangle(rect, brush);
            brush->Release();
        }
        else
        {
            // TODO: Logging
        }
    }).Detach();
    _opacityInputGroup->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _currentColor.a = (uint8_t)value.getAsInteger();
        _OnColorChanged();
    }).Detach();
    auto positionRow = Create<FlexPanel>(FlexDirection::RIGHT);
    positionRow->FillContainerWidth();
    positionRow->SetPadding({ 10, 0, 10, 0 });
    auto positionLabel = Create<Label>(L"Position:");
    positionLabel->SetBaseHeight(26);
    positionLabel->SetProperty(FlexGrow());
    positionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    _positionInput = Create<NumberInput>();
    _positionInput->SetBaseSize(70, 26);
    _positionInput->SetValue(NumberInputValue(0));
    _positionInput->SetMinValue(NumberInputValue("0"));
    _positionInput->SetMaxValue(NumberInputValue("1"));
    _positionInput->SetStepSize(NumberInputValue("0.01"));
    _positionInput->SetPrecision(3);
    _positionInput->SetBackgroundColor(D2D1::ColorF(0x101010));
    _positionInput->SetCornerRounding(2.0f);
    _positionInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        _paletteSlider->SetValue((float)value.getAsDouble(), true);
    }).Detach();
    positionRow->AddItem(std::move(positionLabel));
    positionRow->AddItem(_positionInput.get());

    gradientStopCustomizationPanel->AddItem(_redColorInputGroup.get());
    gradientStopCustomizationPanel->AddItem(_greenColorInputGroup.get());
    gradientStopCustomizationPanel->AddItem(_blueColorInputGroup.get());
    gradientStopCustomizationPanel->AddItem(_opacityInputGroup.get());
    gradientStopCustomizationPanel->AddItem(std::move(positionRow));

    _gradientStopPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _gradientStopPanel->FillContainerWidth();
    _gradientStopPanel->SetBaseWidth(-30);
    _gradientStopPanel->SetHorizontalAlignment(Alignment::CENTER);
    _gradientStopPanel->SetProperty(FlexMarginBefore(-6));
    _gradientStopPanel->SetPadding({ 1, 7, 1, 1 });
    _gradientStopPanel->SetCornerRounding(3.0f);
    _gradientStopPanel->SetBackgroundColor(D2D1::ColorF(0x101010));

    auto presetRow = Create<FlexPanel>(FlexDirection::RIGHT);
    presetRow->FillContainerWidth();
    presetRow->SetSpacing(5);
    presetRow->SetPadding({ 15, 0, 15, 0 });
    presetRow->SetItemAlignment(Alignment::CENTER);
    presetRow->SetProperty(FlexMarginBefore(5));
    auto presetLabel = Create<Label>(L"Presets:");
    presetLabel->SetBaseHeight(26);
    presetLabel->SetVerticalTextAlignment(Alignment::CENTER);
    presetLabel->SetProperty(FlexGrow());
    auto rainbowPresetButton = Create<Button>();
    NeutralButtonStyle::Apply(rainbowPresetButton.get());
    rainbowPresetButton->SetBaseSize(20, 20);
    rainbowPresetButton->SubscribePreContentDraw([=](Component* item, Graphics g) {
        D2D1_SIZE_F size = g.target->GetSize();
        std::vector<_GradientStop> stops;
        stops.push_back({ zutil::Color(0xFF0000, 255), (1.0f / 6.0f) * 0 });
        stops.push_back({ zutil::Color(0xFFFF00, 255), (1.0f / 6.0f) * 1 });
        stops.push_back({ zutil::Color(0x00FF00, 255), (1.0f / 6.0f) * 2 });
        stops.push_back({ zutil::Color(0x00FFFF, 255), (1.0f / 6.0f) * 3 });
        stops.push_back({ zutil::Color(0x0000FF, 255), (1.0f / 6.0f) * 4 });
        stops.push_back({ zutil::Color(0xFF00FF, 255), (1.0f / 6.0f) * 5 });
        stops.push_back({ zutil::Color(0xFF0000, 255), (1.0f / 6.0f) * 6 });
        auto brush = _CreateGradientBrush(item, g, stops);
        if (brush)
        {
            g.target->FillRectangle(D2D1::RectF(0.0f, 0.0f, size.width, size.height), brush);
            brush->Release();
        }
    }).Detach();
    rainbowPresetButton->SubscribeOnActivated([=]() {
        ExecuteSynchronously([=]() {
            _currentPalette.clear();
            _currentPalette.push_back({ zutil::Color(0xFF0000, 255), (1.0f / 6.0f) * 0 });
            _currentPalette.push_back({ zutil::Color(0xFFFF00, 255), (1.0f / 6.0f) * 1 });
            _currentPalette.push_back({ zutil::Color(0x00FF00, 255), (1.0f / 6.0f) * 2 });
            _currentPalette.push_back({ zutil::Color(0x00FFFF, 255), (1.0f / 6.0f) * 3 });
            _currentPalette.push_back({ zutil::Color(0x0000FF, 255), (1.0f / 6.0f) * 4 });
            _currentPalette.push_back({ zutil::Color(0xFF00FF, 255), (1.0f / 6.0f) * 5 });
            _currentPalette.push_back({ zutil::Color(0xFF0000, 255), (1.0f / 6.0f) * 6 });
            _BuildItemsForPalette();
            _OnGradientStopSelected(0);
            _ReorderColorList();
            _SavePalette();
        });
    }).Detach();
    auto cyanMagentaPresetButton = Create<Button>();
    NeutralButtonStyle::Apply(cyanMagentaPresetButton.get());
    cyanMagentaPresetButton->SetBaseSize(20, 20);
    cyanMagentaPresetButton->SubscribePreContentDraw([=](Component* item, Graphics g) {
        D2D1_SIZE_F size = g.target->GetSize();
        std::vector<_GradientStop> stops;
        stops.push_back({ zutil::Color(0x00F0F0, 255), 0.0f });
        stops.push_back({ zutil::Color(0xF000F0, 255), 0.5f });
        stops.push_back({ zutil::Color(0x00F0F0, 255), 1.0f });
        auto brush = _CreateGradientBrush(item, g, stops);
        if (brush)
        {
            g.target->FillRectangle(D2D1::RectF(0.0f, 0.0f, size.width, size.height), brush);
            brush->Release();
        }
    }).Detach();
    cyanMagentaPresetButton->SubscribeOnActivated([=]() {
        ExecuteSynchronously([=]() {
            _currentPalette.clear();
            _currentPalette.push_back({ zutil::Color(0x00F0F0, 255), 0.0f });
            _currentPalette.push_back({ zutil::Color(0xF000F0, 255), 0.5f });
            _currentPalette.push_back({ zutil::Color(0x00F0F0, 255), 1.0f });
            _BuildItemsForPalette();
            _OnGradientStopSelected(0);
            _ReorderColorList();
            _SavePalette();
        });
    }).Detach();
    presetRow->AddItem(std::move(presetLabel));
    presetRow->AddItem(std::move(rainbowPresetButton));
    presetRow->AddItem(std::move(cyanMagentaPresetButton));

    colorsPanel->AddItem(std::move(Create<SectionHeader>(L"Colors", RECT{ 15, 0, 15, 0 })));
    colorsPanel->AddItem(std::move(headColorRow));
    colorsPanel->AddItem(std::move(trailColorsLabel));
    colorsPanel->AddItem(std::move(trailColorsSeparator));
    colorsPanel->AddItem(_paletteSlider.get());
    colorsPanel->AddItem(std::move(gradientStopCustomizationPanel));
    colorsPanel->AddItem(_gradientStopPanel.get());
    colorsPanel->AddItem(std::move(presetRow));
    return colorsPanel;
}

void zcom::CursorTrailParameterPanel::_UpdateActiveItems()
{
    _widthInput->SetActive(!_overlayWindowId && !_fullMonitorCheckbox->Checked());
    _heightInput->SetActive(!_overlayWindowId && !_fullMonitorCheckbox->Checked());
    _xOffsetInput->SetActive(!_overlayWindowId && !_fullMonitorCheckbox->Checked());
    _yOffsetInput->SetActive(!_overlayWindowId && !_fullMonitorCheckbox->Checked());
}

void zcom::CursorTrailParameterPanel::_UpdateColorInput()
{
    zutil::Color headColor = zutil::Color(_scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::HEAD_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _headColorInput->SetBackgroundColor(D2D1::ColorF(headColor.ToIntNoAlpha(), headColor.a / 255.0f));
}

void zcom::CursorTrailParameterPanel::_UpdateButtonsBasedOnOverlayState()
{
    if (_overlayWindowId)
    {
        _enableButton->Label()->SetText(L"Disable");
        DisableButtonStyle::Apply(_enableButton.get());
    }
    else
    {
        _enableButton->Label()->SetText(L"Enable");
        EnableButtonStyle::Apply(_enableButton.get());
    }
}

void zcom::CursorTrailParameterPanel::_OpenOverlayWindow()
{
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
        .WindowClassName(CursorTrailConfig::OVERLAY_WINDOW_NAME)
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
            wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
            wnd->resourceManager.InitImage("trail_head_icon");

            // Remove window decorations
            DefaultNonClientAreaSceneOptions ncOpt;
            ncOpt.drawWindowShadow = false;
            ncOpt.drawWindowBorder = false;
            ncOpt.resizingBorderWidths = { 0, 0, 0, 0 };
            ncOpt.clientAreaMargins = { 0, 0, 0, 0 };
            wnd->LoadNonClientAreaScene<DefaultNonClientAreaScene>(&ncOpt);

            wnd->LoadStartingScene<CursorTrailScene>(nullptr);
        }
    );
}

void zcom::CursorTrailParameterPanel::_CloseOverlayWindow()
{
    Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_overlayWindowId.value());
    if (handle.Valid())
        handle->Close();
    _overlayWindowId = std::nullopt;
}

ID2D1LinearGradientBrush* zcom::CursorTrailParameterPanel::_CreateGradientBrush(Component* item, Graphics g, const std::vector<_GradientStop>& stops)
{
    ID2D1GradientStopCollection* pGradientStops = nullptr;
    ID2D1LinearGradientBrush* pGradientBrush = nullptr;

    std::vector<D2D1_GRADIENT_STOP> gradientStops;
    for (auto& stop : stops)
        gradientStops.push_back({ stop.position, D2D1::ColorF(stop.color.r / 255.0f, stop.color.g / 255.0f, stop.color.b / 255.0f, stop.color.a / 255.0f) });

    g.target->CreateGradientStopCollection(
        gradientStops.data(),
        (UINT32)gradientStops.size(),
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &pGradientStops
    );
    if (pGradientStops)
    {
        g.target->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(0, 0),
                D2D1::Point2F((FLOAT)item->GetWidth(), 0)),
            pGradientStops,
            &pGradientBrush
        );
        pGradientStops->Release();
        if (pGradientBrush)
        {
            return pGradientBrush;
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
    return nullptr;
}

ID2D1ImageBrush* zcom::CursorTrailParameterPanel::_CreateCheckeredPatternBrush(Graphics g, D2D1_COLOR_F cellColor)
{
    float cellSize = 6.0f;

    ComPtr<ID2D1CommandList> patternCommandList = nullptr;
    g.target->CreateCommandList(patternCommandList.GetAddressOf());
    if (!patternCommandList)
    {
        // TODO: Logging
        return nullptr;
    }

    ComPtr<ID2D1SolidColorBrush> cellBrush = nullptr;
    g.target->CreateSolidColorBrush(cellColor, cellBrush.GetAddressOf());
    if (!cellBrush)
    {
        // TODO: Logging
        return nullptr;
    }

    ID2D1Image* stash = nullptr;
    g.target->GetTarget(&stash);
    g.target->SetTarget(patternCommandList.Get());
    g.target->FillRectangle(D2D1::RectF(0.0f, 0.0f, cellSize, cellSize), cellBrush.Get());
    g.target->FillRectangle(D2D1::RectF(cellSize, cellSize, cellSize * 2, cellSize * 2), cellBrush.Get());
    g.target->SetTarget(stash);
    stash->Release();

    patternCommandList->Close();

    ID2D1ImageBrush* patternBrush = nullptr;
    g.target->CreateImageBrush(
        patternCommandList.Get(),
        D2D1::ImageBrushProperties(
            D2D1::RectF(0, 0, cellSize * 2, cellSize * 2),
            D2D1_EXTEND_MODE_WRAP,
            D2D1_EXTEND_MODE_WRAP
        ),
        &patternBrush
    );
    if (!patternBrush)
    {
        // TODO: Logging
    }

    return patternBrush;
}

void zcom::CursorTrailParameterPanel::_BuildItemsForPalette()
{
    for (auto& stop : _currentPalette)
        _CreateInnerGradientStopComponents(stop);
}

void zcom::CursorTrailParameterPanel::_CreateInnerGradientStopComponents(_GradientStop& stop)
{
    stop.selectorItem = Create<FlexPanel>(FlexDirection::RIGHT);
    stop.selectorItem->FillContainerWidth();
    stop.selectorItem->SetHeightFixed(true);
    stop.selectorItem->SetBaseHeight(30);
    stop.selectorItem->SetPadding({ 13, 3, 3, 3 });
    stop.selectorItem->SetSpacing(4);
    stop.selectorItem->AddTag("color_selector");
    stop.colorIndicator = Create<Dummy>();
    stop.colorIndicator->SetBaseWidth(12);
    stop.colorIndicator->SetParentHeightPercent(1.0f);
    stop.colorIndicator->SetVerticalAlignment(Alignment::CENTER);
    stop.colorIndicator->SetBackgroundColor(D2D1::ColorF(stop.color.r / 255.0f, stop.color.g / 255.0f, stop.color.b / 255.0f, stop.color.a / 255.0f));
    stop.colorIndicator->SetCornerRounding(3.0f);
    stop.colorIndicator->SetProperty(FlexMarginAfter(4));
    stop.redLabel = Create<Label>(std::to_wstring(stop.color.r));
    stop.redLabel->SetParentHeightPercent(1.0f);
    stop.redLabel->SetBaseWidth(24);
    stop.redLabel->SetVerticalTextAlignment(Alignment::CENTER);
    stop.redLabel->SetHorizontalTextAlignment(TextAlignment::CENTER);
    stop.redLabel->SetFontColor(D2D1::ColorF(0xA0A0A0));
    stop.redUnderline = Create<Dummy>();
    stop.redUnderline->SetBaseSize(24, 1);
    stop.redUnderline->SetBackgroundColor(D2D1::ColorF(0xA00000));
    stop.redUnderline->SetVerticalAlignment(Alignment::END);
    stop.redUnderline->SetProperty(FlexMarginBefore(-24 - 4));
    stop.greenLabel = Create<Label>(std::to_wstring(stop.color.g));
    stop.greenLabel->SetParentHeightPercent(1.0f);
    stop.greenLabel->SetBaseWidth(24);
    stop.greenLabel->SetVerticalTextAlignment(Alignment::CENTER);
    stop.greenLabel->SetHorizontalTextAlignment(TextAlignment::CENTER);
    stop.greenLabel->SetFontColor(D2D1::ColorF(0xA0A0A0));
    stop.greenUnderline = Create<Dummy>();
    stop.greenUnderline->SetBaseSize(24, 1);
    stop.greenUnderline->SetBackgroundColor(D2D1::ColorF(0x00A000));
    stop.greenUnderline->SetVerticalAlignment(Alignment::END);
    stop.greenUnderline->SetProperty(FlexMarginBefore(-24 - 4));
    stop.blueLabel = Create<Label>(std::to_wstring(stop.color.b));
    stop.blueLabel->SetParentHeightPercent(1.0f);
    stop.blueLabel->SetBaseWidth(24);
    stop.blueLabel->SetVerticalTextAlignment(Alignment::CENTER);
    stop.blueLabel->SetHorizontalTextAlignment(TextAlignment::CENTER);
    stop.blueLabel->SetFontColor(D2D1::ColorF(0xA0A0A0));
    stop.blueUnderline = Create<Dummy>();
    stop.blueUnderline->SetBaseSize(24, 1);
    stop.blueUnderline->SetBackgroundColor(D2D1::ColorF(0x0000A0));
    stop.blueUnderline->SetVerticalAlignment(Alignment::END);
    stop.blueUnderline->SetProperty(FlexMarginBefore(-24 - 4));
    stop.opacityLabel = Create<Label>(std::to_wstring(stop.color.a));
    stop.opacityLabel->SetParentHeightPercent(1.0f);
    stop.opacityLabel->SetBaseWidth(24);
    stop.opacityLabel->SetVerticalTextAlignment(Alignment::CENTER);
    stop.opacityLabel->SetHorizontalTextAlignment(TextAlignment::CENTER);
    stop.opacityLabel->SetFontColor(D2D1::ColorF(0xA0A0A0));
    stop.opacityUnderline = Create<Dummy>();
    stop.opacityUnderline->SetBaseSize(24, 1);
    stop.opacityUnderline->SetBackgroundColor(D2D1::ColorF(0xA0A0A0));
    stop.opacityUnderline->SetVerticalAlignment(Alignment::END);
    stop.opacityUnderline->SetProperty(FlexMarginBefore(-24 - 4));
    stop.positionLabel = Create<Label>((std::wostringstream() << std::fixed << std::setprecision(3) << stop.position).str());
    stop.positionLabel->SetParentHeightPercent(1.0f);
    stop.positionLabel->SetBaseWidth(40);
    stop.positionLabel->SetVerticalTextAlignment(Alignment::CENTER);
    stop.positionLabel->SetHorizontalTextAlignment(TextAlignment::CENTER);
    stop.positionLabel->SetFontColor(D2D1::ColorF(0xA0A0A0));
    auto spacer = Create<Dummy>();
    spacer->SetBaseHeight(0);
    spacer->SetProperty(FlexGrow());
    stop.removeButton = Create<Button>();
    RoundedLiftedButtonStyle::Apply(stop.removeButton.get());
    stop.removeButton->SetBaseSize(20, 20);
    stop.removeButton->SetVerticalAlignment(Alignment::CENTER);
    stop.removeButton->SetActivation(ButtonActivation::PRESS);
    stop.removeButton->SetButtonColor(D2D1::ColorF(0x383838));
    stop.removeButton->SetButtonHoverColor(D2D1::ColorF(0x484848));
    stop.removeButton->SetButtonClickColor(D2D1::ColorF(0x303030));
    stop.removeButton->SetButtonImageAll(_scene->GetWindow()->resourceManager.GetImage("minus_1"));
    stop.removeButton->ButtonImage()->SetPlacement(ImagePlacement::CENTER);
    stop.removeButton->UseImageParamsForAll(stop.removeButton->ButtonImage());
    stop.removeButton->ButtonImage()->SetTintColor(D2D1::ColorF(0xA0A0A0));
    stop.removeButton->ButtonHoverImage()->SetTintColor(D2D1::ColorF(0xD0D0D0));
    stop.removeButton->ButtonClickImage()->SetTintColor(D2D1::ColorF(0xD0D0D0));

    stop.selectorItem->AddItem(stop.colorIndicator.get());
    stop.selectorItem->AddItem(stop.redLabel.get());
    stop.selectorItem->AddItem(stop.redUnderline.get());
    stop.selectorItem->AddItem(stop.greenLabel.get());
    stop.selectorItem->AddItem(stop.greenUnderline.get());
    stop.selectorItem->AddItem(stop.blueLabel.get());
    stop.selectorItem->AddItem(stop.blueUnderline.get());
    stop.selectorItem->AddItem(stop.opacityLabel.get());
    stop.selectorItem->AddItem(stop.opacityUnderline.get());
    stop.selectorItem->AddItem(stop.positionLabel.get());
    stop.selectorItem->AddItem(std::move(spacer));
    stop.selectorItem->AddItem(stop.removeButton.get());
}

void zcom::CursorTrailParameterPanel::_ReorderColorList()
{
    // Deferring layout updates here is not only for performance
    // If no deferring is done, after clearing all items the settings panel will resize and scroll up before readding the items, resulting in a jarring jump
    _gradientStopPanel->DeferLayoutUpdates();
    _gradientStopPanel->ClearItems();
    _UpdateColorListBackgrounds();
    for (int i = 0; i < _currentPalette.size(); i++)
    {
        _GradientStop& stop = _currentPalette[i];
        stop.postLeftPressedEventSubscription = stop.selectorItem->SubscribePostLeftPressed([=, removeButton = stop.removeButton.get()](Component* item, std::vector<EventTargets::Params> targets, int, int) {
            if (!targets.empty() && targets.front().target != removeButton)
                _OnGradientStopSelected(i);
        });
        stop.removeButton->SetVisible(_currentPalette.size() > 1);
        stop.removeEventSubscription = stop.removeButton->SubscribeOnActivated([=]() {
            ExecuteSynchronously([=]() { _OnGradientStopRemoved(i); });
        });
        _gradientStopPanel->AddItem(stop.selectorItem.get());
    }
    _gradientStopPanel->ResumeLayoutUpdates();
}

void zcom::CursorTrailParameterPanel::_UpdateColorListBackgrounds()
{
    for (int i = 0; i < _currentPalette.size(); i++)
    {
        _GradientStop& stop = _currentPalette[i];
        int baseBackgroundColor = _currentColorIndex == i ? 0x004070 : (i % 2 == 0 ? 0x161616 : 0x1D1D1D);
        int hoveredBackgroundColor = _currentColorIndex == i ? 0x004070 : 0x282828;
        stop.selectorItem->SetBackgroundColor(D2D1::ColorF(stop.selectorItem->GetMouseInside() ? hoveredBackgroundColor : baseBackgroundColor));
        stop.mouseEnterEventSubscription = stop.selectorItem->SubscribeOnMouseEnter([=](Component* item) { item->SetBackgroundColor(D2D1::ColorF(hoveredBackgroundColor)); });
        stop.mouseLeaveEventSubscription = stop.selectorItem->SubscribeOnMouseLeave([=](Component* item) { item->SetBackgroundColor(D2D1::ColorF(baseBackgroundColor)); });
    }
}

void zcom::CursorTrailParameterPanel::_OnGradientStopSelected(int index)
{
    _currentColorIndex = index;
    _currentColor = _currentPalette[index].color;
    _paletteSlider->SetValue(_currentPalette[index].position);
    _redColorInputGroup->GetInput()->SetValue(NumberInputValue(_currentColor.r));
    _greenColorInputGroup->GetInput()->SetValue(NumberInputValue(_currentColor.g));
    _blueColorInputGroup->GetInput()->SetValue(NumberInputValue(_currentColor.b));
    _opacityInputGroup->GetInput()->SetValue(NumberInputValue(_currentColor.a));
    _UpdateColorListBackgrounds();
}

void zcom::CursorTrailParameterPanel::_OnGradientStopRemoved(int index)
{
    _currentPalette.erase(_currentPalette.begin() + index);
    _SavePalette();
    if (index < _currentColorIndex)
    {
        _currentColorIndex--;
    }
    else if (index == _currentColorIndex)
    {
        if (_currentColorIndex <= _currentPalette.size() - 1)
            _OnGradientStopSelected(_currentColorIndex);
        else
            _OnGradientStopSelected(_currentColorIndex - 1);
    }
    _ReorderColorList();
}

void zcom::CursorTrailParameterPanel::_OnColorChanged()
{
    _redColorInputGroup->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _greenColorInputGroup->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _blueColorInputGroup->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _opacityInputGroup->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _paletteSlider->GetBodyComponent()->InvokeRedraw();

    auto& changedItem = _currentPalette[_currentColorIndex];
    if (changedItem.color.r != _currentColor.r)
        changedItem.redLabel->SetText(std::to_wstring(_currentColor.r));
    if (changedItem.color.g != _currentColor.g)
        changedItem.greenLabel->SetText(std::to_wstring(_currentColor.g));
    if (changedItem.color.b != _currentColor.b)
        changedItem.blueLabel->SetText(std::to_wstring(_currentColor.b));
    if (changedItem.color.a != _currentColor.a)
        changedItem.opacityLabel->SetText(std::to_wstring(_currentColor.a));
    changedItem.color = _currentColor;
    changedItem.colorIndicator->SetBackgroundColor(D2D1::ColorF(changedItem.color.r / 255.0f, changedItem.color.g / 255.0f, changedItem.color.b / 255.0f, changedItem.color.a / 255.0f));
    _SavePalette();
}

void zcom::CursorTrailParameterPanel::_OnColorPositionChanged(float position)
{
    _paletteSlider->GetBodyComponent()->InvokeRedraw();

    std::vector<float> palettePositions = streams::From(_currentPalette)
        .Map<float>([](const _GradientStop& stop) { return stop.position; })
        .SkipRange(_currentColorIndex, 1)
        .ToVector();

    int positionIndex = 0;
    while (positionIndex < palettePositions.size() && position >= palettePositions[positionIndex])
        positionIndex++;

    _currentPalette[_currentColorIndex].position = position;
    _currentPalette[_currentColorIndex].positionLabel->SetText((std::wostringstream() << std::fixed << std::setprecision(3) << position).str());
    if (positionIndex == _currentColorIndex)
    {
        _SavePalette();
        return;
    }

    _GradientStop stop = std::move(_currentPalette[_currentColorIndex]);
    _currentPalette.erase(_currentPalette.begin() + _currentColorIndex);
    _currentPalette.insert(_currentPalette.begin() + positionIndex, std::move(stop));
    _SavePalette();
    _currentColorIndex = positionIndex;
    _ReorderColorList();
}

void zcom::CursorTrailParameterPanel::_LoadPalette()
{
    _ParsePaletteString(_scene->GetApp()->config.GetConfigValue(CursorTrailConfig::PALETTE, Config::ADD_IF_MISSING));
    if (_currentPalette.empty())
    {
        _ParsePaletteString(CursorTrailConfig::PALETTE.defaultValue);
        _scene->GetApp()->config.SetValue(CursorTrailConfig::PALETTE.name, CursorTrailConfig::PALETTE.defaultValue);
    }
}

void zcom::CursorTrailParameterPanel::_ParsePaletteString(const std::wstring& str)
{
    _currentPalette.clear();
    std::vector<std::wstring> stopStrings;
    split_wstr(str, stopStrings, L'|', true);
    for (auto& stopString : stopStrings)
    {
        std::array<std::wstring, 2> parts;
        split_wstr(stopString, parts, L',');
        _currentPalette.push_back(std::move(_GradientStop{ zutil::Color(std::stoi(parts[0])), std::stof(parts[1]) }));
    }
    std::sort(_currentPalette.begin(), _currentPalette.end(), [](const _GradientStop& stop1, const _GradientStop& stop2) { return stop1.position < stop2.position; });
}

void zcom::CursorTrailParameterPanel::_SavePalette()
{
    std::wostringstream ss(L"");
    for (int i = 0; i < _currentPalette.size(); i++)
    {
        if (i != 0)
            ss << '|';
        ss << _currentPalette[i].color.ToInt() << ',' << std::fixed << std::setprecision(3) << _currentPalette[i].position << std::defaultfloat;
    }
    _scene->GetApp()->config.SetValue(CursorTrailConfig::PALETTE.name, ss.str());
    _lastSaveTime = ztime::Main();
    _configSaved = false;
}

std::optional<zwnd::WindowId> zcom::CursorTrailParameterPanel::_OpenColorSelector(std::wstring windowTitle, std::wstring wndClass, ConfigValue<int> colorConfigValue)
{
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

    return _scene->GetApp()->CreateChildWindow(
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
            opt.configValue = colorConfigValue;
            wnd->LoadStartingScene<ColorSelectorScene>(&opt);
        }
    );
}
