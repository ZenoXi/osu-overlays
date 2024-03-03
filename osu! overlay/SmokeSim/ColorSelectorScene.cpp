#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "ColorSelectorScene.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Dummy.h"
#include "Components/Base/NumberInput.h"
#include "Components/Base/Button.h"

zcom::ColorSelectorScene::ColorSelectorScene(App* app, zwnd::Window* window)
    : Scene(app, window)
{}

void zcom::ColorSelectorScene::_Init(SceneOptionsBase* options)
{
    ColorSelectorSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const ColorSelectorSceneOptions*>(options);
    _optionName = opt.optionName;

    std::optional<int> color = _app->options.GetIntValue(_optionName);
    if (color)
        _initialColor = zutil::Color(color.value());
    else
        _initialColor = zutil::Color(0xFFFFFF, 1.0f);
    _currentColor = _initialColor;

    auto mainPanel = Create<FlexPanel>(FlexDirection::DOWN);
    mainPanel->FillContainerSize();
    mainPanel->SetPadding({ 10, 10, 10, 10 });
    mainPanel->SetSpacing(5);

    auto redRow = Create<FlexPanel>(FlexDirection::RIGHT);
    redRow->FillContainerWidth();
    redRow->SetSpacing(10);
    redRow->SetItemAlignment(Alignment::CENTER);
    auto redLabel = Create<Label>(L"Red:");
    redLabel->SetBaseSize(50, 26);
    redLabel->SetVerticalTextAlignment(Alignment::CENTER);
    redLabel->SetHorizontalTextAlignment(TextAlignment::TRAILING);
    _redSlider = Create<Slider>();
    _redSlider->SetValue(_initialColor.r / 255.0f);
    _redSlider->SetBaseHeight(26);
    _redSlider->SetSliderBodyOffset(2, 2);
    _redSlider->SetInteractionAreaMargins({ 0, 4, 0, 4 });
    _redSlider->SetAnchorOffset(-2);
    _redSlider->SetProperty(FlexGrow());
    auto redSliderBody = Create<Dummy>();
    redSliderBody->SetParentWidthPercent(1.0f);
    redSliderBody->SetBaseSize(-4, 14);
    redSliderBody->SetHorizontalOffsetPixels(2);
    redSliderBody->SetVerticalAlignment(Alignment::CENTER);
    redSliderBody->SubscribePostDraw([=](Component* item, Graphics g) {
        D2D1_COLOR_F startColor = D2D1::ColorF(0.0f, _currentColor.g / 255.0f, _currentColor.b / 255.0f);
        D2D1_COLOR_F endColor = D2D1::ColorF(1.0f, _currentColor.g / 255.0f, _currentColor.b / 255.0f);
        _DrawGradient(item, startColor, endColor, g);
        }).Detach();
        auto redSliderAnchor = Create<Dummy>();
        redSliderAnchor->SetBaseSize(5, 20);
        redSliderAnchor->SetVerticalAlignment(Alignment::CENTER);
        redSliderAnchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor"));
        auto redInput = Create<NumberInput>();
        redInput->SetBaseSize(60, 26);
        redInput->SetValue(NumberInputValue(_initialColor.r));
        redInput->SetMinValue(NumberInputValue(0));
        redInput->SetMaxValue(NumberInputValue(255));
        redInput->SetStepSize(NumberInputValue(3));
        redInput->SetBackgroundColor(D2D1::ColorF(0x101010));
        redInput->SetCornerRounding(2.0f);
        redInput->AddOnValueChanged([=](NumberInputValue value) {
            _redSlider->SetValue(value.getAsInteger() / 255.0f);
            }).Detach();
            _redSlider->SubscribeOnValueChanged([=, redInput = redInput.get()](float* newValue) {
                int redValue = (*newValue) * 255;
                redInput->SetValue(NumberInputValue(redValue));
                _currentColor.r = redValue;
                _OnColorChanged();
                }).Detach();
                _redSlider->SubscribeOnEnterInteractionArea([=, anchor = redSliderAnchor.get()] {
                    anchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor_hovered"));
                    }).Detach();
                    _redSlider->SubscribeOnLeaveInteractionArea([=, anchor = redSliderAnchor.get()] {
                        anchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor"));
                        }).Detach();
                        _redSlider->SubscribeOnWheelUp([=, input = redInput.get()](Component*, int, int) {
                            input->StepUp();
                            }).Detach();
                            _redSlider->SubscribeOnWheelDown([=, input = redInput.get()](Component*, int, int) {
                                input->StepDown();
                                }).Detach();
                                _redSlider->SetBodyComponent(std::move(redSliderBody));
                                _redSlider->SetAnchorComponent(std::move(redSliderAnchor));
                                redRow->AddItem(std::move(redLabel));
                                redRow->AddItem(_redSlider.get());
                                redRow->AddItem(std::move(redInput));

                                auto greenRow = Create<FlexPanel>(FlexDirection::RIGHT);
                                greenRow->FillContainerWidth();
                                greenRow->SetSpacing(10);
                                greenRow->SetItemAlignment(Alignment::CENTER);
                                auto greenLabel = Create<Label>(L"Green:");
                                greenLabel->SetBaseSize(50, 26);
                                greenLabel->SetVerticalTextAlignment(Alignment::CENTER);
                                greenLabel->SetHorizontalTextAlignment(TextAlignment::TRAILING);
                                _greenSlider = Create<Slider>();
                                _greenSlider->SetValue(_initialColor.g / 255.0f);
                                _greenSlider->SetBaseHeight(26);
                                _greenSlider->SetSliderBodyOffset(2, 2);
                                _greenSlider->SetInteractionAreaMargins({ 0, 4, 0, 4 });
                                _greenSlider->SetAnchorOffset(-2);
                                _greenSlider->SetProperty(FlexGrow());
                                auto greenSliderBody = Create<Dummy>();
                                greenSliderBody->SetParentWidthPercent(1.0f);
                                greenSliderBody->SetBaseSize(-4, 14);
                                greenSliderBody->SetHorizontalOffsetPixels(2);
                                greenSliderBody->SetVerticalAlignment(Alignment::CENTER);
                                greenSliderBody->SubscribePostDraw([=](Component* item, Graphics g) {
                                    D2D1_COLOR_F startColor = D2D1::ColorF(_currentColor.r / 255.0f, 0.0f, _currentColor.b / 255.0f);
                                    D2D1_COLOR_F endColor = D2D1::ColorF(_currentColor.r / 255.0f, 1.0f, _currentColor.b / 255.0f);
                                    _DrawGradient(item, startColor, endColor, g);
                                    }).Detach();
                                    auto greenSliderAnchor = Create<Dummy>();
                                    greenSliderAnchor->SetBaseSize(5, 20);
                                    greenSliderAnchor->SetVerticalAlignment(Alignment::CENTER);
                                    greenSliderAnchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor"));
                                    auto greenInput = Create<NumberInput>();
                                    greenInput->SetBaseSize(60, 26);
                                    greenInput->SetValue(NumberInputValue(_initialColor.g));
                                    greenInput->SetMinValue(NumberInputValue(0));
                                    greenInput->SetMaxValue(NumberInputValue(255));
                                    greenInput->SetStepSize(NumberInputValue(3));
                                    greenInput->SetBackgroundColor(D2D1::ColorF(0x101010));
                                    greenInput->SetCornerRounding(2.0f);
                                    greenInput->AddOnValueChanged([=](NumberInputValue value) {
                                        _greenSlider->SetValue(value.getAsInteger() / 255.0f);
                                        }).Detach();
                                        _greenSlider->SubscribeOnValueChanged([=, greenInput = greenInput.get()](float* newValue) {
                                            int greenValue = (*newValue) * 255;
                                            greenInput->SetValue(NumberInputValue(greenValue));
                                            _currentColor.g = greenValue;
                                            _OnColorChanged();
                                            }).Detach();
                                            _greenSlider->SubscribeOnEnterInteractionArea([=, anchor = greenSliderAnchor.get()] {
                                                anchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor_hovered"));
                                                }).Detach();
                                                _greenSlider->SubscribeOnLeaveInteractionArea([=, anchor = greenSliderAnchor.get()] {
                                                    anchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor"));
                                                    }).Detach();
                                                    _greenSlider->SubscribeOnWheelUp([=, input = greenInput.get()](Component*, int, int) {
                                                        input->StepUp();
                                                        }).Detach();
                                                        _greenSlider->SubscribeOnWheelDown([=, input = greenInput.get()](Component*, int, int) {
                                                            input->StepDown();
                                                            }).Detach();
                                                            _greenSlider->SetBodyComponent(std::move(greenSliderBody));
                                                            _greenSlider->SetAnchorComponent(std::move(greenSliderAnchor));
                                                            greenRow->AddItem(std::move(greenLabel));
                                                            greenRow->AddItem(_greenSlider.get());
                                                            greenRow->AddItem(std::move(greenInput));

                                                            auto blueRow = Create<FlexPanel>(FlexDirection::RIGHT);
                                                            blueRow->FillContainerWidth();
                                                            blueRow->SetSpacing(10);
                                                            blueRow->SetItemAlignment(Alignment::CENTER);
                                                            auto blueLabel = Create<Label>(L"Blue:");
                                                            blueLabel->SetBaseSize(50, 26);
                                                            blueLabel->SetVerticalTextAlignment(Alignment::CENTER);
                                                            blueLabel->SetHorizontalTextAlignment(TextAlignment::TRAILING);
                                                            _blueSlider = Create<Slider>();
                                                            _blueSlider->SetValue(_initialColor.b / 255.0f);
                                                            _blueSlider->SetBaseHeight(26);
                                                            _blueSlider->SetSliderBodyOffset(2, 2);
                                                            _blueSlider->SetInteractionAreaMargins({ 0, 4, 0, 4 });
                                                            _blueSlider->SetAnchorOffset(-2);
                                                            _blueSlider->SetProperty(FlexGrow());
                                                            auto blueSliderBody = Create<Dummy>();
                                                            blueSliderBody->SetParentWidthPercent(1.0f);
                                                            blueSliderBody->SetBaseSize(-4, 14);
                                                            blueSliderBody->SetHorizontalOffsetPixels(2);
                                                            blueSliderBody->SetVerticalAlignment(Alignment::CENTER);
                                                            blueSliderBody->SubscribePostDraw([=](Component* item, Graphics g) {
                                                                D2D1_COLOR_F startColor = D2D1::ColorF(_currentColor.r / 255.0f, _currentColor.g / 255.0f, 0.0f);
                                                                D2D1_COLOR_F endColor = D2D1::ColorF(_currentColor.r / 255.0f, _currentColor.g / 255.0f, 1.0f);
                                                                _DrawGradient(item, startColor, endColor, g);
                                                                }).Detach();
                                                                auto blueSliderAnchor = Create<Dummy>();
                                                                blueSliderAnchor->SetBaseSize(5, 20);
                                                                blueSliderAnchor->SetVerticalAlignment(Alignment::CENTER);
                                                                blueSliderAnchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor"));
                                                                auto blueInput = Create<NumberInput>();
                                                                blueInput->SetBaseSize(60, 26);
                                                                blueInput->SetValue(NumberInputValue(_initialColor.b));
                                                                blueInput->SetMinValue(NumberInputValue(0));
                                                                blueInput->SetMaxValue(NumberInputValue(255));
                                                                blueInput->SetStepSize(NumberInputValue(3));
                                                                blueInput->SetBackgroundColor(D2D1::ColorF(0x101010));
                                                                blueInput->SetCornerRounding(2.0f);
                                                                blueInput->AddOnValueChanged([=](NumberInputValue value) {
                                                                    _blueSlider->SetValue(value.getAsInteger() / 255.0f);
                                                                    }).Detach();
                                                                    _blueSlider->SubscribeOnValueChanged([=, blueInput = blueInput.get()](float* newValue) {
                                                                        int blueValue = (*newValue) * 255;
                                                                        blueInput->SetValue(NumberInputValue(blueValue));
                                                                        _currentColor.b = blueValue;
                                                                        _OnColorChanged();
                                                                        }).Detach();
                                                                        _blueSlider->SubscribeOnEnterInteractionArea([=, anchor = blueSliderAnchor.get()] {
                                                                            anchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor_hovered"));
                                                                            }).Detach();
                                                                            _blueSlider->SubscribeOnLeaveInteractionArea([=, anchor = blueSliderAnchor.get()] {
                                                                                anchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor"));
                                                                                }).Detach();
                                                                                _blueSlider->SubscribeOnWheelUp([=, input = blueInput.get()](Component*, int, int) {
                                                                                    input->StepUp();
                                                                                    }).Detach();
                                                                                    _blueSlider->SubscribeOnWheelDown([=, input = blueInput.get()](Component*, int, int) {
                                                                                        input->StepDown();
                                                                                        }).Detach();
                                                                                        _blueSlider->SetBodyComponent(std::move(blueSliderBody));
                                                                                        _blueSlider->SetAnchorComponent(std::move(blueSliderAnchor));
                                                                                        blueRow->AddItem(std::move(blueLabel));
                                                                                        blueRow->AddItem(_blueSlider.get());
                                                                                        blueRow->AddItem(std::move(blueInput));

                                                                                        auto opacityRow = Create<FlexPanel>(FlexDirection::RIGHT);
                                                                                        opacityRow->FillContainerWidth();
                                                                                        opacityRow->SetSpacing(10);
                                                                                        opacityRow->SetItemAlignment(Alignment::CENTER);
                                                                                        auto opacityLabel = Create<Label>(L"Opacity:");
                                                                                        opacityLabel->SetBaseSize(50, 26);
                                                                                        opacityLabel->SetVerticalTextAlignment(Alignment::CENTER);
                                                                                        opacityLabel->SetHorizontalTextAlignment(TextAlignment::TRAILING);
                                                                                        _opacitySlider = Create<Slider>();
                                                                                        _opacitySlider->SetValue(_initialColor.a / 255.0f);
                                                                                        _opacitySlider->SetBaseHeight(26);
                                                                                        _opacitySlider->SetSliderBodyOffset(2, 2);
                                                                                        _opacitySlider->SetInteractionAreaMargins({ 0, 4, 0, 4 });
                                                                                        _opacitySlider->SetAnchorOffset(-2);
                                                                                        _opacitySlider->SetProperty(FlexGrow());
                                                                                        auto opacitySliderBody = Create<Dummy>();
                                                                                        opacitySliderBody->SetParentWidthPercent(1.0f);
                                                                                        opacitySliderBody->SetBaseSize(-4, 14);
                                                                                        opacitySliderBody->SetHorizontalOffsetPixels(2);
                                                                                        opacitySliderBody->SetVerticalAlignment(Alignment::CENTER);
                                                                                        opacitySliderBody->SubscribePostDraw([=](Component* item, Graphics g) {
                                                                                            D2D1_COLOR_F cellColor1 = D2D1::ColorF(0x404040);
                                                                                            D2D1_COLOR_F cellColor2 = D2D1::ColorF(0x303030);
                                                                                            _DrawCheckeredPattern(item, cellColor1, cellColor2, g);
                                                                                            D2D1_COLOR_F startColor = D2D1::ColorF(_currentColor.r / 255.0f, _currentColor.g / 255.0f, _currentColor.b / 255.0f, 0.0f);
                                                                                            D2D1_COLOR_F endColor = D2D1::ColorF(_currentColor.r / 255.0f, _currentColor.g / 255.0f, _currentColor.b / 255.0f, 1.0f);
                                                                                            _DrawGradient(item, startColor, endColor, g);
                                                                                            }).Detach();
                                                                                            auto opacitySliderAnchor = Create<Dummy>();
                                                                                            opacitySliderAnchor->SetBaseSize(5, 20);
                                                                                            opacitySliderAnchor->SetVerticalAlignment(Alignment::CENTER);
                                                                                            opacitySliderAnchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor"));
                                                                                            auto opacityInput = Create<NumberInput>();
                                                                                            opacityInput->SetBaseSize(60, 26);
                                                                                            opacityInput->SetValue(NumberInputValue(_initialColor.a));
                                                                                            opacityInput->SetMinValue(NumberInputValue(0));
                                                                                            opacityInput->SetMaxValue(NumberInputValue(255));
                                                                                            opacityInput->SetStepSize(NumberInputValue(3));
                                                                                            opacityInput->SetBackgroundColor(D2D1::ColorF(0x101010));
                                                                                            opacityInput->SetCornerRounding(2.0f);
                                                                                            opacityInput->AddOnValueChanged([=](NumberInputValue value) {
                                                                                                _opacitySlider->SetValue(value.getAsInteger() / 255.0f);
                                                                                                }).Detach();
                                                                                                _opacitySlider->SubscribeOnValueChanged([=, opacityInput = opacityInput.get()](float* newValue) {
                                                                                                    int opacityValue = (*newValue) * 255;
                                                                                                    opacityInput->SetValue(NumberInputValue(opacityValue));
                                                                                                    _currentColor.a = opacityValue;
                                                                                                    _OnColorChanged();
                                                                                                    }).Detach();
                                                                                                    _opacitySlider->SubscribeOnEnterInteractionArea([=, anchor = opacitySliderAnchor.get()] {
                                                                                                        anchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor_hovered"));
                                                                                                        }).Detach();
                                                                                                        _opacitySlider->SubscribeOnLeaveInteractionArea([=, anchor = opacitySliderAnchor.get()] {
                                                                                                            anchor->SetBackgroundImage(_window->resourceManager.GetImage("slider_anchor"));
                                                                                                            }).Detach();
                                                                                                            _opacitySlider->SubscribeOnWheelUp([=, input = opacityInput.get()](Component*, int, int) {
                                                                                                                input->StepUp();
                                                                                                                }).Detach();
                                                                                                                _opacitySlider->SubscribeOnWheelDown([=, input = opacityInput.get()](Component*, int, int) {
                                                                                                                    input->StepDown();
                                                                                                                    }).Detach();
                                                                                                                    _opacitySlider->SetBodyComponent(std::move(opacitySliderBody));
                                                                                                                    _opacitySlider->SetAnchorComponent(std::move(opacitySliderAnchor));
                                                                                                                    opacityRow->AddItem(std::move(opacityLabel));
                                                                                                                    opacityRow->AddItem(_opacitySlider.get());
                                                                                                                    opacityRow->AddItem(std::move(opacityInput));

                                                                                                                    auto revertButton = Create<Button>(L"Revert changes");
                                                                                                                    revertButton->SetBaseSize(120, 26);
                                                                                                                    revertButton->SetHorizontalAlignment(Alignment::END);
                                                                                                                    revertButton->SetSelectable(false);
                                                                                                                    revertButton->SetBorderVisibility(false);
                                                                                                                    revertButton->SetBackgroundColor(D2D1::ColorF(0x303030));
                                                                                                                    revertButton->SetButtonColor(D2D1::ColorF(0, 0.0f));
                                                                                                                    revertButton->SetButtonHoverColor(D2D1::ColorF(0xFFFFFF, 0.1f));
                                                                                                                    revertButton->SetButtonClickColor(D2D1::ColorF(0x000000, 0.1f));
                                                                                                                    revertButton->SetCornerRounding(2.0f);
                                                                                                                    revertButton->SetProperty(PROP_Shadow());
                                                                                                                    revertButton->SubscribeOnActivated([=] {
                                                                                                                        _currentColor = _initialColor;
                                                                                                                        _redSlider->SetValue(_currentColor.r / 255.0f);
                                                                                                                        _greenSlider->SetValue(_currentColor.g / 255.0f);
                                                                                                                        _blueSlider->SetValue(_currentColor.b / 255.0f);
                                                                                                                        _opacitySlider->SetValue(_currentColor.a / 255.0f);
                                                                                                                        _OnColorChanged();
                                                                                                                        }).Detach();

                                                                                                                        mainPanel->AddItem(std::move(redRow));
                                                                                                                        mainPanel->AddItem(std::move(greenRow));
                                                                                                                        mainPanel->AddItem(std::move(blueRow));
                                                                                                                        mainPanel->AddItem(std::move(opacityRow));
                                                                                                                        mainPanel->AddItem(std::move(revertButton));

                                                                                                                        _OnColorChanged();

                                                                                                                        _canvas->BasePanel()->AddItem(std::move(mainPanel));
                                                                                                                        _canvas->SetBackgroundColor(D2D1::ColorF(0.1f, 0.1f, 0.1f));
}

void zcom::ColorSelectorScene::_Uninit()
{
    _canvas->ClearComponents();
    _SaveOptions();
}

void zcom::ColorSelectorScene::_Focus()
{

}

void zcom::ColorSelectorScene::_Unfocus()
{

}

void zcom::ColorSelectorScene::_Update()
{
    _canvas->Update();

    if (!_optionsSaved && ztime::Main() > _lastColorChange + Duration(250, MILLISECONDS))
    {
        _optionsSaved = true;
        _SaveOptions();
    }
}

void zcom::ColorSelectorScene::_Resize(int width, int height, ResizeInfo info)
{

}

void zcom::ColorSelectorScene::_OnColorChanged()
{
    _redSlider->GetBodyComponent()->InvokeRedraw();
    _greenSlider->GetBodyComponent()->InvokeRedraw();
    _blueSlider->GetBodyComponent()->InvokeRedraw();
    _opacitySlider->GetBodyComponent()->InvokeRedraw();

    _window->GetTitleBarScene()->SetBackground(D2D1::ColorF(_currentColor.r / 255.0f, _currentColor.g / 255.0f, _currentColor.b / 255.0f, _currentColor.a / 255.0f));
    _lastColorChange = ztime::Main();
    _optionsSaved = false;
}

void zcom::ColorSelectorScene::_SaveOptions()
{
    _app->options.SetIntValue(_optionName, _currentColor.ToInt());
}

void zcom::ColorSelectorScene::_DrawGradient(Component* item, D2D1_COLOR_F startColor, D2D1_COLOR_F endColor, Graphics g)
{
    ID2D1GradientStopCollection* pGradientStops = nullptr;
    ID2D1LinearGradientBrush* pGradientBrush = nullptr;

    D2D1_GRADIENT_STOP gradientStops[2];
    gradientStops[0].color = startColor;
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = endColor;
    gradientStops[1].position = 1.0f;
    g.target->CreateGradientStopCollection(
        gradientStops,
        2,
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &pGradientStops
    );
    g.target->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(0, item->GetHeight() / 2),
            D2D1::Point2F(item->GetWidth(), item->GetHeight() / 2)),
        pGradientStops,
        &pGradientBrush
    );
    g.target->FillRectangle(D2D1::RectF(0, 0, item->GetWidth(), item->GetHeight()), pGradientBrush);

    pGradientBrush->Release();
    pGradientStops->Release();
}

void zcom::ColorSelectorScene::_DrawCheckeredPattern(Component* item, D2D1_COLOR_F color1, D2D1_COLOR_F color2, Graphics g)
{
    float cellSize = 5.0f;

    ID2D1CommandList* patternCommandList = nullptr;
    g.target->CreateCommandList(&patternCommandList);

    ID2D1SolidColorBrush* cellBrush = nullptr;
    g.target->CreateSolidColorBrush(color2, &cellBrush);

    ID2D1Image* stash = nullptr;
    g.target->GetTarget(&stash);
    g.target->SetTarget(patternCommandList);
    g.target->FillRectangle(D2D1::RectF(0.0f, 0.0f, cellSize, cellSize), cellBrush);
    g.target->FillRectangle(D2D1::RectF(cellSize, cellSize, cellSize * 2, cellSize * 2), cellBrush);
    g.target->SetTarget(stash);
    stash->Release();

    patternCommandList->Close();

    ID2D1ImageBrush* patternBrush = nullptr;
    g.target->CreateImageBrush(
        patternCommandList,
        D2D1::ImageBrushProperties(
            D2D1::RectF(0, 0, cellSize * 2, cellSize * 2),
            D2D1_EXTEND_MODE_WRAP,
            D2D1_EXTEND_MODE_WRAP
        ),
        &patternBrush
    );

    g.target->Clear(color1);
    g.target->FillRectangle(D2D1::RectF(0, 0, item->GetWidth(), item->GetHeight()), patternBrush);

    patternBrush->Release();
    cellBrush->Release();
    patternCommandList->Release();
}