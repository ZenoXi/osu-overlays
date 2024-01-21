#pragma once

#include "Panel.h"
#include "MenuItem.h"

#include "Window/WindowId.h"
#include "Window/WindowMessage.h"

#include <optional>
#include <future>

// Every menu is contained in its own window, which means opening a child menu requires creating a window.
// Since components cannot be shared between windows (component rendering is tied to a window specific
// device context), the content of a child menu must be generated from a template. This template contains
// a list of nestable MenuItem descriptors. These descriptors contain the item data and logic in the form
// of lambdas.
// 
// When creating a child menu, the child menu subscribes to parent events:
//  - Close request
// And the parent subscribes to child events:
//  - Mouse move
//

namespace zcom
{
    class Canvas;

    struct MenuParams
    {
        RECT parentRect;
        MenuTemplate::Menu menuTemplate;
        std::unique_ptr<AsyncEventSubscription<void>> closeRequestSubscription = nullptr;
        std::optional<EventEmitter<void>> fullCloseRequestEventEmitter = std::nullopt;
        std::optional<EventEmitter<void>> mouseMoveEventEmitter = std::nullopt;
    };

    class MenuPanel : public Panel
    {
#pragma region base_class
    protected:
        void _OnUpdate()
        {
            Panel::_OnUpdate();

            // Hide panel
            if (_childShouldHide && ztime::Main() - _childHoverEndTime >= _hoverToShowDuration)
            {
                if (_childMenuShowing)
                {
                    _closeRequestEventEmitter->InvokeAll();
                    _childMenuShowing = false;
                    _childItemId = std::nullopt;
                }
                _childShouldHide = false;
            }

            // Show panel
            if (_childMenuToShow && ztime::Main() - _childHoverStartTime >= _hoverToShowDuration)
            {
                if (_childMenuShowing)
                {
                    _closeRequestEventEmitter->InvokeAll();
                }
                _OpenChildMenu(_childMenuToShow.value());
                _childMenuShowing = true;
                _childItemId = _childMenuToShow;
                _childMenuToShow = std::nullopt;
            }
        }

        EventTargets _OnMouseMove(int deltaX, int deltaY)
        {
            // Notify parent menu of mouse movement
            _mouseMoveEventEmitter->InvokeAll();

            auto targets = Panel::_OnMouseMove(deltaX, deltaY);
            Component* mainTarget = targets.MainTarget();
            auto it = std::find_if(_items.begin(), _items.end(), [mainTarget](Item& item) { return item.item == mainTarget; });
            if (it != _items.end())
            {
                MenuItem* item = (MenuItem*)it->item;

                if (_hoveredItem)
                    _hoveredItem->SetBackgroundColor(D2D1::ColorF(0, 0.0f));
                _hoveredItem = item;
                if (!_hoveredItem->IsSeparator() && !_hoveredItem->Disabled())
                    _hoveredItem->SetBackgroundColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.1f));

                // Stop scheduled hide
                if (_childMenuShowing && item->GetId() == _childItemId.value())
                {
                    _childShouldHide = false;
                }

                // Stop scheduled show
                if (_childMenuToShow && item->GetId() != _childMenuToShow.value())
                {
                    _childMenuToShow = std::nullopt;
                }

                // Prime open panel to hide
                if (_childMenuShowing && item->GetId() != _childItemId.value())
                {
                    if (!_childShouldHide)
                    {
                        _childShouldHide = true;
                        _childHoverEndTime = ztime::Main();
                    }
                }

                // Prime panel to open
                if (!item->Disabled() && item->GetMenu() && (!_childItemId.has_value() || item->GetId() != _childItemId.value()))
                {
                    if (!_childMenuToShow)
                    {
                        _childMenuToShow = item->GetId();
                        _childHoverStartTime = ztime::Main();
                    }
                }
            }

            return std::move(targets.Add(this, GetMousePosX(), GetMousePosY()));
        }

        EventTargets _OnLeftPressed(int x, int y)
        {
            auto targets = Panel::_OnLeftPressed(x, y);
            Component* mainTarget = targets.MainTarget();
            auto it = std::find_if(_items.begin(), _items.end(), [mainTarget](Item& item) { return item.item == mainTarget; });
            if (it != _items.end())
            {
                MenuItem* item = (MenuItem*)it->item;

                // Adjust nested menu opening timer to open immediatelly
                _childHoverEndTime = ztime::Main() - _hoverToShowDuration;
                _childHoverStartTime = ztime::Main() - _hoverToShowDuration;

                if (!item->Disabled())
                {
                    // Handle checkable items
                    if (item->Checkable())
                    {
                        if (item->CheckGroup() == -1)
                        {
                            item->Invoke(!item->Checked());
                            item->SetChecked(!item->Checked());
                        }
                        else
                        {
                            if (!item->Checked())
                            {
                                // Uncheck others from same group
                                for (int i = 0; i < _items.size(); i++)
                                {
                                    MenuItem* mItem = (MenuItem*)_items[i].item;
                                    if (mItem->CheckGroup() == item->CheckGroup() && mItem->Checked())
                                    {
                                        mItem->SetChecked(false);
                                    }
                                }

                                item->Invoke(true);
                                item->SetChecked(true);
                            }
                        }
                    }
                    else
                    {
                        item->Invoke();
                    }
                }

                if (!item->GetMenu() &&
                    !item->IsSeparator() &&
                    !item->Disabled() &&
                    item->CloseOnClick())
                    FullClose();
            }

            return std::move(targets.Add(this, x, y));
        }

        void _OnMouseLeave()
        {
            // Stop scheduled child menu open
            _childMenuToShow = std::nullopt;

            if (_childMenuShowing)
            {
                // Unhighlight item only if it is not representing the currently open child menu
                bool unhighlight = true;
                for (auto& it : _items)
                {
                    if (((MenuItem*)it.item)->GetId() == _childItemId.value())
                    {
                        if (_hoveredItem == (MenuItem*)it.item)
                        {
                            unhighlight = false;
                            break;
                        }
                    }
                }
                if (unhighlight)
                {
                    if (_hoveredItem)
                    {
                        _hoveredItem->SetBackgroundColor(D2D1::ColorF(0, 0.0f));
                        _hoveredItem = nullptr;
                    }
                }
            }
            else
            {
                // Unhighlight item
                if (_hoveredItem)
                {
                    _hoveredItem->SetBackgroundColor(D2D1::ColorF(0, 0.0f));
                    _hoveredItem = nullptr;
                }
            }
        }

    public:
        const char* GetName() const { return Name(); }
        static const char* Name() { return "menu_panel"; }
#pragma endregion

    private:
        bool _childMenuShowing = false;
        std::optional<MenuItem::Id> _childItemId = std::nullopt;
        MenuItem* _hoveredItem = nullptr;

        RECT _bounds = { 0, 0, 0, 0 };
        // Parent menu or other source rect in virtual screen coordinates
        RECT _parentRect = { 0, 0, 0, 0 };
        int _maxWidth = 600;
        int _minWidth = 70;

        TimePoint _childHoverStartTime = 0;
        std::optional<MenuItem::Id> _childMenuToShow = std::nullopt;
        TimePoint _childHoverEndTime = 0;
        bool _childShouldHide = false;

        TimePoint _showTime = 0;
        Duration _hoverToShowDuration = Duration(200, MILLISECONDS);

        // Parent-child menu communication
        // 
        // Close request:
        // - Parent menu sends a close request to its child when the child should close
        // Full close request:
        // - Any menu that encounters the need to close the entire menu chain, sends the full close
        // - request to the base menu, which in turns closes and sends a close request to its child.
        // - Only the base menu has the subscription, while every menu in the chain has the emitter
        // Mouse move event:
        // - Child menu emits the mouse move event to the parent when a mouse moves in its window
        //

        EventEmitter<void> _closeRequestEventEmitter;
        std::unique_ptr<AsyncEventSubscription<void>> _closeRequestSubscription;

        EventEmitter<void> _fullCloseRequestEventEmitter;
        std::unique_ptr<AsyncEventSubscription<void>> _fullCloseRequestSubscription;
        
        EventEmitter<void> _mouseMoveEventEmitter;
        std::unique_ptr<AsyncEventSubscription<void>> _mouseMoveSubscription;

        std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _parentWindowClickSubscription;


    protected:
        friend class Scene;
        friend class Component;
        MenuPanel(Scene* scene)
          : Panel(scene),
            _closeRequestEventEmitter(EventEmitterThreadMode::MULTITHREADED)
        {}
        void Init(MenuParams params);
    public:
        ~MenuPanel() {}
        MenuPanel(MenuPanel&&) = delete;
        MenuPanel& operator=(MenuPanel&&) = delete;
        MenuPanel(const MenuPanel&) = delete;
        MenuPanel& operator=(const MenuPanel&) = delete;

        void SetMaxWidth(int maxWidth)
        {
            if (_maxWidth != maxWidth)
            {
                _maxWidth = maxWidth;
                _RearrangeMenuItems();
                _CalculatePlacement();
            }
        }

        void SetMinWidth(int minWidth)
        {
            if (_minWidth != minWidth)
            {
                _minWidth = minWidth;
                _RearrangeMenuItems();
                _CalculatePlacement();
            }
        }

        void AddItem(std::unique_ptr<MenuItem> item)
        {
            Panel::AddItem(std::move(item));
            _RearrangeMenuItems();
            _CalculatePlacement();
        }

        MenuItem* GetItem(int index)
        {
            return (MenuItem*)Panel::GetItem(index);
        }

        size_t ItemCount() const
        {
            return Panel::ItemCount();
        }

        void ClearItems()
        {
            Panel::ClearItems();
            _RearrangeMenuItems();
            _CalculatePlacement();
        }

        void HandleCloseRequest();

        void CloseWindow();

        void OnChildMouseMove()
        {
            // Stop cheduled child menu closing
            _childShouldHide = false;

            // Highlight item representing child menu
            for (auto& it : _items)
            {
                if (_childItemId.has_value() && ((MenuItem*)it.item)->GetId() == _childItemId.value())
                {
                    if (_hoveredItem)
                        _hoveredItem->SetBackgroundColor(D2D1::ColorF(0, 0.0f));
                    _hoveredItem = (MenuItem*)it.item;
                    _hoveredItem->SetBackgroundColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.1f));
                    break;
                }
            }
        }

        // Sends a full close request to the root menu
        void FullClose()
        {
            _fullCloseRequestEventEmitter->InvokeAll();
        }

    private:
        std::future<std::optional<zwnd::WindowId>> _OpenChildMenu(MenuItem::Id id);

        void _AddHandlerToCanvas();

        void _RearrangeMenuItems()
        {
            constexpr int MARGINS = 2;

            _RecalculateLayout(GetWidth(), GetHeight());
            int totalHeight = MARGINS;
            int maxWidth = 0;
            for (int i = 0; i < _items.size(); i++)
            {
                _items[i].item->SetOffsetPixels(MARGINS, totalHeight);
                _items[i].item->SetBaseWidth(-MARGINS * 2);
                totalHeight += _items[i].item->GetHeight();
                int width = ((MenuItem*)_items[i].item)->CalculateWidth();
                if (width > maxWidth)
                    maxWidth = width;
            }

            if (maxWidth < _minWidth)
                maxWidth = _minWidth;
            if (maxWidth > _maxWidth)
                maxWidth = _maxWidth;

            SetBaseSize(maxWidth + MARGINS * 2, totalHeight + MARGINS);
        }

        void _CalculatePlacement();
    };
}