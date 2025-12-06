#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "ColorSelectorScene.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Dummy.h"
#include "Components/Base/NumberInput.h"
#include "Components/Base/Button.h"
#include "Shared/Styles/Styles.h"

void zcom::ColorSelectorScene::Init(SceneOptionsBase* options)
{
    ColorSelectorSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const ColorSelectorSceneOptions*>(options);
    _configValue = opt.configValue;
    _colorSelectorSceneValueChangedEventEmitter = opt.colorSelectorSceneValueChangedEventEmitter;
    if (opt.colorSelectorValueChangedEventEmitter)
    {
        _colorSelectorValueChangedSubscription = opt.colorSelectorValueChangedEventEmitter.value()->SubscribeAsync([=](Color color) {
            _basePanel->ExecuteSynchronously([=]{
                _currentColor = color;
                _OnColorChanged();
            });
        });
    }

    if (_configValue)
        _initialColor = Color::ARGB(_app->config.GetIntConfigValue(_configValue.value(), Config::ADD_AND_SAVE_IF_MISSING));
    else if (opt.initialColor)
        _initialColor = opt.initialColor.value();
    _currentColor = _initialColor;

    auto mainPanel = Create<FlexPanel>(FlexDirection::DOWN);
    mainPanel->parentSize = { 1.0f, 1.0f };
    mainPanel->padding = { 10, 10, 10, 10 };
    mainPanel->spacing = 5;

    _redInput = Create<ColorSliderInputGroup>(L"Red:", _initialColor.r);
    _redInput->parentSize = { 1.0f, 0.0f };
    _redInput->autoHeight = true;
    _redInput->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics* g) {
        Color startColor = Color(0, _currentColor.g, _currentColor.b);
        Color endColor = Color(255, _currentColor.g, _currentColor.b);
        _DrawGradient(item, startColor, endColor, g);
    }).Detach();
    _redInput->value.Subscribe([=](uint8_t newValue) {
        _currentColor.r = newValue;
        _OnColorChanged();
    }).Detach();
    
    _greenInput = Create<ColorSliderInputGroup>(L"Green:", _initialColor.g);
    _greenInput->parentSize = { 1.0f, 0.0f };
    _greenInput->autoHeight = true;
    _greenInput->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics* g) {
        Color startColor = Color(_currentColor.r, 0, _currentColor.b);
        Color endColor = Color(_currentColor.r, 255, _currentColor.b);
        _DrawGradient(item, startColor, endColor, g);
    }).Detach();
    _greenInput->value.Subscribe([=](int newValue) {
        _currentColor.g = newValue;
        _OnColorChanged();
    }).Detach();

    _blueInput = Create<ColorSliderInputGroup>(L"Blue:", _initialColor.b);
    _blueInput->parentSize = { 1.0f, 0.0f };
    _blueInput->autoHeight = true;
    _blueInput->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics* g) {
        Color startColor = Color(_currentColor.r, _currentColor.g, 0);
        Color endColor = Color(_currentColor.r, _currentColor.g, 255);
        _DrawGradient(item, startColor, endColor, g);
    }).Detach();
    _blueInput->value.Subscribe([=](uint8_t newValue) {
        _currentColor.b = newValue;
        _OnColorChanged();
    }).Detach();

    _opacityInput = Create<ColorSliderInputGroup>(L"Opacity:", _initialColor.a);
    _opacityInput->parentSize = { 1.0f, 0.0f };
    _opacityInput->autoHeight = true;
    _opacityInput->GetSlider()->GetBodyComponent()->SubscribePostDraw([=](Component* item, Graphics* g) {
        _DrawCheckeredPattern(item, Color(0x404040), Color(0x303030), g);
        Color startColor = Color(_currentColor.r, _currentColor.g, _currentColor.b, 0);
        Color endColor = Color(_currentColor.r, _currentColor.g, _currentColor.b, 255);
        _DrawGradient(item, startColor, endColor, g);
    }).Detach();
    _opacityInput->value.Subscribe([=](uint8_t newValue) {
        _currentColor.a = newValue;
        _OnColorChanged();
    }).Detach();

    auto buttonRow = Create<Panel>();
    buttonRow->parentSize = { 1.0f, 0.0f };
    buttonRow->size = { 0, 26 };
    buttonRow->SetProperty(Shadow());
    if (_configValue)
    {
        auto resetToDefaultButton = Create<Button>(L"Reset to default");
        resetToDefaultButton->size = { 120, 26 };
        NeutralButtonStyle::Apply(resetToDefaultButton.get());
        resetToDefaultButton->RemoveProperty<Shadow>();
        resetToDefaultButton->SubscribeOnActivated([=] {
            _currentColor = Color::ARGB(_configValue->defaultValue);
            _redInput->value = _currentColor.r;
            _greenInput->value = _currentColor.g;
            _blueInput->value = _currentColor.b;
            _opacityInput->value = _currentColor.a;
            _OnColorChanged();
        }).Detach();
        buttonRow->AddItem(std::move(resetToDefaultButton));
    }
    auto revertButton = Create<Button>(L"Revert changes");
    revertButton->size = { 120, 26 };
    revertButton->xAlign = Alignment::END;
    NeutralButtonStyle::Apply(revertButton.get());
    revertButton->RemoveProperty<Shadow>();
    revertButton->SubscribeOnActivated([=] {
        _currentColor = _initialColor;
        _redInput->value = _currentColor.r;
        _greenInput->value = _currentColor.g;
        _blueInput->value = _currentColor.b;
        _opacityInput->value = _currentColor.a;
        _OnColorChanged();
    }).Detach();
    buttonRow->AddItem(std::move(revertButton));
    
    mainPanel->AddItem(_redInput.get());
    mainPanel->AddItem(_greenInput.get());
    mainPanel->AddItem(_blueInput.get());
    mainPanel->AddItem(_opacityInput.get());
    mainPanel->AddItem(std::move(buttonRow));

    _OnColorChanged();

    _basePanel->AddItem(std::move(mainPanel));
    _basePanel->backgroundColor = Color(0x1A1A1A);
    _basePanel->SubscribePostUpdate([=]() {
        _Update();
    }).Detach();
}

void zcom::ColorSelectorScene::Uninit()
{
    _app->config.SaveConfig();
}

void zcom::ColorSelectorScene::_Update()
{
    if (_configValue)
    {
        if (!_optionsSaved && ztime::Main() > _lastColorChange + Duration(250, MILLISECONDS))
        {
            _optionsSaved = true;
            _app->config.SaveConfig();
        }
    }
}

void zcom::ColorSelectorScene::_OnColorChanged()
{
    _redInput->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _greenInput->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _blueInput->GetSlider()->GetBodyComponent()->InvokeRedraw();
    _opacityInput->GetSlider()->GetBodyComponent()->InvokeRedraw();

    // Always have some opacity to allow window caption to be interacted with
    _window->GetTitleBarScene()->SetBackground(_currentColor.WithA(_currentColor.a == 0 ? uint8_t(1) : _currentColor.a));
    if (_colorSelectorSceneValueChangedEventEmitter)
        _colorSelectorSceneValueChangedEventEmitter.value()->InvokeAll(_currentColor);

    if (_configValue)
    {
        _app->config.SetIntValue(_configValue->name, _currentColor.ToInt(), false);
        _lastColorChange = ztime::Main();
        _optionsSaved = false;
    }
}

void zcom::ColorSelectorScene::_DrawGradient(Component* item, Color startColor, Color endColor, Graphics* g)
{
    LinearGradient gradient{};
    gradient.startPosition = { 0.0f, 0.0f };
    gradient.endPosition = { (float)item->size_->width, 0.0f };
    gradient.gradientStops.push_back({ 0.0f, startColor });
    gradient.gradientStops.push_back({ 1.0f, endColor });
    g->FillRectangle(item->size_->ToRect().ToRectF(), gradient);
}

void zcom::ColorSelectorScene::_DrawCheckeredPattern(Component* item, Color color1, Color color2, Graphics* g)
{
    float cellSize = 5.0f;

    ID2D1CommandList* patternCommandList = nullptr;
    g->GetRenderContext()->CreateCommandList(&patternCommandList);
    if (patternCommandList)
    {
        ID2D1SolidColorBrush* cellBrush = nullptr;
        g->GetRenderContext()->CreateSolidColorBrush(ColorToD2D1_COLOR_F(color2), &cellBrush);
        if (cellBrush)
        {
            ID2D1Image* stash = nullptr;
            g->GetRenderContext()->GetTarget(&stash);
            g->GetRenderContext()->SetTarget(patternCommandList);
            g->GetRenderContext()->FillRectangle(D2D1::RectF(0.0f, 0.0f, cellSize, cellSize), cellBrush);
            g->GetRenderContext()->FillRectangle(D2D1::RectF(cellSize, cellSize, cellSize * 2, cellSize * 2), cellBrush);
            g->GetRenderContext()->SetTarget(stash);
            stash->Release();

            patternCommandList->Close();

            ID2D1ImageBrush* patternBrush = nullptr;
            g->GetRenderContext()->CreateImageBrush(
                patternCommandList,
                D2D1::ImageBrushProperties(
                    D2D1::RectF(0, 0, cellSize * 2, cellSize * 2),
                    D2D1_EXTEND_MODE_WRAP,
                    D2D1_EXTEND_MODE_WRAP
                ),
                &patternBrush
            );
            if (patternBrush)
            {
                auto rect = g->GetTargetSourceClipRect().ToRectF();
                patternBrush->SetTransform(D2D1::Matrix3x2F::Translation(rect.left, rect.top));
                g->GetRenderContext()->Clear(ColorToD2D1_COLOR_F(color1));
                g->GetRenderContext()->FillRectangle(RectFToD2D1_RECT_F(rect), patternBrush);
                patternBrush->Release();
            }
            else
            {
                // TODO: Logging
            }
            cellBrush->Release();
        }
        else
        {
            // TODO: Logging
        }
        patternCommandList->Release();
    }
    else
    {
        // TODO: Logging
    }
}