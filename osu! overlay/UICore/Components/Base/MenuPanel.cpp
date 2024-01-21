#include "MenuPanel.h"
#include "Canvas.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Scenes/ContextMenuScene.h"
#include "Window/Window.h"
#include "Helper/MiscUtils.h"

#include "MenuTemplate.h"

#include <string>

void zcom::MenuPanel::Init(MenuParams params)
{
    Panel::Init();

    SetBackgroundColor(D2D1::ColorF(0.05f, 0.05f, 0.05f));

    _parentRect = params.parentRect;
    _bounds = _scene->GetWindow()->Backend().GetMonitorRectAtWindowPoint(_parentRect.right, _parentRect.top);
    
    if (params.closeRequestSubscription)
    {
        _closeRequestSubscription = std::move(params.closeRequestSubscription);
        _closeRequestSubscription->ResetSynchronousHandler([this]() {
            HandleCloseRequest();
        });
    }
    if (params.mouseMoveEventEmitter)
    {
        _mouseMoveEventEmitter = params.mouseMoveEventEmitter.value();
    }
    if (params.fullCloseRequestEventEmitter)
    {
        _fullCloseRequestEventEmitter = params.fullCloseRequestEventEmitter.value();
    }
    else
    {
        _fullCloseRequestEventEmitter = EventEmitter<void>(EventEmitterThreadMode::MULTITHREADED);
        _fullCloseRequestSubscription = _fullCloseRequestEventEmitter->SubscribeAsync([this]() {
            HandleCloseRequest();
        });
    }

    // Create menu components from template
    for (auto& item : params.menuTemplate.items)
    {
        std::unique_ptr<MenuItem> menuItem;

        if (item.separator)
            menuItem = Create<MenuItem>();
        else if (item.onClick)
            menuItem = Create<MenuItem>(item.text, item.onClick);
        else if (item.nestedMenu)
            menuItem = Create<MenuItem>(item.nestedMenu.value(), item.text);
        if (!menuItem)
            continue;
        
        menuItem->SetCheckable(item.checkable);
        menuItem->SetCheckGroup(item.checkGroup);

        if (!item.iconResourceName.empty())
        {
            _scene->GetWindow()->resourceManager.InitImage(item.iconResourceName);
            menuItem->SetIcon(_scene->GetWindow()->resourceManager.GetImage(item.iconResourceName));
        }

        Panel::AddItem(std::move(menuItem));
    }

    _RearrangeMenuItems();
    _CalculatePlacement();

    ExecuteSynchronously([&] {
        _scene->GetWindow()->Backend().SetDisplayType(zwnd::WindowDisplayType::NORMAL_NOACTIVATE);

        // Subscribe to main window messages
        auto parentOptional = _scene->GetWindow()->GetParent();
        if (parentOptional)
        {
            Handle<zwnd::Window> parentWindow = _scene->GetApp()->GetWindow(parentOptional.value());
            if (parentWindow.Valid())
            {
                _parentWindowClickSubscription = parentWindow->SubscribeToWindowMessages([=](zwnd::WindowMessage message) {
                    if (message.id == zwnd::MouseLeftPressedMessage::ID() ||
                        message.id == zwnd::MouseRightPressedMessage::ID() ||
                        message.id == zwnd::NonClientMouseLeftPressedMessage::ID() || 
                        message.id == zwnd::NonClientMouseRightPressedMessage::ID())
                    {
                        ExecuteSynchronously([=] {
                            FullClose();
                        });
                    }
                    else if (message.id == zwnd::WindowActivateMessage::ID())
                    {
                        zwnd::WindowActivateMessage msg{};
                        msg.Decode(message);
                        if (msg.activationType == zwnd::WindowActivateMessage::DEACTIVATED)
                        {
                            ExecuteSynchronously([=] {
                                FullClose();
                            });
                        }
                    }
                    return false;
                });
            }
        }
    });
}

void zcom::MenuPanel::HandleCloseRequest()
{
    _closeRequestEventEmitter->InvokeAll();
    ExecuteSynchronously([this]() {
        CloseWindow();
    });
}

void zcom::MenuPanel::CloseWindow()
{
    // Hiding first doesn't really add any noticeable speed improvement
    //_scene->GetWindow()->Backend().SetDisplayType(zwnd::WindowDisplayType::HIDDEN);
    _scene->GetWindow()->Close();
}

std::future<std::optional<zwnd::WindowId>> zcom::MenuPanel::_OpenChildMenu(MenuItem::Id id)
{
    MenuItem* item = nullptr;
    auto it = std::find_if(_items.begin(), _items.end(), [&](const Item& item) { return ((MenuItem*)item.item)->GetId().value == id.value; });
    if (it == _items.end())
        return MakeCompletedFuture<std::optional<zwnd::WindowId>>(std::nullopt);
    item = (MenuItem*)(*it).item;
    if (!item->GetMenu())
        return MakeCompletedFuture<std::optional<zwnd::WindowId>>(std::nullopt);

    MenuTemplate::Menu menu = item->GetMenu().value();

    zwnd::Window* window = _scene->GetWindow();
    RECT itemRectInScreenCoords = {
        item->GetWindowX(),
        item->GetWindowY(),
        item->GetWindowX() + item->GetWidth(),
        item->GetWindowY() + item->GetHeight()
    };
    RECT windowRect = window->Backend().GetWindowRectangle();
    itemRectInScreenCoords.left += windowRect.left;
    itemRectInScreenCoords.top += windowRect.top;
    itemRectInScreenCoords.right += windowRect.left;
    itemRectInScreenCoords.bottom += windowRect.top;

    auto mouseMoveEventEmitter = EventEmitter<void>(EventEmitterThreadMode::MULTITHREADED);
    _mouseMoveSubscription = mouseMoveEventEmitter->SubscribeAsync([this]() {
        ExecuteSynchronously([this]() {
            OnChildMouseMove();
        });
    });

    auto parentWindowId = window->GetParent();
    if (!parentWindowId)
        return MakeCompletedFuture<std::optional<zwnd::WindowId>>(std::nullopt);

    return _scene->GetApp()->CreateToolWindowAsync(
        // Use top/child window as parent
        parentWindowId.value(),
        zwnd::WindowProperties()
            .WindowClassName(L"wndClassMenu" + string_to_wstring(window->GetWindowId().StringValue()))
            .InitialSize(10, 10)
            .InitialDisplay(zwnd::WindowDisplayType::HIDDEN)
            .DisableWindowAnimations()
            .DisableWindowActivation(),
        [
            itemRectInScreenCoords,
            menu,
            closeRequestEventEmitter = _closeRequestEventEmitter,
            fullCloseRequestEventEmitter = _fullCloseRequestEventEmitter,
            mouseMoveEventEmitter
        ]
        (zwnd::Window* wnd)
        {
            wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
            wnd->resourceManager.InitAllImages();
            wnd->LoadNonClientAreaScene<zcom::DefaultNonClientAreaScene>(nullptr);

            zcom::ContextMenuSceneOptions opt;
            opt.params.parentRect = itemRectInScreenCoords;
            opt.params.menuTemplate = menu;
            // Create non-const copy to call subscribe
            auto nonConstCopy = closeRequestEventEmitter;
            opt.params.closeRequestSubscription = nonConstCopy->SubscribeAsync();
            opt.params.fullCloseRequestEventEmitter = fullCloseRequestEventEmitter;
            opt.params.mouseMoveEventEmitter = mouseMoveEventEmitter;
            wnd->LoadStartingScene<zcom::ContextMenuScene>(&opt);
        }
    );
}

void zcom::MenuPanel::_AddHandlerToCanvas()
{
    // General notes about menu rework implementation. Leaving here for documentation


    // Stopping the scheduled child close requires detecting when a mouse enters the child window.
    // Likely possible to add an event handler to the child window canvas, but doing this can easily
    // run into accessing freed memory if the closing order of the windows is not consistent or
    // communicated. Since the parent subscribes to a child window, the child should notify the
    // parent when it closes. It might be possible to just check if the child window is still open
    // and if it isn't, just not call the unsubscribe.

    // Stopping the scheduled child open should be done when mouse leaves the window (or in this case
    // the menu panel).

    // Highlighting the item representing the open child requires detecting when mouse enters the
    // child item window. Unhighlighting items not representing the open child just requires detecting
    // mouse leave and checking which item is highlighted at the moment.

    // If no child is open, on mouse leave the highlighted item should be unhighlighted



    // A menu should be closed when the mouse is clicked anywhere outside its and its parent,
    // boundaries. The parent part is important since clicking on the parent menu doesn't
    // necessarily mean that the menu should be closed. That logic should be left to the parent
    // panel.

    // Clicks outside of this and parent panel can fall into 4 categories:
    //  - Some deeper parent:
    //      This case can be ignored, since the clicked menu manages its children
    //  - Somewhere inside the window ('window' here refers to the window that owns the menu):
    //      The menu chain should be closed by calling FullClose() from the menu with no children
    //      Detecting a window click would involve adding a window mouse event handler
    //  - Outside the window:
    //      The menu chain should be closed by calling FullClose() from the menu with no children
    //      This can be detected by checking for deactivation of the window
    //  - Other menu chain:
    //      Realistically two menu chains should not be active at once
    //
}

void zcom::MenuPanel::_CalculatePlacement()
{
    constexpr int LEFT = 1;
    constexpr int RIGHT = 2;
    constexpr int UP = 1;
    constexpr int DOWN = 2;

    // Horizontal placement
    int hPlacement;
    if (_parentRect.right + GetBaseWidth() < _bounds.right)
        hPlacement = RIGHT;
    else if (_parentRect.left - GetBaseWidth() > _bounds.left)
        hPlacement = LEFT;
    else
        if (_bounds.right - _parentRect.right > _parentRect.left - _bounds.left)
            hPlacement = RIGHT;
        else
            hPlacement = LEFT;

    // Vertical placement
    int vPlacement;
    if (_parentRect.top + GetBaseHeight() < _bounds.bottom)
        vPlacement = DOWN;
    else if (_parentRect.bottom - GetBaseHeight() > _bounds.top)
        vPlacement = UP;
    else
        if (_bounds.bottom - _parentRect.top > _parentRect.bottom - _bounds.top)
            vPlacement = DOWN;
        else
            vPlacement = UP;

    // Final x position
    int xPos;
    if (hPlacement == RIGHT)
        xPos = _parentRect.right;
    else
        xPos = _parentRect.left - GetBaseWidth();

    // Final y position
    int yPos;
    if (vPlacement == DOWN)
        yPos = _parentRect.top;
    else
        yPos = _parentRect.bottom - GetBaseHeight();

    // Set window position accounting for non client area overlap
    RECT clientAreaMargins = _scene->GetWindow()->GetNonClientAreaScene()->GetClientAreaMargins();
    RECT finalRect = {
        xPos - clientAreaMargins.left,
        yPos - clientAreaMargins.top,
        xPos + GetBaseWidth() + clientAreaMargins.right,
        yPos + GetBaseHeight() + clientAreaMargins.bottom
    };
    _scene->GetWindow()->Backend().SetWindowRectangle(finalRect);
}