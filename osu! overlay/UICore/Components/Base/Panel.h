#pragma once

#include "ComponentBase.h"

#include <algorithm>
#include <functional>
#include <optional>

#define HIDE_PANEL_METHODS \
protected: \
using Panel::AddItem; \
using Panel::InsertItem; \
using Panel::RemoveItem; \
using Panel::ItemCount; \
using Panel::GetItem; \
using Panel::ClearItems; \
using Panel::FindChildRelativeOffset;

namespace zcom
{
    // Components with the property have a box shadow rendered below them
    // Components with the same group (excluding nullopt) will have a single shadow drawn
    // This is useful when the components overlap, but a single shadow below the components is required
    class Shadow : public Property
    {
        void _CopyFields(const Shadow& other)
        {
            valid = other.valid;
            offsetX = other.offsetX;
            offsetY = other.offsetY;
            blurStandardDeviation = other.blurStandardDeviation;
            color = other.color;
            group = other.group;
        }
    public:
        static std::string _NAME_() { return "shadow"; }

        Shadow() {}
        Shadow(float offsetX, float offsetY, float blurStandardDeviation, Color color, std::optional<int> group = std::nullopt)
            : offsetX(offsetX), offsetY(offsetY), blurStandardDeviation(blurStandardDeviation), color(color) {}
        Shadow(const Shadow& other)
        {
            _CopyFields(other);
        }
        Shadow& operator=(const Shadow& other)
        {
            _CopyFields(other);
            return *this;
        }

        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float blurStandardDeviation = 3.0f;
        Color color = Color(0, 0.75f);
        std::optional<int> group = std::nullopt;

        Shadow& WithOffsetX(float offset) { offsetX = offset; return *this; }
        Shadow& WithOffsetY(float offset) { offsetY = offset; return *this; }
        Shadow& WithBlurStandardDeviation(float amount) { blurStandardDeviation = amount; return *this; }
        Shadow& WithColor(Color color) { this->color = color; return *this; }
        Shadow& WithGroup(int group) { this->group = group; return *this; }
    };

    class Panel : public Component
    {
        DEFINE_COMPONENT(Panel, Component)
    public:
        ~Panel()
        {
            DeferLayoutUpdates(); // Do not do layout updates, since the panel is being destroyed anyway
            ClearItems();
        }
    protected:
        void Init()
        {
            // By default allow iterating nested components
            tabIndex = 0;
        }

    public:

        Value<Rect> padding = Value<Rect>({ 0, 0, 0, 0 }, [=](Rect& currentValue, const Rect& padding) {
            currentValue = padding;
            if (_deferUpdates)
            {
                _updatesDeferred = true;
                return;
            }
            _RecalculateLayoutWithMouseAdjust();
        });
        // When event fallthrough is enabled, the panel will not handle mouse events that don't hit any nested components
        Value<bool> fallthroughMouseEvents = Value<bool>(false, [=](bool& currentValue, const bool& fallthrough) {
            currentValue = fallthrough;

            if (fallthrough)
            {
                // If the mouse is currently holding the panel, and not a nested component,
                // manually invoke mouse release, since the panel itself will be uninteractable
                bool insideNestedComponent = false;
                for (auto& item : _items)
                {
                    if (item.item->hovered_)
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
            }

            // Invoke mouse move resending on parent component
            _onLayoutChanged->InvokeAll();
        });

        Value<Size> contentSize_ = Size{ 0, 0 };


        void AddItem(Component* item)
        {
            _AddItem(item, _items.size(), false);
        }

        void AddItem(std::unique_ptr<Component> item)
        {
            _AddItem(item.release(), _items.size(), true);
        }

        void InsertItem(Component* item, size_t position)
        {
            _AddItem(item, position, false);
        }

        void InsertItem(std::unique_ptr<Component> item, size_t position)
        {
            _AddItem(item.release(), position, true);
        }

        void InsertItemAfter(Component* item, Component* other)
        {
            for (size_t i = 0; i < _items.size(); i++)
            {
                if (_items[i].item == other)
                {
                    _AddItem(item, i + 1, false);
                    return;
                }
            }
            _AddItem(item, _items.size(), false);
        }

        void InsertItemAfter(std::unique_ptr<Component> item, Component* other)
        {
            for (size_t i = 0; i < _items.size(); i++)
            {
                if (_items[i].item == other)
                {
                    _AddItem(item.release(), i + 1, true);
                    return;
                }
            }
            _AddItem(item.release(), _items.size(), true);
        }

        void InsertItemBefore(Component* item, Component* other)
        {
            for (size_t i = 0; i < _items.size(); i++)
            {
                if (_items[i].item == other)
                {
                    _AddItem(item, i, false);
                    return;
                }
            }
            _AddItem(item, _items.size(), false);
        }

        void InsertItemBefore(std::unique_ptr<Component> item, Component* other)
        {
            for (size_t i = 0; i < _items.size(); i++)
            {
                if (_items[i].item == other)
                {
                    _AddItem(item.release(), i, true);
                    return;
                }
            }
            _AddItem(item.release(), _items.size(), true);
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
                    _RecalculateLayoutWithMouseAdjust();
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
            _RecalculateLayoutWithMouseAdjust();
        }

        size_t ItemCount() const
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
            _RecalculateLayoutWithMouseAdjust();
        }

        // Calculates the offset from the top left corner of this panel to the top left corner of target child
        // Performs a recursive DFS search; If the specified child is not found, returns nullopt
        std::optional<Point> FindChildRelativeOffset(Component* child)
        {
            return _FindChildRelativeOffset(this, child);
        }

    private:
        std::optional<Point> _FindChildRelativeOffset(Component* parent, Component* child)
        {
            auto children = parent->GetChildren();
            for (auto item : children)
            {
                if (item == child)
                    return item->position_;

                std::optional<Point> result = _FindChildRelativeOffset(item, child);
                if (result)
                    return item->position_.Get() + result.value();
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
        // Setting 'immediate' to true performs layout updates immediatelly, instead of the next update
        void ResumeLayoutUpdates(bool executePending = true, bool immediate = false)
        {
            _deferUpdates = false;
            if (_updatesDeferred && executePending)
            {
                if (immediate)
                    _RecalculateLayoutWithMouseAdjust();
                else
                    _layoutChanged = true;
            }
            _updatesDeferred = false;
        }

        // Performs any queued layout updates immediatelly, instead of the next update
        void ForceLayoutUpdate()
        {
            _layoutChanged = false;
            _RecalculateLayoutWithMouseAdjust();
        }

        void ReindexTabOrder()
        {
            _selectableItems.clear();
            for (int i = 0; i < _items.size(); i++)
            {
                if (_items[i].item->tabIndex != -1)
                {
                    _selectableItems.push_back(_items[i].item);
                }
            }

            // Sort indices
            std::sort(_selectableItems.begin(), _selectableItems.end(), [](Component* a, Component* b) { return a->tabIndex < b->tabIndex; });

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

    protected:
        struct Item
        {
            Component* item;
            bool owned;
            EventSubscription<void> layoutChangeHandler;
            EventSubscription<void, Component*, bool> selectHandler;
            EventSubscription<void, Component*> destroyHandler;
        };
        std::vector<Item> _items;
    private:
        std::vector<Component*> _selectableItems;

        bool _deferUpdates = false;
        bool _updatesDeferred = false;
        bool _layoutChanged = false;
        bool _recalculatingLayout = false;

    protected:
        virtual void _AddItem(Component* item, size_t position, bool transferOwnership)
        {
            if (position > _items.size())
                position = _items.size();

            auto it = _items.begin() + position;
            _items.insert(it, { item, transferOwnership });

            // Add layout change handler
            _items[position].layoutChangeHandler = item->SubscribeOnLayoutChanged([&, item]() {
                if (_deferUpdates)
                {
                    _updatesDeferred = true;
                    return;
                }
                _layoutChanged = true;
            });
            // Add selection event bubbling
            // This is mainly done to enable scroll panels to scroll to nested selected components
            _items[position].selectHandler = item->SubscribeOnSelected([&](Component* srcItem, bool reverse) {
                _onSelected->InvokeAll(srcItem, reverse);
            });
            if (!transferOwnership)
            {
                _items[position].destroyHandler = item->SubscribeOnDestroyed([=](Component* item) {
                    RemoveItem(item);
                });
            }

            ReindexTabOrder();
            if (_deferUpdates)
            {
                _updatesDeferred = true;
                return;
            }
            _RecalculateLayoutWithMouseAdjust();
        }

        void _RecalculateLayoutWithMouseAdjust()
        {
            _RecalculateLayout();
            if (hovered_)
                OnMouseMove(mousePosition_);
        }

        void _RecalculateLayout()
        {
            int cycleCount = 0;
            while (cycleCount < 10)
            {
                _ComputeItemLayout();
                if (_layoutChanged)
                {
                    _layoutChanged = false;
                    continue;
                }
                return;
            }
            std::cout << "Failed to reach stable layout after " << cycleCount << " cycles\n";
        }

        virtual void _ComputeItemLayout()
        {
            int widthWithoutPadding = size_->width - padding->left - padding->right;
            int heightWithoutPadding = size_->height - padding->top - padding->bottom;

            // Calculate item sizes and positions
            int maxRightEdge = 0;
            int maxBottomEdge = 0;
            for (auto& _item : _items)
            {
                Component* item = _item.item;

                int newWidth = (int)std::round(widthWithoutPadding * item->parentSize->width) + item->size->width + item->selfSize_->width;
                int newHeight = (int)std::round(heightWithoutPadding * item->parentSize->height) + item->size->height + item->selfSize_->height;
                if (newWidth < 0)
                    newWidth = 0;
                if (newHeight < 0)
                    newHeight = 0;

                int newPosX = 0;
                if (item->xAlign == Alignment::START)
                    newPosX = (int)std::round((widthWithoutPadding - newWidth) * item->parentPosition->x);
                else if (item->xAlign == Alignment::CENTER)
                    newPosX = (widthWithoutPadding - newWidth) / 2;
                else if (item->xAlign == Alignment::END)
                    newPosX = (int)std::round((widthWithoutPadding - newWidth) * (1.0f - item->parentPosition->x));
                newPosX += item->position->x;
                newPosX += padding->left;

                int newPosY = 0;
                if (item->yAlign == Alignment::START)
                    newPosY = (int)std::round((heightWithoutPadding - newHeight) * item->parentPosition->y);
                else if (item->yAlign == Alignment::CENTER)
                    newPosY = (heightWithoutPadding - newHeight) / 2;
                else if (item->yAlign == Alignment::END)
                    newPosY = (int)std::round((heightWithoutPadding - newHeight) * (1.0f - item->parentPosition->y));
                newPosY += item->position->y;
                newPosY += padding->top;

                item->SetPosition({ newPosX, newPosY });
                item->Resize({ newWidth, newHeight });

                if (newPosX + newWidth > maxRightEdge)
                    maxRightEdge = newPosX + newWidth;
                if (newPosY + newHeight > maxBottomEdge)
                    maxBottomEdge = newPosY + newHeight;
            }
            contentSize_ = {
                maxRightEdge + padding->right,
                maxBottomEdge + padding->bottom
            };

            _SetWindowPositions();
            InvokeRedraw();
        }

        virtual void _SetWindowPositions()
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                item->SetWindowPosition(windowPosition_.Get() + item->position_);
            }
        }

    protected:
        void _OnUpdate() override
        {
            if (_layoutChanged)
            {
                _layoutChanged = false;
                _RecalculateLayoutWithMouseAdjust();
            }

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

        void _OnDraw(Graphics* g) override;

        struct DrawParams
        {
            PointF contentOffset = { 0.0f, 0.0f };
        };

        void _OnDraw(Graphics* g, DrawParams params);

        void _OnResize(Size size) override
        {
            _RecalculateLayoutWithMouseAdjust();
        }

        void _OnWindowPosChange(Point position) override
        {
            _SetWindowPositions();
        }

        EventContext _OnMouseMove(Point point, Point deltaPos) override
        {
            std::vector<Component*> hoveredComponents;
            //Base* handledItem = nullptr;
            bool itemHandled = false;
            EventContext targets;
            for (auto& _item : _items)
            {
                Component* item = _item.item;

                if (!item->visible || !item->interactable)
                    continue;

                if (point.x >= item->position_->x && point.x < item->position_->x + item->size_->width &&
                    point.y >= item->position_->y && point.y < item->position_->y + item->size_->height)
                {
                    if (item->leftClicked_ || item->rightClicked_)
                    {
                        if (!itemHandled)
                        {
                            targets = item->OnMouseMove(point - item->position_);
                            itemHandled = true;
                        }
                    }
                    hoveredComponents.push_back(item);
                    if (!item->hoveredArea_)
                        item->OnMouseEnterArea();
                }
                else
                {
                    if (item->hovered_)
                    {
                        if (item->leftClicked_ || item->rightClicked_)
                        {

                            if (!itemHandled)
                            {
                                targets = item->OnMouseMove(point - item->position_);
                                itemHandled = true;
                            }
                        }
                        else
                        {
                            item->OnMouseLeave();
                        }
                    }
                    if (item->hoveredArea_)
                        item->OnMouseLeaveArea();
                }
            }
            if (itemHandled)
                return std::move(targets.Add(this, point));

            //std::cout << hoveredComponents.size() << std::endl;

            if (!hoveredComponents.empty())
            {
                // Sort components in ascending z-index order
                std::sort(hoveredComponents.begin(), hoveredComponents.end(), [](zcom::Component* item1, zcom::Component* item2) { return item1->zIndex > item2->zIndex; });

                bool eventHandled = false;
                EventContext result;
                for (auto& item : hoveredComponents)
                {
                    if (!eventHandled)
                    {
                        result = item->OnMouseMove(point - item->position_);
                        if (!result.Empty())
                        {
                            eventHandled = true;
                            continue;
                        }
                    }
                    if (item->hovered_)
                        item->OnMouseLeave();
                }
                return std::move(result.Add(this, point));
            }

            if (fallthroughMouseEvents)
                return targets;
            else
                return std::move(targets.Add(this, point));
        }

        void _OnMouseLeave() override
        {
            //std::cout << "Mouse leave\n";
            for (auto& item : _items)
            {
                if (item.item->hovered_)
                    item.item->OnMouseLeave();
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
                if (item.item->hoveredArea_)
                    item.item->OnMouseLeaveArea();
            }
        }

        void _OnMouseEnterArea() override
        {

            //std::cout << "Mouse enter\n";
        }

        EventContext _OnLeftPressed(Point point) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->visible || !item->interactable)
                    continue;
                if (item->hovered_)
                    return item->OnLeftPressed(point - item->position_).Add(this, point);
            }
            return EventContext().Add(this, point);
        }

        EventContext _OnLeftReleased(std::optional<Point> point) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->visible || !item->interactable)
                    continue;
                if (item->hovered_)
                {
                    if (point.has_value())
                        return item->OnLeftReleased(point.value() - item->position_).Add(this, point);
                    else
                        return item->OnLeftReleased().Add(this, point);
                }
            }
            return EventContext().Add(this, point);
        }

        EventContext _OnRightPressed(Point point) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->visible || !item->interactable)
                    continue;
                if (item->hovered_)
                    return item->OnRightPressed(point - item->position_).Add(this, point);
            }
            return EventContext().Add(this, point);
        }

        EventContext _OnRightReleased(std::optional<Point> point) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->visible || !item->interactable)
                    continue;
                if (item->hovered_)
                {
                    if (point.has_value())
                        return item->OnRightReleased(point.value() - item->position_).Add(this, point);
                    else
                        return item->OnRightReleased().Add(this, point);
                }
            }
            return EventContext().Add(this, point);
        }

        EventContext _OnWheelUp(Point point) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->visible || !item->interactable)
                    continue;
                if (item->hovered_)
                    return item->OnWheelUp(point - item->position_);
            }
            return EventContext();
        }

        EventContext _OnWheelDown(Point point) override
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->visible || !item->interactable)
                    continue;
                if (item->hovered_)
                    return item->OnWheelDown(point - item->position_);
            }
            return EventContext();
        }

        void _OnSelected(bool reverse) override
        {
            for (size_t i = reverse ? _selectableItems.size() - 1 : 0;
                reverse ? i >= 0 : i < _selectableItems.size();
                reverse ? i-- : i++)
            {
                if (!_selectableItems[i]->visible || _selectableItems[i]->disabled)
                    continue;

                OnDeselected();
                _selectableItems[i]->OnSelected(reverse);
                break;
            }
        }

    public:
        std::vector<Component*> GetChildren() override
        {
            std::vector<Component*> children;
            for (auto& item : _items)
            {
                children.push_back(item.item);
            }
            return children;
        }

        std::vector<Component*> GetAllChildren() override
        {
            std::vector<Component*> children;
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
                if (selected_)
                    return nullptr;
                else
                    return this;
            }

            // While searching == true, _selectableItems is iterated until the first
            // available to select item is found (IterateTab() returns itself). If no
            // available item is found return null to signal end of tab selection.
            // 'Available' means visible, active, can be selected (returns !nullptr).
            bool searching = false;

            for (size_t i = reverse ? _selectableItems.size() - 1 : 0;
                reverse ? i >= 0 : i < _selectableItems.size();
                reverse ? i-- : i++)
            {
                if (!_selectableItems[i]->visible || _selectableItems[i]->disabled)
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

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;

            values.push_back(Rect::LeftValueProxy("left padding", std::make_any<Value<Rect>*>(&padding)));
            values.push_back(Rect::TopValueProxy("top padding", std::make_any<Value<Rect>*>(&padding)));
            values.push_back(Rect::RightValueProxy("right padding", std::make_any<Value<Rect>*>(&padding)));
            values.push_back(Rect::BottomValueProxy("bottom padding", std::make_any<Value<Rect>*>(&padding)));
            values.push_back(ValueProxy::BasicBoolValueProxy("fallthrough mouse events", std::make_any<Value<bool>*>(&fallthroughMouseEvents)));
            values.push_back(Size::WidthValueProxy("calculated content width", std::make_any<Value<Size>*>(&contentSize_)));
            values.push_back(Size::HeightValueProxy("calculated content height", std::make_any<Value<Size>*>(&contentSize_)));

            auto data = Component::GetReflectionData();
            data.insert(data.begin(), { "Panel", std::move(values) });
            return data;
        }
    };
}