#include "App.h"
#include "SharedContext.h"
#include "CursorTrailSetupComponent.h"
#include "CursorTrailConfig.h"

#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Components/NumberParameterInput.h"
#include "Shared/Styles/Styles.h"
#include "Shared/Util/Streams.h"

void zcom::CursorTrailSetupComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    ScrollPanel::Init();

    size = { 300, 0 };
    parentSize = { 0.0f, 1.0f };
    backgroundColor = Color(0x202020);
    yScrollbar.scrollable = true;
    yScrollbar.backgroundVisible = true;

    _overlay = overlay;
    _overlayView.emplace(overlay->Id(), this);

    
    _CreateInsertionMarker();

    auto flexPanel = Create<FlexPanel>(FlexDirection::DOWN);
    flexPanel->parentSize = { 1.0f, 0.0f };
    flexPanel->autoHeight = true;
    flexPanel->AddItem(_SetUpGeneralPanel());
    flexPanel->AddItem(_SetUpAppearancePanel());
    flexPanel->AddItem(_SetUpColorsPanel());
    flexPanel->SubscribePostMouseMove([=](Component* panel, std::vector<EventContext::Params> targets, Point point, Point) {
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

                int panelWindowY = panel->windowPosition_->y;
                int itemWindowY = target.target->windowPosition_->y;
                if (target.point.value_or(Point{ 0, 0 }).y < target.target->size_->height / 2)
                {
                    _hoveredColorSelectorIsTopHalf = true;
                    _insertionMarker->position.Assign(Y(itemWindowY - panelWindowY - _insertionMarker->size_->height / 2));
                }
                else
                {
                    _hoveredColorSelectorIsTopHalf = false;
                    _insertionMarker->position.Assign(Y(itemWindowY - panelWindowY - _insertionMarker->size_->height / 2 + target.target->size_->height));
                }
                _insertionMarker->visible = true;
                return;
            }
            else if (target.target == _insertionMarker.get())
            {
                return;
            }
        }
        _insertionMarker->visible = false;
    }).Detach();
    // OnLeave handler set on outer-most component, since if it is set on flexPanel, every time the insertionMarker is shown,
    // the mouse enters the marker and MouseLeave on the flexPanel is invoked (since insertionMarker is not a child of flexPanel)
    // resulting in rapid toggling between visible and hidden
    SubscribeOnMouseLeave([=](Component*) {
        _insertionMarker->visible = false;
    }).Detach();
    AddItem(std::move(flexPanel));

    _LoadPalette();
    _BuildItemsForPalette();
    _OnGradientStopSelected(0);
    _ReorderColorList();

    _OnColorChanged();
}

void zcom::CursorTrailSetupComponent::_OnUpdate()
{
    ScrollPanel::_OnUpdate();

    if (!_configSaved && ztime::Main() > _lastSaveTime + Duration(250, MILLISECONDS))
    {
        _configSaved = true;
        _scene->GetApp()->config.SaveConfig();
    }
}

void zcom::CursorTrailSetupComponent::_CreateInsertionMarker()
{
    _insertionMarker = Create<FlexPanel>(FlexDirection::RIGHT);
    _insertionMarker->parentSize = { 1.0f, 0.0f };
    _insertionMarker->size = { -10, 20 };
    _insertionMarker->xAlign = Alignment::CENTER;
    _insertionMarker->padding = { 0, 0, 11, 0 };
    _insertionMarker->itemAlignment = Alignment::CENTER;
    _insertionMarker->position = { 0, 636 };
    _insertionMarker->zIndex = 1;
    _insertionMarker->SetProperty(Shadow().WithBlurStandardDeviation(1.5f));
    _insertionMarker->fallthroughMouseEvents = true;
    auto insertionMarkerHead = Create<Button>();
    RoundedLiftedButtonStyle::Apply(insertionMarkerHead.get());
    insertionMarkerHead->size = { 20, 20 };
    insertionMarkerHead->yAlign = Alignment::CENTER;
    insertionMarkerHead->activation = ButtonActivation::PRESS;
    insertionMarkerHead->ValueFromButtonState<Color>(insertionMarkerHead->buttonColor, Color(0x383838), Color(0x484848), Color(0x303030));
    insertionMarkerHead->Image()->image = _scene->GetWindow()->resourceManager.GetImage("insertion_marker_icon");
    insertionMarkerHead->Image()->imagePlacement = ImagePlacement::CENTER;
    insertionMarkerHead->ValueFromButtonState<Color>(insertionMarkerHead->Image()->tintColor, Color(0xA0A0A0), Color(0xD0D0D0), Color(0xD0D0D0));
    auto insertionMarkerTransition = Create<Image>(_scene->GetWindow()->resourceManager.GetImage("wall_to_2px_pipe_transition_right"));
    insertionMarkerTransition->size = { 2, 8 };
    insertionMarkerTransition->tintColor = Color(0x383838);
    auto insertionMarkerSeparator = Create<Dummy>();
    insertionMarkerSeparator->size = { 0, 2 };
    insertionMarkerSeparator->SetProperty(FlexGrow());
    insertionMarkerSeparator->backgroundColor = Color(0x383838);

    insertionMarkerHead->SubscribeOnMouseEnterArea([=, transition = insertionMarkerTransition.get(), separator = insertionMarkerSeparator.get()](Component* item) {
        _SetInsertionMarkerColors((Button*)item, transition, separator);
    }).Detach();
    insertionMarkerHead->SubscribeOnMouseLeaveArea([=, transition = insertionMarkerTransition.get(), separator = insertionMarkerSeparator.get()](Component* item) {
        _SetInsertionMarkerColors((Button*)item, transition, separator);
    }).Detach();
    insertionMarkerHead->SubscribeOnLeftPressed([=, transition = insertionMarkerTransition.get(), separator = insertionMarkerSeparator.get()](Component* item, Point) {
        _SetInsertionMarkerColors((Button*)item, transition, separator);
    }).Detach();
    insertionMarkerHead->SubscribeOnLeftReleased([=, transition = insertionMarkerTransition.get(), separator = insertionMarkerSeparator.get()](Component* item, std::optional<Point>) {
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
                    Color(
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
                    Color(
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

void zcom::CursorTrailSetupComponent::_SetInsertionMarkerColors(Button* head, Image* transition, Dummy* separator)
{
    if (head->hoveredArea_)
    {
        _insertionMarker->SetProperty(Shadow().WithBlurStandardDeviation(3.0f));
        if (head->leftClicked_)
        {
            transition->tintColor = Color(0x303030);
            separator->backgroundColor = Color(0x303030);
        }
        else
        {
            transition->tintColor = Color(0x484848);
            separator->backgroundColor = Color(0x484848);
        }
    }
    else
    {
        _insertionMarker->SetProperty(Shadow().WithBlurStandardDeviation(1.5f));
        transition->tintColor = Color(0x383838);
        separator->backgroundColor = Color(0x383838);
    }
}

std::unique_ptr<zcom::FlexPanel> zcom::CursorTrailSetupComponent::_SetUpGeneralPanel()
{
    auto generalPanel = Create<FlexPanel>(FlexDirection::DOWN);
    generalPanel->parentSize = { 1.0f, 0.0f };
    generalPanel->autoHeight = true;

    auto titleRow = Create<FlexPanel>(FlexDirection::RIGHT);
    titleRow->parentSize = { 1.0f, 0.0f };
    titleRow->autoHeight = true;
    titleRow->padding = { 15, 15, 15, 15 };
    auto generalLabel = Create<Label>(L"Cursor trail");
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

    auto overlayLayoutSetupPanel = Create<OverlayLayoutSetup>(_scene->GetApp()->config.GetConfigValue(CursorTrailConfig::LAYOUT_STRING));
    overlayLayoutSetupPanel->parentSize = { 1.0f, 0.0f };
    overlayLayoutSetupPanel->padding = { 0, 0, 0, 10 };
    overlayLayoutSetupPanel->autoHeight = true;
    overlayLayoutSetupPanel->SubscribeOnOverlayLayoutStringChanged([=](std::wstring layoutString) {
        _scene->GetApp()->config.SetValue(CursorTrailConfig::LAYOUT_STRING.name, layoutString);
    }).Detach();

    generalPanel->AddItem(std::move(titleRow));
    generalPanel->AddItem(std::move(overlayLayoutSetupPanel));
    return generalPanel;
}

std::unique_ptr<zcom::FlexPanel> zcom::CursorTrailSetupComponent::_SetUpAppearancePanel()
{
    auto appearancePanel = Create<FlexPanel>(FlexDirection::DOWN);
    appearancePanel->parentSize = { 1.0f, 0.0f };
    appearancePanel->autoHeight = true;

    auto trailWidthInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail width", L"How thick the trail is, in pixels",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_WIDTH, Config::ADD_IF_MISSING),
        1, 1000, 1
    ));
    trailWidthInput->GetInput()->size.Assign(Width(70));
    trailWidthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::TRAIL_WIDTH.name, value.getAsInteger());
    }).Detach();
    
    auto headSizeInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail head size", L"How big the head of the trail is, in pixels",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::HEAD_SIZE, Config::ADD_IF_MISSING),
        1, 1000, 1
    ));
    headSizeInput->GetInput()->size.Assign(Width(70));
    headSizeInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::HEAD_SIZE.name, value.getAsInteger());
    }).Detach();
    
    auto trailEdgeWidthInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Trail edge width", L"How wide the fade out area on the edge of the trail is, in pixels",
        _scene->GetApp()->config.GetDoubleConfigValue(CursorTrailConfig::TRAIL_EDGE_WIDTH, Config::ADD_IF_MISSING),
        0.1f, 1000.0f, 0.1f
    ));
    trailEdgeWidthInput->GetInput()->size.Assign(Width(70));
    trailEdgeWidthInput->GetInput()->precision = 1;
    trailEdgeWidthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(CursorTrailConfig::TRAIL_EDGE_WIDTH.name, value.getAsDouble());
    }).Detach();
    
    auto headEdgeWidthInput = Create<NumberParameterInput<float>>(NumberParameterInputParams(
        L"Head edge width", L"How wide the fade out area on the edge of the trail head is, in pixels",
        _scene->GetApp()->config.GetDoubleConfigValue(CursorTrailConfig::HEAD_EDGE_WIDTH, Config::ADD_IF_MISSING),
        0.1f, 1000.0f, 0.1f
    ));
    headEdgeWidthInput->GetInput()->size.Assign(Width(70));
    headEdgeWidthInput->GetInput()->precision = 1;
    headEdgeWidthInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetDoubleValue(CursorTrailConfig::HEAD_EDGE_WIDTH.name, value.getAsDouble());
    }).Detach();
    
    auto trailLifetimeInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail lifetime", L"How is the trail visible for, in milliseconds",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_LIFETIME, Config::ADD_IF_MISSING),
        1, 10000, 50
    ));
    trailLifetimeInput->GetInput()->size.Assign(Width(70));
    trailLifetimeInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::TRAIL_LIFETIME.name, value.getAsInteger());
    }).Detach();
    
    auto colorCycleDurationInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Color cycle duration", L"How long it takes to cycle through all of the colors, in milliseconds",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::COLOR_CYCLE_DURATION, Config::ADD_IF_MISSING),
        1, 100000, 500
    ));
    colorCycleDurationInput->GetInput()->size.Assign(Width(70));
    colorCycleDurationInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::COLOR_CYCLE_DURATION.name, value.getAsInteger());
    }).Detach();

    auto iconSpinDurationInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Icon spin duration", L"How long it takes for the cursor icon to make a full rotation, in milliseconds",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::ICON_SPIN_DURATION, Config::ADD_IF_MISSING),
        1, 100000, 500
    ));
    iconSpinDurationInput->GetInput()->size.Assign(Width(70));
    iconSpinDurationInput->GetInput()->SubscribeOnValueChanged([=](NumberInputValue value) {
        _scene->GetApp()->config.SetIntValue(CursorTrailConfig::ICON_SPIN_DURATION.name, value.getAsInteger());
    }).Detach();

    auto trailResolutionInput = Create<NumberParameterInput<int>>(NumberParameterInputParams(
        L"Trail resolution", L"How many points to calculate between each cursor position. Higher values make the trail smoother. Might impact performance",
        _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_RESOLUTION, Config::ADD_IF_MISSING),
        1, 100, 1
    ));
    trailResolutionInput->GetInput()->size.Assign(Width(70));
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

std::unique_ptr<zcom::FlexPanel> zcom::CursorTrailSetupComponent::_SetUpColorsPanel()
{
    auto colorsPanel = Create<FlexPanel>(FlexDirection::DOWN);
    colorsPanel->parentSize = { 1.0f, 0.0f };
    colorsPanel->autoHeight = true;
    colorsPanel->padding = { 0, 0, 0, 15 };

    auto headColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    headColorRow->parentSize = { 1.0f, 0.0f };
    headColorRow->autoHeight = true;
    headColorRow->spacing = 10;
    headColorRow->padding = { 15, 0, 15, 10 };


    _headColorInput = Create<ColorSelector>();
    _headColorInput->UseConfigValue(CursorTrailConfig::HEAD_COLOR);
    _headColorInput->size = { 60, 26 };
    _headColorInput->border.cornerRadius = 2.0f;
    //_headColorInput->SubscribeOnLeftReleased([=](Component*, std::optional<Point>) {
    //    if (!_headColorSelectorWindow.open_)
    //    {
    //        _OpenColorSelector(L"Bar color", CursorTrailConfig::HEAD_COLOR_SELECTOR_WINDOW_NAME, CursorTrailConfig::HEAD_COLOR);
    //    }
    //    else
    //    {
    //        Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_headColorSelectorWindow.id_->value());
    //        if (handle.Valid())
    //            handle->Backend().Focus();
    //    }
    //}).Detach();
    auto headColorLabel = Create<Label>(L"Trail head color");
    headColorLabel->size = { 0, 26 };
    headColorLabel->yTextAlign = Alignment::CENTER;
    headColorLabel->SetProperty(FlexGrow());
    headColorRow->AddItem(_headColorInput.get());
    headColorRow->AddItem(std::move(headColorLabel));

    auto trailColorsLabel = Create<Label>(L"Trail customization");
    trailColorsLabel->size = { 160, 20 };
    trailColorsLabel->position = { 15, 0 };
    auto trailColorsSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(trailColorsSeparator.get(), 15);
    trailColorsSeparator->backgroundColor = Color(0x383838);
    trailColorsSeparator->SetProperty(FlexMarginBefore(-4));
    trailColorsSeparator->SetProperty(FlexMarginAfter(4));

    _paletteSlider = Create<Slider>();
    _paletteSlider->parentSize = { 1.0f, 0.0f };
    _paletteSlider->size = { 0, 30 };
    _paletteSlider->bodyStartOffset = 15;
    _paletteSlider->bodyEndOffset = 15;
    _paletteSlider->interactionAreaMargins = { 10, 4, 10, 4 };
    _paletteSlider->anchorOffset = -2;
    _paletteSlider->SetProperty(Shadow());
    _paletteSlider->eatScrollEvents = true;

    auto paletteSliderBody = Create<Dummy>();
    paletteSliderBody->parentSize = { 1.0f, 0.0f };
    paletteSliderBody->size = { -30, 18 };
    paletteSliderBody->xAlign = Alignment::CENTER;
    paletteSliderBody->yAlign = Alignment::CENTER;
    paletteSliderBody->SubscribePostDraw([=](Component* item, Graphics* g) {
        auto patternBrush = _CreateCheckeredPatternBrush(g, D2D1::ColorF(0x404040));
        if (patternBrush)
        {
            auto rect = g->GetTargetSourceClipRect().ToRectF();
            D2D1_ROUNDED_RECT rrect = D2D1::RoundedRect(RectFToD2D1_RECT_F(rect), 3.0f, 3.0f);
            patternBrush->SetTransform(D2D1::Matrix3x2F::Translation(rect.left, rect.top));
            g->GetRenderContext()->Clear(D2D1::ColorF(0x303030));
            g->GetRenderContext()->FillRoundedRectangle(rrect, patternBrush);
            patternBrush->Release();
        }
        else
        {
            // TODO: Logging
        }

        LinearGradient gradient{};
        gradient.startPosition = { 0.0f, 0.0f };
        gradient.endPosition = { (float)item->size_->width, 0.0f };
        for (auto& stop : _currentPalette)
            gradient.gradientStops.push_back({ stop.position, stop.color });
        RoundedRect rrect = { 3.0f, 3.0f, item->size_->ToRect().ToRectF() };
        g->FillRoundedRectangle(rrect, gradient);
    }).Detach();
    auto paletteSliderAnchor = Create<Dummy>();
    paletteSliderAnchor->size = { 5, 24 };
    paletteSliderAnchor->yAlign = Alignment::CENTER;
    paletteSliderAnchor->SubscribePostDraw([=](Component* item, Graphics* g) {
        RectF itemRect = item->size_->ToRect().ToRectF();
        if (_paletteSliderHovered)
        {
            g->DrawBitmap(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_top_hovered").value(), RectF{ itemRect.left, itemRect.top, itemRect.right, itemRect.top + 6 });
            g->DrawBitmap(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_bottom_hovered").value(), RectF{ itemRect.left, itemRect.bottom - 6, itemRect.right, itemRect.bottom });
        }
        else
        {
            g->DrawBitmap(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_top").value(), RectF{ itemRect.left, itemRect.top, itemRect.right, itemRect.top + 6 });
            g->DrawBitmap(_scene->GetWindow()->resourceManager.GetImage("slider_anchor_bottom").value(), RectF{ itemRect.left, itemRect.bottom - 6, itemRect.right, itemRect.bottom });
        }
    }).Detach();
    _paletteSlider->insideInteractionArea_.Subscribe([=](bool inside) {
        _paletteSliderHovered = inside;
        _paletteSlider->GetAnchorComponent()->InvokeRedraw();
    }).Detach();
    _paletteSlider->SubscribeOnValueChanged([=](Slider*, float* value) {
        _positionInput->value = NumberInputValue(*value);
        _OnColorPositionChanged(*value);
    }).Detach();
    _paletteSlider->SubscribeOnWheelUp([=](Component*, Point) {
        _positionInput->StepUp();
        float newValue = (float)_positionInput->value->getAsDouble();
        _paletteSlider->value = newValue;
        _OnColorPositionChanged(newValue);
    }).Detach();
    _paletteSlider->SubscribeOnWheelDown([=](Component*, Point) {
        _positionInput->StepDown();
        float newValue = (float)_positionInput->value->getAsDouble();
        _paletteSlider->value = newValue;
        _OnColorPositionChanged(newValue);
    }).Detach();
    _paletteSlider->SetBodyComponent(std::move(paletteSliderBody));
    _paletteSlider->SetAnchorComponent(std::move(paletteSliderAnchor));

    auto gradientStopCustomizationPanel = Create<FlexPanel>(FlexDirection::DOWN);
    gradientStopCustomizationPanel->parentSize = { 1.0f, 0.0f };
    gradientStopCustomizationPanel->size = { -30, 0 };
    gradientStopCustomizationPanel->autoHeight = true;
    gradientStopCustomizationPanel->spacing = 5;
    gradientStopCustomizationPanel->padding = { 0, 10, 0, 10 };
    gradientStopCustomizationPanel->xAlign = Alignment::CENTER;
    gradientStopCustomizationPanel->border.cornerRadius = 3.0f;
    gradientStopCustomizationPanel->backgroundColor = Color(0x282828);
    gradientStopCustomizationPanel->SetProperty(Shadow());
    gradientStopCustomizationPanel->SetProperty(FlexMarginBefore(-2));
    gradientStopCustomizationPanel->zIndex = 0;

    _redColorInputGroup = Create<ColorSliderInputGroup>(L"Red:");
    _redColorInputGroup->parentSize = { 1.0f, 0.0f };
    _redColorInputGroup->autoHeight = true;
    _redColorInputGroup->padding = { 10, 0, 10, 0 };
    _redColorInputGroup->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics* g) {
        LinearGradient gradient{};
        gradient.startPosition = { 0.0f, 0.0f };
        gradient.endPosition = { (float)item->size_->width, 0.0f };
        gradient.gradientStops.push_back({ 0.0f, _currentColor.WithR(0).WithA(255) });
        gradient.gradientStops.push_back({ 1.0f, _currentColor.WithR(255).WithA(255) });
        g->FillRectangle(item->size_->ToRect().ToRectF(), gradient);
    }).Detach();
    _redColorInputGroup->value.Subscribe([=](uint8_t value) {
        _currentColor.r = value;
        _OnColorChanged();
    }).Detach();
    _greenColorInputGroup = Create<ColorSliderInputGroup>(L"Green:");
    _greenColorInputGroup->parentSize = { 1.0f, 0.0f };
    _greenColorInputGroup->autoHeight = true;
    _greenColorInputGroup->padding = { 10, 0, 10, 0 };
    _greenColorInputGroup->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics* g) {
        LinearGradient gradient{};
        gradient.startPosition = { 0.0f, 0.0f };
        gradient.endPosition = { (float)item->size_->width, 0.0f };
        gradient.gradientStops.push_back({ 0.0f, _currentColor.WithG(0).WithA(255) });
        gradient.gradientStops.push_back({ 1.0f, _currentColor.WithG(255).WithA(255) });
        g->FillRectangle(item->size_->ToRect().ToRectF(), gradient);
    }).Detach();
    _greenColorInputGroup->value.Subscribe([=](uint8_t value) {
        _currentColor.g = value;
        _OnColorChanged();
    }).Detach();
    _blueColorInputGroup = Create<ColorSliderInputGroup>(L"Blue:");
    _blueColorInputGroup->parentSize = { 1.0f, 0.0f };
    _blueColorInputGroup->autoHeight = true;
    _blueColorInputGroup->padding = { 10, 0, 10, 0 };
    _blueColorInputGroup->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics* g) {
        LinearGradient gradient{};
        gradient.startPosition = { 0.0f, 0.0f };
        gradient.endPosition = { (float)item->size_->width, 0.0f };
        gradient.gradientStops.push_back({ 0.0f, _currentColor.WithB(0).WithA(255) });
        gradient.gradientStops.push_back({ 1.0f, _currentColor.WithB(255).WithA(255) });
        g->FillRectangle(item->size_->ToRect().ToRectF(), gradient);
    }).Detach();
    _blueColorInputGroup->value.Subscribe([=](uint8_t value) {
        _currentColor.b = value;
        _OnColorChanged();
    }).Detach();
    _opacityInputGroup = Create<ColorSliderInputGroup>(L"Opacity:");
    _opacityInputGroup->parentSize = { 1.0f, 0.0f };
    _opacityInputGroup->autoHeight = true;
    _opacityInputGroup->padding = { 10, 0, 10, 0 };
    _opacityInputGroup->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics* g) {
        auto rect = g->GetTargetSourceClipRect().ToRectF();

        auto patternBrush = _CreateCheckeredPatternBrush(g, D2D1::ColorF(0x404040));
        if (patternBrush)
        {
            patternBrush->SetTransform(D2D1::Matrix3x2F::Translation(rect.left, rect.top));
            g->GetRenderContext()->Clear(D2D1::ColorF(0x303030));
            g->GetRenderContext()->FillRectangle(RectFToD2D1_RECT_F(rect), patternBrush);
            patternBrush->Release();
        }
        else
        {
            // TODO: Logging
        }

        LinearGradient gradient{};
        gradient.startPosition = { 0.0f, 0.0f };
        gradient.endPosition = { (float)item->size_->width, 0.0f };
        gradient.gradientStops.push_back({ 0.0f, _currentColor.WithA(0) });
        gradient.gradientStops.push_back({ 1.0f, _currentColor.WithA(255) });
        g->FillRectangle(item->size_->ToRect().ToRectF(), gradient);
    }).Detach();
    _opacityInputGroup->value.Subscribe([=](uint8_t value) {
        _currentColor.a = value;
        _OnColorChanged();
    }).Detach();
    auto positionRow = Create<FlexPanel>(FlexDirection::RIGHT);
    positionRow->parentSize = { 1.0f, 0.0f };
    positionRow->autoHeight = true;
    positionRow->padding = { 10, 0, 10, 0 };
    auto positionLabel = Create<Label>(L"Position:");
    positionLabel->size = { 0, 26 };
    positionLabel->SetProperty(FlexGrow());
    positionLabel->yTextAlign = Alignment::CENTER;
    _positionInput = Create<NumberInput>();
    _positionInput->size = { 70, 26 };
    _positionInput->value = NumberInputValue(0);
    _positionInput->minValue = NumberInputValue("0");
    _positionInput->maxValue = NumberInputValue("1");
    _positionInput->stepSize = NumberInputValue("0.01");
    _positionInput->precision = 3;
    _positionInput->backgroundColor = Color(0x101010);
    _positionInput->border.cornerRadius = 2.0f;
    _positionInput->SubscribeOnValueChanged([=](NumberInputValue value) {
        _paletteSlider->value = (float)value.getAsDouble();
        _OnColorPositionChanged((float)value.getAsDouble());
    }).Detach();
    positionRow->AddItem(std::move(positionLabel));
    positionRow->AddItem(_positionInput.get());

    gradientStopCustomizationPanel->AddItem(_redColorInputGroup.get());
    gradientStopCustomizationPanel->AddItem(_greenColorInputGroup.get());
    gradientStopCustomizationPanel->AddItem(_blueColorInputGroup.get());
    gradientStopCustomizationPanel->AddItem(_opacityInputGroup.get());
    gradientStopCustomizationPanel->AddItem(std::move(positionRow));

    _gradientStopPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _gradientStopPanel->parentSize = { 1.0f, 0.0f };
    _gradientStopPanel->size = { -30 };
    _gradientStopPanel->autoHeight = true;
    _gradientStopPanel->xAlign = Alignment::CENTER;
    _gradientStopPanel->SetProperty(FlexMarginBefore(-6));
    _gradientStopPanel->padding = { 1, 7, 1, 1 };
    _gradientStopPanel->border.cornerRadius = 3.0f;
    _gradientStopPanel->backgroundColor = Color(0x101010);

    auto presetRow = Create<FlexPanel>(FlexDirection::RIGHT);
    presetRow->parentSize = { 1.0f, 0.0f };
    presetRow->autoHeight = true;
    presetRow->spacing = 5;
    presetRow->padding = { 15, 0, 15, 0 };
    presetRow->itemAlignment = Alignment::CENTER;
    presetRow->SetProperty(FlexMarginBefore(5));
    auto presetLabel = Create<Label>(L"Presets:");
    presetLabel->size = { 0, 26 };
    presetLabel->yTextAlign = Alignment::CENTER;
    presetLabel->SetProperty(FlexGrow());
    auto rainbowPresetButton = Create<Button>();
    NeutralButtonStyle::Apply(rainbowPresetButton.get());
    rainbowPresetButton->size = { 20, 20 };
    rainbowPresetButton->SubscribePreContentDraw([=](Component* item, Graphics* g) {
        LinearGradient gradient{};
        gradient.startPosition = { 0.0f, 0.0f };
        gradient.endPosition = { (float)item->size_->width, 0.0f };
        gradient.gradientStops.push_back({ (1.0f / 6.0f) * 0, Color(0xFF0000) });
        gradient.gradientStops.push_back({ (1.0f / 6.0f) * 1, Color(0xFFFF00) });
        gradient.gradientStops.push_back({ (1.0f / 6.0f) * 2, Color(0x00FF00) });
        gradient.gradientStops.push_back({ (1.0f / 6.0f) * 3, Color(0x00FFFF) });
        gradient.gradientStops.push_back({ (1.0f / 6.0f) * 4, Color(0x0000FF) });
        gradient.gradientStops.push_back({ (1.0f / 6.0f) * 5, Color(0xFF00FF) });
        gradient.gradientStops.push_back({ (1.0f / 6.0f) * 6, Color(0xFF0000) });
        g->FillRectangle(item->size_->ToRect().ToRectF(), gradient);
    }).Detach();
    rainbowPresetButton->SubscribeOnActivated([=]() {
        ExecuteSynchronously([=]() {
            _currentPalette.clear();
            _currentPalette.push_back({ Color(0xFF0000), (1.0f / 6.0f) * 0 });
            _currentPalette.push_back({ Color(0xFFFF00), (1.0f / 6.0f) * 1 });
            _currentPalette.push_back({ Color(0x00FF00), (1.0f / 6.0f) * 2 });
            _currentPalette.push_back({ Color(0x00FFFF), (1.0f / 6.0f) * 3 });
            _currentPalette.push_back({ Color(0x0000FF), (1.0f / 6.0f) * 4 });
            _currentPalette.push_back({ Color(0xFF00FF), (1.0f / 6.0f) * 5 });
            _currentPalette.push_back({ Color(0xFF0000), (1.0f / 6.0f) * 6 });
            _BuildItemsForPalette();
            _OnGradientStopSelected(0);
            _ReorderColorList();
            _SavePalette();
        });
    }).Detach();
    auto cyanMagentaPresetButton = Create<Button>();
    NeutralButtonStyle::Apply(cyanMagentaPresetButton.get());
    cyanMagentaPresetButton->size = { 20, 20 };
    cyanMagentaPresetButton->SubscribePreContentDraw([=](Component* item, Graphics* g) {
        LinearGradient gradient{};
        gradient.startPosition = { 0.0f, 0.0f };
        gradient.endPosition = { (float)item->size_->width, 0.0f };
        gradient.gradientStops.push_back({ 0.0f, Color(0x00F0F0) });
        gradient.gradientStops.push_back({ 0.5f, Color(0xF000F0) });
        gradient.gradientStops.push_back({ 1.0f, Color(0x00F0F0) });
        g->FillRectangle(item->size_->ToRect().ToRectF(), gradient);
    }).Detach();
    cyanMagentaPresetButton->SubscribeOnActivated([=]() {
        ExecuteSynchronously([=]() {
            _currentPalette.clear();
            _currentPalette.push_back({ Color(0x00F0F0), 0.0f });
            _currentPalette.push_back({ Color(0xF000F0), 0.5f });
            _currentPalette.push_back({ Color(0x00F0F0), 1.0f });
            _BuildItemsForPalette();
            _OnGradientStopSelected(0);
            _ReorderColorList();
            _SavePalette();
        });
    }).Detach();
    presetRow->AddItem(std::move(presetLabel));
    presetRow->AddItem(std::move(rainbowPresetButton));
    presetRow->AddItem(std::move(cyanMagentaPresetButton));

    colorsPanel->AddItem(std::move(Create<SectionHeader>(L"Colors", Rect{ 15, 0, 15, 0 })));
    colorsPanel->AddItem(std::move(headColorRow));
    colorsPanel->AddItem(std::move(trailColorsLabel));
    colorsPanel->AddItem(std::move(trailColorsSeparator));
    colorsPanel->AddItem(_paletteSlider.get());
    colorsPanel->AddItem(std::move(gradientStopCustomizationPanel));
    colorsPanel->AddItem(_gradientStopPanel.get());
    colorsPanel->AddItem(std::move(presetRow));
    return colorsPanel;
}

ID2D1ImageBrush* zcom::CursorTrailSetupComponent::_CreateCheckeredPatternBrush(Graphics* g, D2D1_COLOR_F cellColor)
{
    float cellSize = 6.0f;

    ComPtr<ID2D1CommandList> patternCommandList = nullptr;
    g->GetRenderContext()->CreateCommandList(patternCommandList.GetAddressOf());
    if (!patternCommandList)
    {
        // TODO: Logging
        return nullptr;
    }

    ComPtr<ID2D1SolidColorBrush> cellBrush = nullptr;
    g->GetRenderContext()->CreateSolidColorBrush(cellColor, cellBrush.GetAddressOf());
    if (!cellBrush)
    {
        // TODO: Logging
        return nullptr;
    }

    ID2D1Image* stash = nullptr;
    g->GetRenderContext()->GetTarget(&stash);
    g->GetRenderContext()->SetTarget(patternCommandList.Get());
    //D2D1_RECT_F targetRect = g.GetTargetRect();
    //g.target->FillRectangle(D2D1::RectF(targetRect.left + 0.0f, targetRect.top + 0.0f, targetRect.left + cellSize, targetRect.top + cellSize), cellBrush.Get());
    //g.target->FillRectangle(D2D1::RectF(targetRect.left + cellSize, targetRect.top + cellSize, targetRect.left + cellSize * 2, targetRect.top + cellSize * 2), cellBrush.Get());
    g->GetRenderContext()->FillRectangle(D2D1::RectF(0.0f, 0.0f, cellSize, cellSize), cellBrush.Get());
    g->GetRenderContext()->FillRectangle(D2D1::RectF(cellSize, cellSize, cellSize * 2, cellSize * 2), cellBrush.Get());
    g->GetRenderContext()->SetTarget(stash);
    stash->Release();

    patternCommandList->Close();

    ID2D1ImageBrush* patternBrush = nullptr;
    g->GetRenderContext()->CreateImageBrush(
        patternCommandList.Get(),
        D2D1::ImageBrushProperties(
            //D2D1::RectF(targetRect.left, targetRect.top, targetRect.left + cellSize * 2, targetRect.top + cellSize * 2),
            D2D1::RectF(0.0f, 0.0f, cellSize * 2, cellSize * 2),
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

void zcom::CursorTrailSetupComponent::_BuildItemsForPalette()
{
    for (auto& stop : _currentPalette)
        _CreateInnerGradientStopComponents(stop);
}

void zcom::CursorTrailSetupComponent::_CreateInnerGradientStopComponents(_GradientStop& stop)
{
    stop.selectorItem = Create<FlexPanel>(FlexDirection::RIGHT);
    stop.selectorItem->parentSize = { 1.0f, 0.0f };
    stop.selectorItem->size = { 0, 30 };
    stop.selectorItem->padding = { 13, 3, 3, 3 };
    stop.selectorItem->spacing = 4;
    stop.selectorItem->AddTag("color_selector");
    stop.colorIndicator = Create<Dummy>();
    stop.colorIndicator->size = { 12, 0 };
    stop.colorIndicator->parentSize = { 0.0f, 1.0f };
    stop.colorIndicator->yAlign = Alignment::CENTER;
    stop.colorIndicator->backgroundColor = stop.color;
    stop.colorIndicator->border.cornerRadius = 3.0f;
    stop.colorIndicator->SetProperty(FlexMarginAfter(4));
    stop.redLabel = Create<Label>(std::to_wstring(stop.color.r));
    stop.redLabel->parentSize = { 0.0f, 1.0f };
    stop.redLabel->size = { 24, 0 };
    stop.redLabel->yTextAlign = Alignment::CENTER;
    stop.redLabel->xTextAlign = TextAlignment::CENTER;
    stop.redLabel->fontColor = Color(0xA0A0A0);
    stop.redUnderline = Create<Dummy>();
    stop.redUnderline->size = { 24, 1 };
    stop.redUnderline->backgroundColor = Color(0xA00000);
    stop.redUnderline->yAlign = Alignment::END;
    stop.redUnderline->SetProperty(FlexMarginBefore(-24 - 4));
    stop.greenLabel = Create<Label>(std::to_wstring(stop.color.g));
    stop.greenLabel->parentSize = { 0.0f, 1.0f };
    stop.greenLabel->size = { 24, 0 };
    stop.greenLabel->yTextAlign = Alignment::CENTER;
    stop.greenLabel->xTextAlign = TextAlignment::CENTER;
    stop.greenLabel->fontColor = Color(0xA0A0A0);
    stop.greenUnderline = Create<Dummy>();
    stop.greenUnderline->size = { 24, 1 };
    stop.greenUnderline->backgroundColor = Color(0x00A000);
    stop.greenUnderline->yAlign = Alignment::END;
    stop.greenUnderline->SetProperty(FlexMarginBefore(-24 - 4));
    stop.blueLabel = Create<Label>(std::to_wstring(stop.color.b));
    stop.blueLabel->parentSize = { 0.0f, 1.0f };
    stop.blueLabel->size = { 24, 0 };
    stop.blueLabel->yTextAlign = Alignment::CENTER;
    stop.blueLabel->xTextAlign = TextAlignment::CENTER;
    stop.blueLabel->fontColor = Color(0xA0A0A0);
    stop.blueUnderline = Create<Dummy>();
    stop.blueUnderline->size = { 24, 1 };
    stop.blueUnderline->backgroundColor = Color(0x0000A0);
    stop.blueUnderline->yAlign = Alignment::END;
    stop.blueUnderline->SetProperty(FlexMarginBefore(-24 - 4));
    stop.opacityLabel = Create<Label>(std::to_wstring(stop.color.a));
    stop.opacityLabel->parentSize = { 0.0f, 1.0f };
    stop.opacityLabel->size = { 24, 0 };
    stop.opacityLabel->yTextAlign = Alignment::CENTER;
    stop.opacityLabel->xTextAlign = TextAlignment::CENTER;
    stop.opacityLabel->fontColor = Color(0xA0A0A0);
    stop.opacityUnderline = Create<Dummy>();
    stop.opacityUnderline->size = { 24, 1 };
    stop.opacityUnderline->backgroundColor = Color(0xA0A0A0);
    stop.opacityUnderline->yAlign = Alignment::END;
    stop.opacityUnderline->SetProperty(FlexMarginBefore(-24 - 4));
    stop.positionLabel = Create<Label>((std::wostringstream() << std::fixed << std::setprecision(3) << stop.position).str());
    stop.positionLabel->parentSize = { 0.0f, 1.0f };
    stop.positionLabel->size = { 40, 0 };
    stop.positionLabel->yTextAlign = Alignment::CENTER;
    stop.positionLabel->xTextAlign = TextAlignment::CENTER;
    stop.positionLabel->fontColor = Color(0xA0A0A0);
    auto spacer = Create<Dummy>();
    spacer->size = { 0, 0 };
    spacer->SetProperty(FlexGrow());
    stop.removeButton = Create<Button>();
    RoundedLiftedButtonStyle::Apply(stop.removeButton.get());
    stop.removeButton->size = { 20, 20 };
    stop.removeButton->yAlign = Alignment::CENTER;
    stop.removeButton->activation = ButtonActivation::PRESS;
    stop.removeButton->ValueFromButtonState<Color>(stop.removeButton->buttonColor, Color(0x383838), Color(0x484848), Color(0x303030));
    stop.removeButton->Image()->image = _scene->GetWindow()->resourceManager.GetImage("minus_1");
    stop.removeButton->Image()->imagePlacement = ImagePlacement::CENTER;
    stop.removeButton->ValueFromButtonState<Color>(stop.removeButton->Image()->tintColor, Color(0xA0A0A0), Color(0xD0D0D0), Color(0xD0D0D0));

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

void zcom::CursorTrailSetupComponent::_ReorderColorList()
{
    // Deferring layout updates here is not only for performance
    // If no deferring is done, after clearing all items the settings panel will resize and scroll up before re-adding the items, resulting in a jarring jump
    _gradientStopPanel->DeferLayoutUpdates();
    _gradientStopPanel->ClearItems();
    _UpdateColorListBackgrounds();
    for (int i = 0; i < _currentPalette.size(); i++)
    {
        _GradientStop& stop = _currentPalette[i];
        stop.postLeftPressedEventSubscription = stop.selectorItem->SubscribePostLeftPressed([=, removeButton = stop.removeButton.get()](Component* item, std::vector<EventContext::Params> targets, Point) {
            if (!targets.empty() && targets.front().target != removeButton)
                _OnGradientStopSelected(i);
        });
        stop.removeButton->visible = _currentPalette.size() > 1;
        stop.removeEventSubscription = stop.removeButton->SubscribeOnActivated([=]() {
            ExecuteSynchronously([=]() { _OnGradientStopRemoved(i); });
        });
        _gradientStopPanel->AddItem(stop.selectorItem.get());
    }
    _gradientStopPanel->ResumeLayoutUpdates();
}

void zcom::CursorTrailSetupComponent::_UpdateColorListBackgrounds()
{
    for (int i = 0; i < _currentPalette.size(); i++)
    {
        _GradientStop& stop = _currentPalette[i];
        int baseBackgroundColor = _currentColorIndex == i ? 0x004070 : (i % 2 == 0 ? 0x161616 : 0x1D1D1D);
        int hoveredBackgroundColor = _currentColorIndex == i ? 0x004070 : 0x282828;
        stop.selectorItem->backgroundColor = Color(stop.selectorItem->hovered_ ? hoveredBackgroundColor : baseBackgroundColor);
        stop.mouseEnterEventSubscription = stop.selectorItem->SubscribeOnMouseEnter([=](Component* item) { item->backgroundColor = Color(hoveredBackgroundColor); });
        stop.mouseLeaveEventSubscription = stop.selectorItem->SubscribeOnMouseLeave([=](Component* item) { item->backgroundColor = Color(baseBackgroundColor); });
    }
}

void zcom::CursorTrailSetupComponent::_OnGradientStopSelected(int index)
{
    _currentColorIndex = index;
    _currentColor = _currentPalette[index].color;
    _paletteSlider->value = _currentPalette[index].position;
    _positionInput->value = NumberInputValue(_currentPalette[index].position);
    _redColorInputGroup->value = _currentColor.r;
    _greenColorInputGroup->value = _currentColor.g;
    _blueColorInputGroup->value = _currentColor.b;
    _opacityInputGroup->value = _currentColor.a;
    _UpdateColorListBackgrounds();
}

void zcom::CursorTrailSetupComponent::_OnGradientStopRemoved(int index)
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

void zcom::CursorTrailSetupComponent::_OnColorChanged()
{
    _redColorInputGroup->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _greenColorInputGroup->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _blueColorInputGroup->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _opacityInputGroup->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _paletteSlider->GetBodyComponent()->InvokeRedraw();

    auto& changedItem = _currentPalette[_currentColorIndex];
    if (changedItem.color.r != _currentColor.r)
        changedItem.redLabel->text = std::to_wstring(_currentColor.r);
    if (changedItem.color.g != _currentColor.g)
        changedItem.greenLabel->text = std::to_wstring(_currentColor.g);
    if (changedItem.color.b != _currentColor.b)
        changedItem.blueLabel->text = std::to_wstring(_currentColor.b);
    if (changedItem.color.a != _currentColor.a)
        changedItem.opacityLabel->text = std::to_wstring(_currentColor.a);
    changedItem.color = _currentColor;
    changedItem.colorIndicator->backgroundColor = changedItem.color;
    _SavePalette();
}

void zcom::CursorTrailSetupComponent::_OnColorPositionChanged(float position)
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
    _currentPalette[_currentColorIndex].positionLabel->text = (std::wostringstream() << std::fixed << std::setprecision(3) << position).str();
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

void zcom::CursorTrailSetupComponent::_LoadPalette()
{
    _ParsePaletteString(_scene->GetApp()->config.GetConfigValue(CursorTrailConfig::PALETTE, Config::ADD_IF_MISSING));
    if (_currentPalette.empty())
    {
        _ParsePaletteString(CursorTrailConfig::PALETTE.defaultValue);
        _scene->GetApp()->config.SetValue(CursorTrailConfig::PALETTE.name, CursorTrailConfig::PALETTE.defaultValue);
    }
}

void zcom::CursorTrailSetupComponent::_ParsePaletteString(const std::wstring& str)
{
    _currentPalette.clear();
    std::vector<std::wstring> stopStrings;
    split_wstr(str, stopStrings, L'|', true);
    for (auto& stopString : stopStrings)
    {
        std::array<std::wstring, 2> parts;
        split_wstr(stopString, parts, L',');
        _currentPalette.push_back(std::move(_GradientStop{ Color::ARGB(std::stoi(parts[0])), std::stof(parts[1]) }));
    }
    std::sort(_currentPalette.begin(), _currentPalette.end(), [](const _GradientStop& stop1, const _GradientStop& stop2) { return stop1.position < stop2.position; });
}

void zcom::CursorTrailSetupComponent::_SavePalette()
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