#include "App.h"
#include "Scenes/Scene.h"
#include "Scenes/DefaultNonClientAreaScene.h"
#include "Scenes/DefaultTitleBarScene.h"
#include "Scenes/Scene.h"
#include "ComboBarParameterPanel.h"
#include "Components/Base/FlexPanel.h"
#include "ComboBarConfig.h"
#include "ComboBarScene.h"
#include "Helper/StringHelper.h"
#include "Shared/Styles/Styles.h"
#include "Shared/Scenes/ColorSelectorScene.h"

void zcom::ComboBarParameterPanel::Init()
{
    ScrollPanel::Init();

    Handle<zwnd::Window> windowHandle = _scene->GetApp()->FindWindowByClassName(ComboBarConfig::OVERLAY_WINDOW_NAME);
    if (windowHandle.Valid())
        _overlayWindowId = windowHandle->GetWindowId();
    Handle<zwnd::Window> barColorSelectorWindowHandle = _scene->GetApp()->FindWindowByClassName(ComboBarConfig::BAR_COLOR_SELECTOR_WINDOW_NAME);
    if (barColorSelectorWindowHandle.Valid())
        _barColorSelectorWindowId = barColorSelectorWindowHandle->GetWindowId();
    Handle<zwnd::Window> textColorSelectorWindowHandle = _scene->GetApp()->FindWindowByClassName(ComboBarConfig::TEXT_COLOR_SELECTOR_WINDOW_NAME);
    if (textColorSelectorWindowHandle.Valid())
        _textColorSelectorWindowId = textColorSelectorWindowHandle->GetWindowId();
    Handle<zwnd::Window> gridColorSelectorWindowHandle = _scene->GetApp()->FindWindowByClassName(ComboBarConfig::GRID_COLOR_SELECTOR_WINDOW_NAME);
    if (gridColorSelectorWindowHandle.Valid())
        _gridColorSelectorWindowId = gridColorSelectorWindowHandle->GetWindowId();

    _dataProviderConnectionEvent = _scene->GetApp()->dataProvider.SubscribeOnConnectionEvent();
    _dataProviderConnectionEvent->ResetSynchronousHandler([=](osu::DataProvider::ConnectionEvent event) {
        ExecuteSynchronously([=]() {
            if (event == osu::DataProvider::CONNECTION_SUCCESSFUL)
            {
                _enableButton->SetActive(true);
            }
            else
            {
                _enableButton->SetActive(false);
                if (_overlayWindowId)
                {
                    _CloseOverlayWindow();
                    _UpdateButtonsBasedOnOverlayState();
                }
            }
        });
    });
    
    _windowClosedEventSubscription = _scene->GetApp()->SubscribeOnWindowClosed([=](zwnd::WindowId id) {
        ExecuteSynchronously([=] {
            if (_barColorSelectorWindowId.has_value() && _barColorSelectorWindowId.value() == id)
                _barColorSelectorWindowId = std::nullopt;
            if (_textColorSelectorWindowId.has_value() && _textColorSelectorWindowId.value() == id)
                _textColorSelectorWindowId = std::nullopt;
            if (_gridColorSelectorWindowId.has_value() && _gridColorSelectorWindowId.value() == id)
                _gridColorSelectorWindowId = std::nullopt;
        });
    });

    _configChangedEventSubscription = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        if (changes)
        {
            bool colorChanged = changes.value().first == ComboBarConfig::BAR_COLOR.name
                || changes.value().first == ComboBarConfig::TEXT_COLOR.name
                || changes.value().first == ComboBarConfig::GRID_COLOR.name;
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
    auto generalLabel = Create<Label>(L"Combo counter");
    generalLabel->SetBaseHeight(30);
    generalLabel->SetVerticalTextAlignment(Alignment::CENTER);
    generalLabel->SetFontSize(20.0f);
    generalLabel->SetProperty(FlexGrow());
    _enableButton = Create<Button>(L"Enable");
    _enableButton->SetBaseSize(90, 30);
    _enableButton->SetBorderVisibility(false);
    _enableButton->Label()->SetFontColor(D2D1::ColorF(0xEAEAEA));
    _enableButton->SetButtonColor(D2D1::ColorF(0x307020));
    _enableButton->SetButtonHoverColor(D2D1::ColorF(0x309020));
    _enableButton->SetButtonClickColor(D2D1::ColorF(0x308020));
    _enableButton->SetCornerRounding(2.0f);
    _enableButton->SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));
    _enableButton->SetProperty(PROP_Shadow{});
    _enableButton->SetActivation(ButtonActivation::RELEASE);
    _enableButton->SubscribeOnActivated([=]() {
        if (!_overlayWindowId)
            _OpenOverlayWindow();
        else
            _CloseOverlayWindow();
        _UpdateButtonsBasedOnOverlayState();
    }).Detach();
    _enableButton->SetActive(_scene->GetApp()->dataProvider.Ready());
    titleRow->AddItem(std::move(generalLabel));
    titleRow->AddItem(_enableButton.get());

    auto interactionHelpLabel = Create<Label>(L"Hold Ctrl+Shift+Q+W while overlay is enabled to show the border, which can be used to quickly resize and move the overlay");
    interactionHelpLabel->SetParentWidthPercent(1.0f);
    interactionHelpLabel->AutomaticHeight();
    interactionHelpLabel->SetWordWrap(true);
    interactionHelpLabel->SetPadding({ 15, 0, 15, 10 });
    interactionHelpLabel->SetFontStyle(DWRITE_FONT_STYLE_ITALIC);
    interactionHelpLabel->SetHorizontalTextAlignment(TextAlignment::JUSTIFIED);

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
            auto widthRow = Create<FlexPanel>(FlexDirection::RIGHT);
            widthRow->FillContainerWidth();
            widthRow->SetSpacing(10);
            _widthInput = Create<NumberInput>();
            _widthInput->SetBaseSize(70, 26);
            _widthInput->SetValue(NumberInputValue(_scene->GetApp()->config.GetIntConfigValue(ComboBarConfig::INITIAL_WIDTH, Config::ADD_IF_MISSING)));
            _widthInput->SetMinValue(NumberInputValue(300));
            _widthInput->SetMaxValue(NumberInputValue(10000));
            _widthInput->SetStepSize(NumberInputValue(10));
            _widthInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _widthInput->SetCornerRounding(2.0f);
            _widthInput->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(ComboBarConfig::INITIAL_WIDTH.name, value.getAsInteger());
            }).Detach();
            auto widthLabel = Create<Label>(L"Width");
            widthLabel->SetBaseHeight(26);
            widthLabel->SetVerticalTextAlignment(Alignment::CENTER);
            widthLabel->SetProperty(FlexGrow());
            widthLabel->SetHoverText(L"Initial width of the overlay, in pixels");
            widthRow->AddItem(_widthInput.get());
            widthRow->AddItem(std::move(widthLabel));
            sizeCol->AddItem(std::move(widthRow));
        } {
            auto heightRow = Create<FlexPanel>(FlexDirection::RIGHT);
            heightRow->FillContainerWidth();
            heightRow->SetSpacing(10);
            _heightInput = Create<NumberInput>();
            _heightInput->SetBaseSize(70, 26);
            _heightInput->SetValue(NumberInputValue(_scene->GetApp()->config.GetIntConfigValue(ComboBarConfig::INITIAL_HEIGHT, Config::ADD_IF_MISSING)));
            _heightInput->SetMinValue(NumberInputValue(100));
            _heightInput->SetMaxValue(NumberInputValue(10000));
            _heightInput->SetStepSize(NumberInputValue(10));
            _heightInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _heightInput->SetCornerRounding(2.0f);
            _heightInput->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(ComboBarConfig::INITIAL_HEIGHT.name, value.getAsInteger());
            }).Detach();
            auto heightLabel = Create<Label>(L"Height");
            heightLabel->SetBaseHeight(26);
            heightLabel->SetVerticalTextAlignment(Alignment::CENTER);
            heightLabel->SetProperty(FlexGrow());
            heightLabel->SetHoverText(L"Initial height of the overlay, in pixels");
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
            _xOffsetInput->SetValue(NumberInputValue(_scene->GetApp()->config.GetIntConfigValue(ComboBarConfig::INITIAL_X_OFFSET, Config::ADD_IF_MISSING)));
            _xOffsetInput->SetMinValue(NumberInputValue(-10000));
            _xOffsetInput->SetMaxValue(NumberInputValue(10000));
            _xOffsetInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _xOffsetInput->SetCornerRounding(2.0f);
            _xOffsetInput->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(ComboBarConfig::INITIAL_X_OFFSET.name, value.getAsInteger());
            }).Detach();
            auto xOffsetLabel = Create<Label>(L"X offset");
            xOffsetLabel->SetBaseHeight(26);
            xOffsetLabel->SetVerticalTextAlignment(Alignment::CENTER);
            xOffsetLabel->SetProperty(FlexGrow());
            xOffsetLabel->SetHoverText(L"Initial horizontal shift of the overlay, in pixels");
            xOffsetRow->AddItem(_xOffsetInput.get());
            xOffsetRow->AddItem(std::move(xOffsetLabel));
            offsetCol->AddItem(std::move(xOffsetRow));
        } {
            auto yOffsetRow = Create<FlexPanel>(FlexDirection::RIGHT);
            yOffsetRow->FillContainerWidth();
            yOffsetRow->SetSpacing(10);
            _yOffsetInput = Create<NumberInput>();
            _yOffsetInput->SetBaseSize(70, 26);
            _yOffsetInput->SetValue(NumberInputValue(_scene->GetApp()->config.GetIntConfigValue(ComboBarConfig::INITIAL_Y_OFFSET, Config::ADD_IF_MISSING)));
            _yOffsetInput->SetMinValue(NumberInputValue(-10000));
            _yOffsetInput->SetMaxValue(NumberInputValue(10000));
            _yOffsetInput->SetBackgroundColor(D2D1::ColorF(0x101010));
            _yOffsetInput->SetCornerRounding(2.0f);
            _yOffsetInput->SubscribeOnValueChanged([=](NumberInputValue value) {
                _scene->GetApp()->config.SetIntValue(ComboBarConfig::INITIAL_Y_OFFSET.name, value.getAsInteger());
            }).Detach();
            auto yOffsetLabel = Create<Label>(L"Y offset");
            yOffsetLabel->SetBaseHeight(26);
            yOffsetLabel->SetVerticalTextAlignment(Alignment::CENTER);
            yOffsetLabel->SetProperty(FlexGrow());
            yOffsetLabel->SetHoverText(L"Initial vertical shift of the overlay, in pixels");
            yOffsetRow->AddItem(_yOffsetInput.get());
            yOffsetRow->AddItem(std::move(yOffsetLabel));
            offsetCol->AddItem(std::move(yOffsetRow));
        }
        layoutSection->AddItem(std::move(offsetCol));
    }

    auto actionsRow = Create<FlexPanel>(FlexDirection::RIGHT);
    actionsRow->FillContainerWidth();
    actionsRow->SetSpacing(10);
    actionsRow->SetPadding({ 15, 0, 15, 10 });
    actionsRow->SetProperty(PROP_Shadow());
    _applyCurrentButton = Create<Button>(L"Apply to overlay");
    _applyCurrentButton->SetBaseHeight(40);
    _applyCurrentButton->SetProperty(FlexGrow());
    NeutralButtonStyle::Apply(_applyCurrentButton.get());
    _applyCurrentButton->RemoveProperty<PROP_Shadow>();
    _applyCurrentButton->Label()->SetWordWrap(true);
    _applyCurrentButton->Label()->SetPadding({ 5, 0, 5, 0 });
    _applyCurrentButton->SetHoverText(L"Resizes and moves the overlay to match set values");
    _applyCurrentButton->SubscribeOnActivated([=]() {
        if (_overlayWindowId)
        {
            Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_overlayWindowId.value());
            if (handle.Valid())
            {
                int width = (int)_widthInput->GetValue().getAsInteger();
                int height = (int)_heightInput->GetValue().getAsInteger();
                int xOffset = (int)_xOffsetInput->GetValue().getAsInteger();
                int yOffset = (int)_yOffsetInput->GetValue().getAsInteger();
                handle->Backend().SetWindowRectangle(RECT{xOffset, yOffset, xOffset + width, yOffset + height});
            }
        }
    }).Detach();
    _useCurrentButton = Create<Button>(L"Load from overlay");
    _useCurrentButton->SetBaseHeight(40);
    _useCurrentButton->SetProperty(FlexGrow());
    NeutralButtonStyle::Apply(_useCurrentButton.get());
    _useCurrentButton->RemoveProperty<PROP_Shadow>();
    _useCurrentButton->Label()->SetWordWrap(true);
    _useCurrentButton->Label()->SetPadding({ 5, 0, 5, 0 });
    _useCurrentButton->SetHoverText(L"Sets the values to match the current overlay layout");
    _useCurrentButton->SubscribeOnActivated([=]() {
        if (_overlayWindowId)
        {
            Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_overlayWindowId.value());
            if (handle.Valid())
            {
                RECT windowRect = handle->Backend().GetWindowRectangle();
                _widthInput->SetValue(NumberInputValue(int(windowRect.right - windowRect.left)));
                _heightInput->SetValue(NumberInputValue(int(windowRect.bottom - windowRect.top)));
                _xOffsetInput->SetValue(NumberInputValue(int(windowRect.left)));
                _yOffsetInput->SetValue(NumberInputValue(int(windowRect.top)));
            }
        }
    }).Detach();
    auto resetDefaultsButton = Create<Button>(L"Restore defaults");
    resetDefaultsButton->SetBaseHeight(40);
    resetDefaultsButton->SetProperty(FlexGrow());
    NeutralButtonStyle::Apply(resetDefaultsButton.get());
    resetDefaultsButton->RemoveProperty<PROP_Shadow>();
    resetDefaultsButton->SetBackgroundColor(D2D1::ColorF(0x503030));
    resetDefaultsButton->Label()->SetFontColor(D2D1::ColorF(0xEAEAEA));
    resetDefaultsButton->Label()->SetWordWrap(true);
    resetDefaultsButton->Label()->SetPadding({ 5, 0, 5, 0 });
    resetDefaultsButton->SetHoverText(L"Restores values to their defaults");
    resetDefaultsButton->SubscribeOnActivated([=]() {
        _widthInput->SetValue(NumberInputValue(ComboBarConfig::INITIAL_WIDTH.defaultValue));
        _heightInput->SetValue(NumberInputValue(ComboBarConfig::INITIAL_HEIGHT.defaultValue));
        _xOffsetInput->SetValue(NumberInputValue(ComboBarConfig::INITIAL_X_OFFSET.defaultValue));
        _yOffsetInput->SetValue(NumberInputValue(ComboBarConfig::INITIAL_Y_OFFSET.defaultValue));
    }).Detach();
    actionsRow->AddItem(_applyCurrentButton.get());
    actionsRow->AddItem(_useCurrentButton.get());
    actionsRow->AddItem(std::move(resetDefaultsButton));

    generalPanel->AddItem(std::move(titleRow));
    generalPanel->AddItem(std::move(interactionHelpLabel));
    generalPanel->AddItem(std::move(layoutSection));
    generalPanel->AddItem(std::move(actionsRow));
    flexPanel->AddItem(std::move(generalPanel));

    auto appearancePanel = Create<FlexPanel>(FlexDirection::DOWN);
    appearancePanel->FillContainerWidth();

    auto appearanceHeader = Create<FlexPanel>(FlexDirection::RIGHT);
    appearanceHeader->FillContainerWidth();
    appearanceHeader->SetBaseHeight(30);
    appearanceHeader->SetPadding(RECT{ 15, 0, 15, 10 });
    appearanceHeader->SetSpacing(5);
    auto headerLabel = Create<Label>(L"Appearance");
    headerLabel->AutomaticWidth();
    headerLabel->SetBaseHeight(30);
    headerLabel->SetVerticalTextAlignment(Alignment::CENTER);
    headerLabel->SetFontSize(14.0f);
    headerLabel->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
    headerLabel->SetFont(L"Arial");
    auto headerSeparator = Create<Dummy>();
    HorizontalSeparatorStyle::Apply(headerSeparator.get());
    headerSeparator->SetVerticalOffsetPixels(1);
    headerSeparator->SetVerticalAlignment(Alignment::CENTER);
    headerSeparator->SetProperty(FlexShrink());
    appearanceHeader->AddItem(std::move(headerLabel));
    appearanceHeader->AddItem(std::move(headerSeparator));
    
    auto barColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    barColorRow->FillContainerWidth();
    barColorRow->SetSpacing(10);
    barColorRow->SetPadding({ 15, 0, 15, 10 });
    _barColorInput = Create<Dummy>();
    _barColorInput->SetBaseSize(60, 26);
    _barColorInput->SetCornerRounding(2.0f);
    _barColorInput->SubscribeOnLeftReleased([=](Component*, int, int) {
        if (!_barColorSelectorWindowId)
        {
            _barColorSelectorWindowId = _OpenColorSelector(L"Bar color", ComboBarConfig::BAR_COLOR_SELECTOR_WINDOW_NAME, ComboBarConfig::BAR_COLOR);
        }
        else
        {
            Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_barColorSelectorWindowId.value());
            if (handle.Valid())
                handle->Backend().Focus();
        }
    }).Detach();
    auto barColorLabel = Create<Label>(L"Bar color");
    barColorLabel->SetBaseHeight(26);
    barColorLabel->SetVerticalTextAlignment(Alignment::CENTER);
    barColorLabel->SetProperty(FlexGrow());
    barColorRow->AddItem(_barColorInput.get());
    barColorRow->AddItem(std::move(barColorLabel));
    auto textColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    textColorRow->FillContainerWidth();
    textColorRow->SetSpacing(10);
    textColorRow->SetPadding({ 15, 0, 15, 10 });
    _textColorInput = Create<Dummy>();
    _textColorInput->SetBaseSize(60, 26);
    _textColorInput->SetCornerRounding(2.0f);
    _textColorInput->SubscribeOnLeftReleased([=](Component*, int, int) {
        if (!_textColorSelectorWindowId)
        {
            _textColorSelectorWindowId = _OpenColorSelector(L"Text color", ComboBarConfig::TEXT_COLOR_SELECTOR_WINDOW_NAME, ComboBarConfig::TEXT_COLOR);
        }
        else
        {
            Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_textColorSelectorWindowId.value());
            if (handle.Valid())
                handle->Backend().Focus();
        }
    }).Detach();
    auto textColorLabel = Create<Label>(L"Text color");
    textColorLabel->SetBaseHeight(26);
    textColorLabel->SetVerticalTextAlignment(Alignment::CENTER);
    textColorLabel->SetProperty(FlexGrow());
    textColorRow->AddItem(_textColorInput.get());
    textColorRow->AddItem(std::move(textColorLabel));
    auto gridColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    gridColorRow->FillContainerWidth();
    gridColorRow->SetSpacing(10);
    gridColorRow->SetPadding({ 15, 0, 15, 10 });
    _gridColorInput = Create<Dummy>();
    _gridColorInput->SetBaseSize(60, 26);
    _gridColorInput->SetCornerRounding(2.0f);
    _gridColorInput->SubscribeOnLeftReleased([=](Component*, int, int) {
        if (!_gridColorSelectorWindowId)
        {
            _gridColorSelectorWindowId = _OpenColorSelector(L"Grid color", ComboBarConfig::GRID_COLOR_SELECTOR_WINDOW_NAME, ComboBarConfig::GRID_COLOR);
        }
        else
        {
            Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_gridColorSelectorWindowId.value());
            if (handle.Valid())
                handle->Backend().Focus();
        }
    }).Detach();
    auto gridColorLabel = Create<Label>(L"Grid color");
    gridColorLabel->SetBaseHeight(26);
    gridColorLabel->SetVerticalTextAlignment(Alignment::CENTER);
    gridColorLabel->SetProperty(FlexGrow());
    gridColorRow->AddItem(_gridColorInput.get());
    gridColorRow->AddItem(std::move(gridColorLabel));

    appearancePanel->AddItem(std::move(appearanceHeader));
    appearancePanel->AddItem(std::move(barColorRow));
    appearancePanel->AddItem(std::move(textColorRow));
    appearancePanel->AddItem(std::move(gridColorRow));
    flexPanel->AddItem(std::move(appearancePanel));

    AddItem(std::move(flexPanel));

    _UpdateButtonsBasedOnOverlayState();
    _UpdateColorInput();
}

void zcom::ComboBarParameterPanel::_UpdateColorInput()
{
    zutil::Color barColor = zutil::Color(_scene->GetApp()->config.GetIntConfigValue(ComboBarConfig::BAR_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    zutil::Color textColor = zutil::Color(_scene->GetApp()->config.GetIntConfigValue(ComboBarConfig::TEXT_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    zutil::Color gridColor = zutil::Color(_scene->GetApp()->config.GetIntConfigValue(ComboBarConfig::GRID_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _barColorInput->SetBackgroundColor(D2D1::ColorF(barColor.ToIntNoAlpha(), barColor.a / 255.0f));
    _textColorInput->SetBackgroundColor(D2D1::ColorF(textColor.ToIntNoAlpha(), textColor.a / 255.0f));
    _gridColorInput->SetBackgroundColor(D2D1::ColorF(gridColor.ToIntNoAlpha(), gridColor.a / 255.0f));
}

void zcom::ComboBarParameterPanel::_OpenOverlayWindow()
{
    int width = (int)_widthInput->GetValue().getAsInteger();
    int height = (int)_heightInput->GetValue().getAsInteger();
    int xOffset = (int)_xOffsetInput->GetValue().getAsInteger();
    int yOffset = (int)_yOffsetInput->GetValue().getAsInteger();
    zwnd::WindowProperties props = zwnd::WindowProperties()
        .WindowClassName(ComboBarConfig::OVERLAY_WINDOW_NAME)
        .InitialSize(width, height)
        .MinSize(300, 100)
        .InitialOffset(xOffset, yOffset)
        .IgnoreTaskbarForPlacement()
        .TopMost()
        .DisableWindowAnimations()
        .DisableWindowActivation()
        .DisableMouseInteraction()
        .DisableFastTooltips();

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

            wnd->LoadStartingScene<ComboBarScene>(nullptr);
        }
    );
}

void zcom::ComboBarParameterPanel::_CloseOverlayWindow()
{
    Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_overlayWindowId.value());
    if (handle.Valid())
        handle->Close();
    _overlayWindowId = std::nullopt;
}

void zcom::ComboBarParameterPanel::_UpdateButtonsBasedOnOverlayState()
{
    if (_overlayWindowId)
    {
        _enableButton->Label()->SetText(L"Disable");
        DisableButtonStyle::Apply(_enableButton.get());
        _applyCurrentButton->SetActive(true);
        _applyCurrentButton->SetBackgroundColor(D2D1::ColorF(0x303030));
        _useCurrentButton->SetActive(true);
        _useCurrentButton->SetBackgroundColor(D2D1::ColorF(0x303030));
    }
    else
    {
        _enableButton->Label()->SetText(L"Enable");
        EnableButtonStyle::Apply(_enableButton.get());
        _applyCurrentButton->SetActive(false);
        _applyCurrentButton->SetBackgroundColor(D2D1::ColorF(0x505050));
        _useCurrentButton->SetActive(false);
        _useCurrentButton->SetBackgroundColor(D2D1::ColorF(0x505050));
    }
}

std::optional<zwnd::WindowId> zcom::ComboBarParameterPanel::_OpenColorSelector(std::wstring windowTitle, std::wstring wndClass, ConfigValue<int> colorConfigValue)
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