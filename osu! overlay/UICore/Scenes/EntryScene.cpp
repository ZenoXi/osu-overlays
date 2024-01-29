#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "EntryScene.h"
#include "DefaultNonClientAreaScene.h"
#include "DefaultTitleBarScene.h"
#include "SmokeSim/SmokeSimScene.h"
#include "SmokeSim/SmokeSimParameterPanel.h"
#include "Components/Base/ScrollPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/EmptyPanel.h"

zcom::EntryScene::EntryScene(App* app, zwnd::Window* window)
    : Scene(app, window)
{}

void zcom::EntryScene::_Init(SceneOptionsBase* options)
{
    EntrySceneOptions opt;
    if (options)
    {
        opt = *reinterpret_cast<const EntrySceneOptions*>(options);
    }

    _overlayListLabel = Create<Label>(L"Overlay list");
    _overlayListLabel->SetOffsetPixels(15, 15);
    _overlayListLabel->SetBaseSize(200, 30);
    _overlayListLabel->SetFontSize(20.0f);
    _overlayListLabel->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
    _overlayListLabel->SetFont(L"Arial");

    

    _mainPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    _mainPanel->SetSizeFixed(true, true);
    _mainPanel->SetSpacing(1);
    _mainPanel->SetParentSizePercent(1.0f, 1.0f);

    _selectionPanel = Create<Panel>();
    _selectionPanel->SetParentSizePercent(1.0f, 1.0f);
    _selectionPanel->SetProperty(FlexShrink());
    _selectionPanel->SetBackgroundColor(D2D1::ColorF(0x1A1A1A));

    _overlayListPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _overlayListPanel->FillContainerWidth();
    _overlayListPanel->SetVerticalOffsetPixels(60);
    _overlayListPanel->SetSpacing(3);
    _overlayListPanel->SetItemAlignment(Alignment::CENTER);

    _CreateOverlaySelector(L"Enhanced smoke", L"enhancedSmokeOverlay", [=] {
        auto paramPanel = Create<SmokeSimParameterPanel>(SmokeSimType::ENHANCED_SMOKE);
        paramPanel->SetParentHeightPercent(1.0f);
        paramPanel->SetBaseWidth(300);
        return paramPanel;
    });
    _CreateOverlaySelector(L"Cursor trail", L"cursorTrailOverlay", [=] {
        auto paramPanel = Create<SmokeSimParameterPanel>(SmokeSimType::CURSOR_TRAIL);
        paramPanel->SetParentHeightPercent(1.0f);
        paramPanel->SetBaseWidth(300);
        return paramPanel;
    });

    auto creditsLabel1 = Create<Label>(L"Made by Zenox");
    creditsLabel1->SetParentWidthPercent(1.0f);
    creditsLabel1->SetBaseSize(-20, 20);
    creditsLabel1->SetVerticalOffsetPixels(-100);
    creditsLabel1->SetAlignment(Alignment::CENTER, Alignment::END);
    creditsLabel1->SetTextSelectable(true);

    auto creditsLabel2 = Create<Label>(L"If you have any questions, you can message me directly on osu!, username: ZenoXLTU\nFor updates and FAQ check the app page: https://github.com/ZenoXi/osu-overlays");
    creditsLabel2->SetParentWidthPercent(1.0f);
    creditsLabel2->SetBaseSize(-20, 80);
    creditsLabel2->SetVerticalOffsetPixels(-20);
    creditsLabel2->SetAlignment(Alignment::CENTER, Alignment::END);
    creditsLabel2->SetFontStyle(DWRITE_FONT_STYLE_ITALIC);
    creditsLabel2->SetTextSelectable(true);
    creditsLabel2->SetWordWrap(true);

    _windowCreatedEventSubscription = _app->SubscribeOnWindowCreated([=](zwnd::WindowId id, zwnd::WindowType, zwnd::WindowProperties props) {
        _canvas->BasePanel()->ExecuteSynchronously([=] {
            _HandleWindowCreatedEvent(id, props);
        });
    });
    _windowClosedEventSubscription = _app->SubscribeOnWindowClosed([=](zwnd::WindowId id) {
        _canvas->BasePanel()->ExecuteSynchronously([=] {
            _HandleWindowClosedEvent(id);
        });
    });

    _selectionPanel->AddItem(_overlayListLabel.get());
    _selectionPanel->AddItem(_overlayListPanel.get());
    _selectionPanel->AddItem(std::move(creditsLabel1));
    _selectionPanel->AddItem(std::move(creditsLabel2));

    _mainPanel->AddItem(_selectionPanel.get());

    _canvas->AddComponent(_mainPanel.get());
    _canvas->SetBackgroundColor(D2D1::ColorF(0));
}

void zcom::EntryScene::_Uninit()
{
    _canvas->ClearComponents();
}

void zcom::EntryScene::_Focus()
{

}

void zcom::EntryScene::_Unfocus()
{

}

void zcom::EntryScene::_Update()
{
    _canvas->Update();
}

void zcom::EntryScene::_Resize(int width, int height, ResizeInfo info)
{

}

void zcom::EntryScene::_CreateOverlaySelector(std::wstring buttonText, std::wstring windowClassName, std::function<std::unique_ptr<Component>()> parameterPanelInitFunc)
{
    auto row = Create<FlexPanel>(FlexDirection::RIGHT);
    row->FillContainerWidth();
    row->SetBaseWidth(-20);
    row->SetSpacing(3);
    row->SetProperty(PROP_Shadow{});
    auto statusIndicator = Create<EmptyPanel>();
    statusIndicator->SetBaseSize(10, 30);
    statusIndicator->SetCornerRounding(3);
    statusIndicator->SetBackgroundColor(D2D1::ColorF(0x30B020));
    statusIndicator->SetVisible(false);
    auto button = Create<Button>(buttonText);
    button->SetParentWidthPercent(1.0f);
    button->SetBaseHeight(30);
    button->SetProperty(FlexShrink());
    button->SetSelectable(false);
    button->SetBorderVisibility(false);
    button->SetBackgroundColor(D2D1::ColorF(0x303030));
    button->SetButtonColor(D2D1::ColorF(0, 0.0f));
    button->SetButtonHoverColor(D2D1::ColorF(0xFFFFFF, 0.1f));
    button->SetButtonClickColor(D2D1::ColorF(0x000000, 0.1f));
    button->SetCornerRounding(3);
    button->SubscribeOnActivated([=] {
        if (_currentPropertyPanel)
            _mainPanel->RemoveItem(_currentPropertyPanel);

        auto paramPanel = parameterPanelInitFunc();
        _currentPropertyPanel = paramPanel.get();
        _mainPanel->AddItem(std::move(paramPanel));
    }).Detach();

    _OverlaySelector selector;
    selector.overlayWindowClassName = windowClassName;
    selector.statusIndicator = statusIndicator.get();
    _overlaySelectors.push_back(std::move(selector));

    row->AddItem(std::move(statusIndicator));
    row->AddItem(std::move(button));
    _overlayListPanel->AddItem(std::move(row));
}

void zcom::EntryScene::_HandleWindowCreatedEvent(zwnd::WindowId windowId, zwnd::WindowProperties props)
{
    for (auto& selector : _overlaySelectors)
    {
        if (selector.overlayWindowClassName == props.windowClassName)
        {
            selector.overlayWindowId = windowId;
            selector.statusIndicator->SetVisible(true);
            break;
        }
    }
}

void zcom::EntryScene::_HandleWindowClosedEvent(zwnd::WindowId windowId)
{
    for (auto& selector : _overlaySelectors)
    {
        if (selector.overlayWindowId.has_value() && selector.overlayWindowId.value() == windowId)
        {
            selector.overlayWindowId = std::nullopt;
            selector.statusIndicator->SetVisible(false);
            break;
        }
    }
}