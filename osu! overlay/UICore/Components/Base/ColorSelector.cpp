#include "ColorSelector.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

#include "Scenes/ColorSelectorScene.h"

zcom::ColorSelector::~ColorSelector()
{
    if (_windowView->open_)
    {
        Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_windowView->id_->value());
        if (handle.Valid())
            handle->Close();
    }
}

void zcom::ColorSelector::Init()
{
    _windowClassName = L"colorSelector_" + std::to_wstring(GetId());
    _windowView.emplace(_windowClassName, this);
}

void zcom::ColorSelector::UseConfigValue(ConfigValue<int> configValue)
{
    _configValue = configValue;
    color = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(configValue, Config::ADD_AND_SAVE_IF_MISSING));
}

void zcom::ColorSelector::_OpenColorSelector()
{
    int width = 300;
    int height = 214;
    RECT mainWindowRect = _scene->GetWindow()->Backend().GetWindowRectangle();
    int mainWindowCenterX = (mainWindowRect.left + mainWindowRect.right) / 2;
    int mainWindowCenterY = (mainWindowRect.top + mainWindowRect.bottom) / 2;

    zwnd::WindowProperties props = zwnd::WindowProperties()
        .WindowClassName(_windowClassName)
        .InitialSize(width, height)
        .InitialOffset(mainWindowCenterX - width / 2, mainWindowCenterY - height / 2)
        .FixedSize()
        .DisableMaximizing()
        .DisableMinimizing()
        .DisableFastTooltips();

    EventEmitter<void, Color> sceneValueEventEmitter(EventEmitterThreadMode::MULTITHREADED);
    _colorSelectorSceneValueChangedSubscription = sceneValueEventEmitter->SubscribeAsync([=](Color newColor) {
        _settingInternally = true;
        color = newColor;
        _settingInternally = false;
        _colorChangedEventEmitter->InvokeAll(newColor);
    });

    _scene->GetApp()->CreateChildWindow(
        _scene->GetWindow()->GetWindowId(),
        props,
        [=, configValue = _configValue, initialColor = color.Get(), valueEventEmitter = _colorSelectorValueChangedEventEmitter](zwnd::Window* wnd) {
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
            tbOpt.windowTitle = colorSelectorPopupName;
            tbOpt.useCleartype = false;
            wnd->LoadTitleBarScene<DefaultTitleBarScene>(&tbOpt);

            ColorSelectorSceneOptions opt;
            opt.configValue = configValue;
            opt.initialColor = configValue ? std::nullopt : std::optional<Color>(initialColor);
            opt.colorSelectorValueChangedEventEmitter = valueEventEmitter;
            opt.colorSelectorSceneValueChangedEventEmitter = sceneValueEventEmitter;
            wnd->LoadStartingScene<ColorSelectorScene>(&opt);
        }
    );
}

ID2D1ImageBrush* zcom::ColorSelector::_CreateCheckeredPatternBrush(Graphics* g, D2D1_COLOR_F cellColor)
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
    g->GetRenderContext()->FillRectangle(D2D1::RectF(0.0f, 0.0f, cellSize, cellSize), cellBrush.Get());
    g->GetRenderContext()->FillRectangle(D2D1::RectF(cellSize, cellSize, cellSize * 2, cellSize * 2), cellBrush.Get());
    g->GetRenderContext()->SetTarget(stash);
    stash->Release();

    patternCommandList->Close();

    ID2D1ImageBrush* patternBrush = nullptr;
    g->GetRenderContext()->CreateImageBrush(
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

void zcom::ColorSelector::_OnDraw(Graphics* g)
{
    g->Clear(Color(0x303030));

    auto patternBrush = _CreateCheckeredPatternBrush(g, D2D1::ColorF(0x404040));
    if (patternBrush)
    {
        g->GetRenderContext()->FillRectangle(RectFToD2D1_RECT_F(g->GetTargetSourceClipRect().ToRectF()), patternBrush);
        patternBrush->Release();
    }

    g->FillRectangle(size_->ToRect().ToRectF(), color);
}

zcom::EventContext zcom::ColorSelector::_OnLeftReleased(std::optional<Point> point)
{
    if (!_windowView->open_)
    {
        _OpenColorSelector();
    }
    else
    {
        Handle<zwnd::Window> handle = _scene->GetApp()->GetWindow(_windowView->id_->value());
        if (handle.Valid())
            handle->Backend().Focus();
    }

    return Component::_OnLeftReleased(point);
}
