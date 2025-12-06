#pragma once

#include "FlexPanel.h"
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
        RECT parentRect = {};
        MenuTemplate::Menu menuTemplate;
        std::unique_ptr<AsyncEventSubscription<void>> closeRequestSubscription = nullptr;
        std::optional<EventEmitter<void>> fullCloseRequestEventEmitter = std::nullopt;
        std::optional<EventEmitter<void>> mouseMoveEventEmitter = std::nullopt;
    };

    class MenuPanel : public FlexPanel
    {
        DEFINE_COMPONENT(MenuPanel, FlexPanel)
        DEFAULT_DESTRUCTOR(MenuPanel)
    protected:
        void Init(MenuParams params);

    public:
        Value<int> minWidth = Value<int>(70, [=](int& currentValue, const int& minWidth) {
            currentValue = minWidth;
            DeferLayoutUpdates();
            for (auto& item : _items)
                ((MenuItem*)item.item)->minWidth = minWidth;
            ResumeLayoutUpdates();
        });
        Value<int> maxWidth = Value<int>(600, [=](int& currentValue, const int& maxWidth) {
            currentValue = maxWidth;
            DeferLayoutUpdates();
            for (auto& item : _items)
                ((MenuItem*)item.item)->maxWidth = maxWidth;
            ResumeLayoutUpdates();
        });

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
                        _hoveredItem->backgroundColor = Color();
                    _hoveredItem = (MenuItem*)it.item;
                    _hoveredItem->backgroundColor = Color(0xFFFFFF, 0.1f);
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
        bool _childMenuShowing = false;
        std::optional<MenuItem::Id> _childItemId = std::nullopt;
        MenuItem* _hoveredItem = nullptr;

        RECT _bounds = { 0, 0, 0, 0 };
        // Parent menu or other source rect in virtual screen coordinates
        RECT _parentRect = { 0, 0, 0, 0 };

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

        std::future<std::optional<zwnd::WindowId>> _OpenChildMenu(MenuItem::Id id);

        void _AddHandlerToCanvas();

        void _AddItem(Component* item, size_t position, bool transferOwnership) override
        {
            if (!dynamic_cast<MenuItem*>(item))
            {
                if (transferOwnership)
                    delete item;
                return;
            }
            Panel::_AddItem(item, position, transferOwnership);
        }

        void _ComputeItemLayout() override
        {
            FlexPanel::_ComputeItemLayout();
            _CalculatePlacement();
        }

        void _CalculatePlacement();

    protected:
        void _OnUpdate() override
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

        EventContext _OnMouseMove(Point point, Point deltaPos) override
        {
            // Notify parent menu of mouse movement
            _mouseMoveEventEmitter->InvokeAll();

            auto targets = Panel::_OnMouseMove(point, deltaPos);
            Component* mainTarget = targets.MainTarget();
            auto it = std::find_if(_items.begin(), _items.end(), [mainTarget](Item& item) { return item.item == mainTarget; });
            if (it != _items.end())
            {
                MenuItem* item = (MenuItem*)it->item;

                if (_hoveredItem)
                    _hoveredItem->backgroundColor = Color();
                _hoveredItem = item;
                if (!_hoveredItem->IsSeparator() && !_hoveredItem->disabled)
                    _hoveredItem->backgroundColor = Color(0xFFFFFF, 0.1f);

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
                if (!item->disabled && item->GetMenu() && (!_childItemId.has_value() || item->GetId() != _childItemId.value()))
                {
                    if (!_childMenuToShow)
                    {
                        _childMenuToShow = item->GetId();
                        _childHoverStartTime = ztime::Main();
                    }
                }
            }

            return std::move(targets.Add(this, point));
        }

        EventContext _OnLeftPressed(Point point) override
        {
            auto targets = Panel::_OnLeftPressed(point);
            Component* mainTarget = targets.MainTarget();
            auto it = std::find_if(_items.begin(), _items.end(), [mainTarget](Item& item) { return item.item == mainTarget; });
            if (it != _items.end())
            {
                MenuItem* item = (MenuItem*)it->item;

                // Adjust nested menu opening timer to open immediatelly
                _childHoverEndTime = ztime::Main() - _hoverToShowDuration;
                _childHoverStartTime = ztime::Main() - _hoverToShowDuration;

                if (!item->disabled)
                {
                    // Handle checkable items
                    if (item->checkable)
                    {
                        if (item->checkGroup == -1)
                        {
                            item->Invoke(!item->checked);
                            item->checked = !item->checked;
                        }
                        else
                        {
                            if (!item->checked)
                            {
                                // Uncheck others from same group
                                for (int i = 0; i < _items.size(); i++)
                                {
                                    MenuItem* mItem = (MenuItem*)_items[i].item;
                                    if (mItem->checkGroup == item->checkGroup && mItem->checked)
                                        mItem->checked = false;
                                }
                                item->Invoke(true);
                                item->checked = true;
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
                    !item->disabled &&
                    item->closeOnClick)
                    FullClose();
            }

            return std::move(targets.Add(this, point));
        }

        void _OnMouseLeave() override
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
                        _hoveredItem->backgroundColor = Color();
                        _hoveredItem = nullptr;
                    }
                }
            }
            else
            {
                // Unhighlight item
                if (_hoveredItem)
                {
                    _hoveredItem->backgroundColor = Color();
                    _hoveredItem = nullptr;
                }
            }
        }

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicIntValueProxy<int>("min width", std::make_any<Value<int>*>(&minWidth), ValueProxy::Number(0)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("max width", std::make_any<Value<int>*>(&maxWidth), ValueProxy::Number(0)));

            auto data = FlexPanel::GetReflectionData();
            data.insert(data.begin(), { "Menu panel", std::move(values) });
            return data;
        }
    };
}