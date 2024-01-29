#pragma once

#include "ComponentBase.h"

#include <algorithm>
#include <functional>
#include <optional>

namespace zcom
{
    class PROP_Shadow : public Property
    {
        void _CopyFields(const PROP_Shadow& other)
        {
            valid = other.valid;
            offsetX = other.offsetX;
            offsetY = other.offsetY;
            blurStandardDeviation = other.blurStandardDeviation;
            color = other.color;
        }
    public:
        static std::string _NAME_() { return "shadow"; }

        PROP_Shadow() {}
        PROP_Shadow(const PROP_Shadow& other)
        {
            _CopyFields(other);
        }
        PROP_Shadow& operator=(const PROP_Shadow& other)
        {
            _CopyFields(other);
            return *this;
        }

        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float blurStandardDeviation = 3.0f;
        D2D1_COLOR_F color = D2D1::ColorF(0, 0.75f);
    };

    class Panel : public Component
    {
#pragma region base_class
    protected:
        void _OnUpdate() override
        {
            for (auto& item : _items)
            {
                item.item->Update();
            }
        }

        bool _Redraw() override
        {
            for (auto& item : _items)
            {
                if (item.item->Redraw())
                {
                    return true;
                }
            }
            return false;
        }

        void _OnDraw(Graphics g) override
        {
            // Any external transforms should be applied only to the final image produced by composing items in the panel
            // Stash the current transform and restore it after rendering child items
            D2D1_MATRIX_3X2_F originalTransform;
            g.target->GetTransform(&originalTransform);
            g.target->SetTransform(D2D1::Matrix3x2F::Identity());

            // Get bitmaps of all items
            std::list<std::pair<ID2D1Bitmap*, Component*>> bitmaps;
            for (auto& item : _items)
            {
                // Order by z-index
                auto it = bitmaps.rbegin();
                for (; it != bitmaps.rend(); it++)
                {
                    if (item.item->GetZIndex() >= it->second->GetZIndex())
                    {
                        break;
                    }
                }
                if (item.item->Redraw())
                    item.item->Draw(g);
                bitmaps.insert(it.base(), { item.item->ContentImage(), item.item });
            }

            g.target->SetTransform(originalTransform);

            // Draw the bitmaps
            for (auto& it : bitmaps)
            {
                if (it.first == nullptr || it.second->GetOpacity() <= 0.0f || !it.second->GetVisible())
                    continue;

                // Draw shadow
                auto prop = it.second->GetProperty<PROP_Shadow>();
                if (prop.valid)
                {
                    ID2D1Effect* shadowEffect = nullptr;
                    g.target->CreateEffect(CLSID_D2D1Shadow, &shadowEffect);
                    shadowEffect->SetInput(0, it.first);
                    shadowEffect->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1::Vector4F(prop.color.r, prop.color.g, prop.color.b, prop.color.a));
                    shadowEffect->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, prop.blurStandardDeviation);

                    if (it.second->GetOpacity() < 1.0f)
                    {
#ifdef CLSID_D2D1Opacity
                        ID2D1Effect* opacityEffect = nullptr;
                        g.target->CreateEffect(CLSID_D2D1Opacity, &opacityEffect);
                        opacityEffect->SetValue(D2D1_OPACITY_PROP_OPACITY, it.second->GetOpacity());

                        ID2D1Effect* compositeEffect = nullptr;
                        g.target->CreateEffect(CLSID_D2D1Composite, &compositeEffect);
                        compositeEffect->SetInputEffect(0, shadowEffect);
                        compositeEffect->SetInputEffect(1, opacityEffect);

                        g.target->DrawImage(compositeEffect, D2D1::Point2F(it.second->GetX() + prop.offsetX, it.second->GetY() + prop.offsetY));
                        compositeEffect->Release();
                        opacityEffect->Release();
#else
                        // Draw to separate render target and use 'DrawBitmap' with opacity
                        ID2D1Image* stash = nullptr;
                        ID2D1Bitmap1* contentBitmap = nullptr;
                        g.target->CreateBitmap(
                            D2D1::SizeU(GetWidth(), GetHeight()),
                            nullptr,
                            0,
                            D2D1::BitmapProperties1(
                                D2D1_BITMAP_OPTIONS_TARGET,
                                { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
                            ),
                            &contentBitmap
                        );

                        g.target->GetTarget(&stash);
                        g.target->SetTarget(contentBitmap);
                        g.target->Clear();
                        g.target->DrawImage(shadowEffect, D2D1::Point2F(it.second->GetX() + prop.offsetX, it.second->GetY() + prop.offsetY));
                        g.target->SetTarget(stash);
                        stash->Release();
                        g.target->DrawBitmap(contentBitmap, (const D2D1_RECT_F*)0, it.second->GetOpacity());

                        contentBitmap->Release();
#endif
                    }
                    else
                    {
                        g.target->DrawImage(shadowEffect, D2D1::Point2F(it.second->GetX() + prop.offsetX, it.second->GetY() + prop.offsetY));
                    }
                    shadowEffect->Release();
                }

                g.target->DrawBitmap(
                    it.first,
                    D2D1::RectF(
                        it.second->GetX(),
                        it.second->GetY(),
                        it.second->GetX() + it.second->GetWidth(),
                        it.second->GetY() + it.second->GetHeight()
                    ),
                    it.second->GetOpacity()
                );
            }
        }

        void _OnResize(int width, int height) override
        {
            _RecalculateLayout(width, height);
        }

        void _OnWindowPosChange(int x, int y) override
        {
            _SetWindowPositions();
        }

        EventTargets _OnMouseMove(int x, int y, int deltaX, int deltaY) override
        {
            std::vector<Component*> hoveredComponents;
            //Base* handledItem = nullptr;
            bool itemHandled = false;
            EventTargets targets;
            for (auto& _item : _items)
            {
                Component* item = _item.item;

                if (!item->GetVisible()) continue;

                if (x >= item->GetX() && x < item->GetX() + item->GetWidth() &&
                    y >= item->GetY() && y < item->GetY() + item->GetHeight())
                {
                    if (item->GetMouseLeftClicked() || item->GetMouseRightClicked())
                    {
                        if (!itemHandled)
                        {
                            targets = item->OnMouseMove(x - item->GetX(), y - item->GetY());
                            itemHandled = true;
                        }
                    }
                    hoveredComponents.push_back(item);
                    if (!item->GetMouseInsideArea())
                    {
                        item->OnMouseEnterArea();
                    }
                }
                else
                {
                    if (item->GetMouseInside())
                    {
                        if (item->GetMouseLeftClicked() || item->GetMouseRightClicked())
                        {

                            if (!itemHandled)
                            {
                                targets = item->OnMouseMove(x - item->GetX(), y - item->GetY());
                                itemHandled = true;
                            }
                        }
                        else
                        {
                            item->OnMouseLeave();
                        }
                    }
                    if (item->GetMouseInsideArea())
                    {
                        item->OnMouseLeaveArea();
                    }
                }
            }
            if (itemHandled)
                return std::move(targets.Add(this, x, y));

            //std::cout << hoveredComponents.size() << std::endl;

            if (!hoveredComponents.empty())
            {
                // Sort components in ascending z-index order
                std::sort(hoveredComponents.begin(), hoveredComponents.end(), [](zcom::Component* item1, zcom::Component* item2) { return item1->GetZIndex() > item2->GetZIndex(); });

                bool eventHandled = false;
                EventTargets result;
                for (auto& item : hoveredComponents)
                {
                    if (!eventHandled)
                    {
                        result = item->OnMouseMove(x - item->GetX(), y - item->GetY());
                        if (!result.Empty())
                        {
                            eventHandled = true;
                            continue;
                        }
                    }
                    if (item->GetMouseInside())
                        item->OnMouseLeave();
                }
                return std::move(result.Add(this, x, y));

                //Component* topmost = hoveredComponents[0];
                //for (int i = 1; i < hoveredComponents.size(); i++)
                //{
                //    if (hoveredComponents[i]->GetZIndex() > topmost->GetZIndex())
                //    {
                //        topmost = hoveredComponents[i];
                //    }
                //}

                //for (int i = 0; i < hoveredComponents.size(); i++)
                //{
                //    if (hoveredComponents[i] != topmost && hoveredComponents[i]->GetMouseInside())
                //    {
                //        hoveredComponents[i]->OnMouseLeave();
                //    }
                //}

                //if (!topmost->GetMouseInside())
                //{
                //    topmost->OnMouseEnter();
                //}
                //return topmost->OnMouseMove(adjX - topmost->GetX(), adjY - topmost->GetY()).Add(this, GetMousePosX(), GetMousePosY());
            }

            if (_fallthroughMouseEvents)
                return targets;
            else
                return std::move(targets.Add(this, x, y));

            //for (auto& _item : _items)
            //{
            //    Base* item = _item.item;

            //    if (adjX >= item->GetX() && adjX < item->GetX() + item->GetWidth() &&
            //        adjY >= item->GetY() && adjY < item->GetY() + item->GetHeight())
            //    {
            //        if (!item->GetMouseInside())
            //        {
            //            item->OnMouseEnter();
            //        }
            //        item->OnMouseMove(adjX - item->GetX(), adjY - item->GetY());
            //    }
            //    else if (item->GetMouseInside())
            //    {
            //        if (item->GetMouseLeftClicked() || item->GetMouseRightClicked())
            //        {
            //            item->OnMouseMove(adjX - item->GetX(), adjY - item->GetY());
            //        }
            //        else
            //        {
            //            item->OnMouseLeave();
            //        }
            //    }
            //}
        }

        void _OnMouseLeave() override
        {
            //std::cout << "Mouse leave\n";
            for (auto& item : _items)
            {
                if (item.item->GetMouseInside())
                {
                    item.item->OnMouseLeave();
                }
            }
        }

        void _OnMouseEnter() override
        {

            //std::cout << "Mouse enter\n";
        }

        void _OnMouseLeaveArea() override
        {
            //std::cout << "Mouse leave\n";
            for (auto& item : _items)
            {
                if (item.item->GetMouseInsideArea())
                {
                    item.item->OnMouseLeaveArea();
                }
            }
        }

        void _OnMouseEnterArea() override
        {

            //std::cout << "Mouse enter\n";
        }

        EventTargets _OnLeftPressed(int x, int y) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->GetVisible())
                    continue;
                if (item->GetMouseInside())
                    return item->OnLeftPressed(x - item->GetX(), y - item->GetY()).Add(this, x, y);
            }
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnLeftReleased(int x, int y) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->GetVisible())
                    continue;
                if (item->GetMouseInside())
                    return item->OnLeftReleased(x - item->GetX(), y - item->GetY()).Add(this, x, y);
            }
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnRightPressed(int x, int y) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->GetVisible())
                    continue;
                if (item->GetMouseInside())
                    return item->OnRightPressed(x - item->GetX(), y - item->GetY()).Add(this, x, y);
            }
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnRightReleased(int x, int y) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->GetVisible())
                    continue;
                if (item->GetMouseInside())
                    return item->OnRightReleased(x - item->GetX(), y - item->GetY()).Add(this, x, y);
            }
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnWheelUp(int x, int y) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->GetVisible())
                    continue;
                if (item->GetMouseInside())
                    return item->OnWheelUp(x - item->GetX(), y - item->GetY());
            }
            return EventTargets();
        }

        EventTargets _OnWheelDown(int x, int y) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->GetVisible())
                    continue;
                if (item->GetMouseInside())
                    return item->OnWheelDown(x - item->GetX(), y - item->GetY());
            }
            return EventTargets();
        }

        void _OnSelected(bool reverse) override
        {
            for (int i = reverse ? _selectableItems.size() - 1 : 0;
                         reverse ? i >= 0                      : i < _selectableItems.size();
                         reverse ? i--                         : i++)
            {
                if (!_selectableItems[i]->GetVisible() || !_selectableItems[i]->GetActive())
                    continue;

                OnDeselected();
                _selectableItems[i]->OnSelected(reverse);
                break;
            }
        }

    public:
        std::list<Component*> GetChildren() override
        {
            std::list<Component*> children;
            for (auto& item : _items)
            {
                children.push_back(item.item);
            }
            return children;
        }

        std::list<Component*> GetAllChildren() override
        {
            std::list<Component*> children;
            for (auto& item : _items)
            {
                children.push_back(item.item);
                auto itemChildren = item.item->GetAllChildren();
                if (!itemChildren.empty())
                {
                    children.insert(children.end(), itemChildren.begin(), itemChildren.end());
                }
            }
            return children;
        }

        Component* IterateTab(bool reverse) override
        {
            if (_selectableItems.empty())
            {
                if (Selected())
                    return nullptr;
                else
                    return this;
            }

            // While searching == true, _selectableItems is iterated until the first
            // available to select item is found (IterateTab() returns itself). If no
            // available item is found return null to signal end of tab selection.
            // 'Available' means visible, active, can be selected (returns !nullptr).
            bool searching = false;
            
            for (int i = reverse ? _selectableItems.size() - 1 : 0;
                         reverse ? i >= 0                      : i < _selectableItems.size();
                         reverse ? i--                         : i++)
            {
                if (!_selectableItems[i]->GetVisible() || !_selectableItems[i]->GetActive())
                    continue;

                Component* item = _selectableItems[i]->IterateTab(reverse);
                if (item == nullptr)
                {
                    if (!searching)
                    {
                        searching = true;
                        continue;
                    }
                }
                else if (item != _selectableItems[i])
                {
                    return item;
                }
                else if (searching)
                {
                    return item;
                }
            }

            if (searching)
                return nullptr;

            return this;
        }

        const char* GetName() const override { return Name(); }
        static const char* Name() { return "panel"; }
#pragma endregion

    protected:
        virtual void _RecalculateLayout(int width, int height)
        {
            int widthWithoutPadding = GetWidth() - _padding.left - _padding.right;
            int heightWithoutPadding = GetHeight() - _padding.top - _padding.bottom;

            // Calculate item sizes and positions
            int maxRightEdge = 0;
            int maxBottomEdge = 0;
            for (auto& _item : _items)
            {
                Component* item = _item.item;

                int newWidth = (int)std::round(widthWithoutPadding * item->GetParentWidthPercent()) + item->GetBaseWidth();
                int newHeight = (int)std::round(heightWithoutPadding * item->GetParentHeightPercent()) + item->GetBaseHeight();
                if (newWidth < 1)
                    newWidth = 1;
                if (newHeight < 1)
                    newHeight = 1;

                int newPosX = 0;
                if (item->GetHorizontalAlignment() == Alignment::START)
                    newPosX = std::round((widthWithoutPadding - newWidth) * item->GetHorizontalOffsetPercent());
                else if (item->GetHorizontalAlignment() == Alignment::CENTER)
                    newPosX = (widthWithoutPadding - newWidth) / 2;
                else if (item->GetHorizontalAlignment() == Alignment::END)
                    newPosX = std::round((widthWithoutPadding - newWidth) * (1.0f - item->GetHorizontalOffsetPercent()));
                newPosX += item->GetHorizontalOffsetPixels();
                newPosX += _padding.left;

                int newPosY = 0;
                if (item->GetVerticalAlignment() == Alignment::START)
                    newPosY = std::round((heightWithoutPadding - newHeight) * item->GetVerticalOffsetPercent());
                else if (item->GetVerticalAlignment() == Alignment::CENTER)
                    newPosY = (heightWithoutPadding - newHeight) / 2;
                else if (item->GetVerticalAlignment() == Alignment::END)
                    newPosY = std::round((heightWithoutPadding - newHeight) * (1.0f - item->GetVerticalOffsetPercent()));
                newPosY += item->GetVerticalOffsetPixels();
                newPosY += _padding.top;

                item->SetPosition(newPosX, newPosY);
                item->Resize(newWidth, newHeight);

                if (newPosX + newWidth > maxRightEdge)
                    maxRightEdge = newPosX + newWidth;
                if (newPosY + newHeight > maxBottomEdge)
                    maxBottomEdge = newPosY + newHeight;
            }
            _contentWidth = maxRightEdge + _padding.right;
            _contentHeight = maxBottomEdge + _padding.bottom;

            _SetWindowPositions();
            InvokeRedraw();
        }

        virtual void _SetWindowPositions()
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                item->SetWindowPosition(
                    GetWindowX() + item->GetX(),
                    GetWindowY() + item->GetY()
                );
            }
        }

        struct Item
        {
            Component* item;
            bool owned;
            EventSubscription<void> layoutChangeHandler;
            EventSubscription<void, Component*, bool> selectHandler;
        };
        std::vector<Item> _items;
    private:
        std::vector<Component*> _selectableItems;

        // Child placement
        RECT _padding = { 0, 0, 0, 0 };
        int _contentWidth = 0;
        int _contentHeight = 0;

        // Auto child resize
        bool _deferUpdates = false;
        bool _updatesDeferred = false;

        bool _fallthroughMouseEvents = false;
        

    protected:
        friend class Scene;
        friend class Component;
        Panel(Scene* scene) : Component(scene) {}
        void Init()
        {
            // By default allow iterating nested components
            SetTabIndex(0);
        }
    public:
        ~Panel()
        {
            ClearItems();
        }
        Panel(Panel&&) = delete;
        Panel& operator=(Panel&&) = delete;
        Panel(const Panel&) = delete;
        Panel& operator=(const Panel&) = delete;

    protected:
        virtual void _AddItem(Component* item, bool transferOwnership)
        {
            _items.push_back({ item, transferOwnership });

            // Add layout change handler
            _items.back().layoutChangeHandler = item->SubscribeOnLayoutChanged([&, item]()
            {
                if (_deferUpdates)
                {
                    _updatesDeferred = true;
                    return;
                }
                ExecuteSynchronously([=]() {
                    _RecalculateLayout(GetWidth(), GetHeight());
                    if (GetMouseInside())
                        OnMouseMove(GetMousePosX(), GetMousePosY());
                });
            });

            // Add selection event bubbling
            // This is mainly done to enable scroll panels to scroll to nested selected components
            _items.back().selectHandler = item->SubscribeOnSelected([&](zcom::Component* srcItem, bool reverse)
            {
                _onSelected->InvokeAll(srcItem, reverse);
            });

            ReindexTabOrder();
            if (_deferUpdates)
            {
                _updatesDeferred = true;
                return;
            }
            _RecalculateLayout(GetWidth(), GetHeight());
            if (GetMouseInside())
                OnMouseMove(GetMousePosX(), GetMousePosY());
        }

    public:
        void AddItem(Component* item)
        {
            _AddItem(item, false);
        }

        void AddItem(std::unique_ptr<Component> item)
        {
            _AddItem(item.release(), true);
        }

        void RemoveItem(Component* item)
        {
            for (int i = 0; i < _items.size(); i++)
            {
                if (_items[i].item == item)
                {
                    if (_items[i].owned)
                        delete _items[i].item;
                    _items.erase(_items.begin() + i);
                    ReindexTabOrder();
                    if (_deferUpdates)
                    {
                        _updatesDeferred = true;
                        return;
                    }
                    _RecalculateLayout(GetWidth(), GetHeight());
                    if (GetMouseInsideArea())
                        OnMouseMove(GetMousePosX(), GetMousePosY());
                    return;
                }
            }
        }

        void RemoveItem(int index)
        {
            if (_items[index].owned)
                delete _items[index].item;
            _items.erase(_items.begin() + index);
            ReindexTabOrder();
            if (_deferUpdates)
            {
                _updatesDeferred = true;
                return;
            }
            _RecalculateLayout(GetWidth(), GetHeight());
            if (GetMouseInside())
                OnMouseMove(GetMousePosX(), GetMousePosY());
        }

        int ItemCount() const
        {
            return _items.size();
        }

        Component* GetItem(int index)
        {
            return _items[index].item;
        }

        void ClearItems()
        {
            for (int i = 0; i < _items.size(); i++)
            {
                if (_items[i].owned)
                {
                    delete _items[i].item;
                }
            }
            _items.clear();
            _selectableItems.clear();
            if (_deferUpdates)
            {
                _updatesDeferred = true;
                return;
            }
            _RecalculateLayout(GetWidth(), GetHeight());
            //OnMouseMove(GetMousePosX(), GetMousePosY());
        }

        // Calculates the offset from the top left corner of this panel to the top left corner of target child
        // Performs a recursive DFS search; If the specified child is not found, returns nullopt
        std::optional<std::pair<int, int>> FindChildRelativeOffset(Component* child)
        {
            return _FindChildRelativeOffset(this, child);
        }

    private:
        std::optional<std::pair<int, int>> _FindChildRelativeOffset(Component* parent, Component* child)
        {
            auto children = parent->GetChildren();
            for (auto item : children)
            {
                if (item == child)
                    return std::make_pair(item->GetX(), item->GetY());

                std::optional<std::pair<int, int>> result = _FindChildRelativeOffset(item, child);
                if (result)
                    return std::make_pair(item->GetX() + result.value().first, item->GetY() + result.value().second);
            }
            return std::nullopt;
        }

    public:

        // Calling this function suppresses layout updates on item add/remove/layout change
        // until 'ResumeLayoutUpdates()' is called, at which point the deferred updates are executed.
        // Useful when doing lots of item manipulation, which causes many layout updates
        // when only 1 is required at the end.
        void DeferLayoutUpdates()
        {
            _deferUpdates = true;
            _updatesDeferred = false;
        }

        // Enables reactive layout updates. Any deferred updates are executed, unless 'executePending' is false.
        void ResumeLayoutUpdates(bool executePending = true)
        {
            _deferUpdates = false;
            if (_updatesDeferred && executePending)
            {
                ExecuteSynchronously([=]() {
                    _RecalculateLayout(GetWidth(), GetHeight());
                    if (GetMouseInside())
                        OnMouseMove(GetMousePosX(), GetMousePosY());
                });
            }
            _updatesDeferred = false;
        }

        // When event fallthrough is enabled, the panel will not handle mouse events that don't hit any nested components
        void EnableMouseEventFallthrough()
        {
            _fallthroughMouseEvents = true;

            // If the mouse is currently holding the panel, and not a nested component,
            // manually invoke mouse release, since the panel itself will be uninteractable
            bool insideNestedComponent = false;
            for (auto& item : _items)
            {
                if (item.item->GetMouseInside())
                {
                    insideNestedComponent = true;
                    break;
                }
            }
            if (!insideNestedComponent)
            {
                OnLeftReleased();
                OnRightReleased();
            }

            // Invoke mouse move resending on parent component
            _onLayoutChanged->InvokeAll();
        }

        void DisableMouseEventFallthrough()
        {
            _fallthroughMouseEvents = false;

            // Invoke mouse move resending on parent component
            _onLayoutChanged->InvokeAll();
        }

        void ReindexTabOrder()
        {
            _selectableItems.clear();
            for (int i = 0; i < _items.size(); i++)
            {
                if (_items[i].item->GetTabIndex() != -1)
                {
                    _selectableItems.push_back(_items[i].item);
                }
            }

            // Sort indices
            std::sort(_selectableItems.begin(), _selectableItems.end(), [](Component* a, Component* b) { return a->GetTabIndex() < b->GetTabIndex(); });

            // Remove duplicates
            //for (int i = 1; i < _selectableItems.size(); i++)
            //{
            //    if (_selectableItems[i - 1]->GetTabIndex() == _selectableItems[i]->GetTabIndex())
            //    {
            //        _selectableItems.erase(_selectableItems.begin() + i);
            //        i--;
            //    }
            //}
        }

        RECT GetPadding() const
        {
            return _padding;
        }

        void SetPadding(RECT padding)
        {
            if (_padding != padding)
            {
                _padding = padding;
                if (_deferUpdates)
                {
                    _updatesDeferred = true;
                    return;
                }
                _RecalculateLayout(GetWidth(), GetHeight());
            }
        }

        int GetContentWidth() const
        {
            return _contentWidth;
        }

        int GetContentHeight() const
        {
            return _contentHeight;
        }
    };
}