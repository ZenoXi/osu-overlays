#pragma once

#include "ComponentBase.h"
#include "Helper/Time.h"

#include <algorithm>
#include <functional>

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

    enum class Scrollbar
    {
        VERTICAL,
        HORIZONTAL
    };

    class Panel : public Component
    {
    private:
        void _UpdateScrollbar(Scrollbar direction)
        {
            _Scrollbar& scrollbar = _GetScrollbar(direction);

            if (scrollbar.hangDuration == 0)
            {
                if (MaxScroll(direction) != 0 && !scrollbar.visible)
                {
                    scrollbar.visible = true;
                    InvokeRedraw();
                }
                else if (MaxScroll(direction) == 0 && scrollbar.visible)
                {
                    scrollbar.visible = false;
                    InvokeRedraw();
                }
            }

            // Fade scrollbar
            if (scrollbar.visible)
            {
                if (scrollbar.hangDuration == 0)
                {
                    if (scrollbar.opacity != 1.0f)
                    {
                        scrollbar.opacity = 1.0f;
                        InvokeRedraw();
                    }
                }
                else
                {
                    Duration timeElapsed = ztime::Main() - scrollbar.showTime;
                    if (timeElapsed > scrollbar.hangDuration)
                    {
                        timeElapsed -= scrollbar.hangDuration;
                        if (timeElapsed > scrollbar.fadeDuration)
                        {
                            scrollbar.visible = false;
                            scrollbar.opacity = 0.0f;
                        }
                        else
                        {
                            float fadeProgress = timeElapsed.GetDuration() / (float)scrollbar.fadeDuration.GetDuration();
                            scrollbar.opacity = 1.0f - powf(fadeProgress, 0.5f);
                        }
                        InvokeRedraw();
                    }
                    else
                    {
                        scrollbar.opacity = 1.0f;
                    }
                }
            }

            // Animate scroll
            if (scrollbar.scrollAnimation.inProgress)
            {
                float timeProgress = (ztime::Main() - scrollbar.scrollAnimation.startTime).GetDuration() / (float)scrollbar.scrollAnimation.duration.GetDuration();
                if (timeProgress >= 1.0f)
                {
                    scrollbar.scrollAnimation.inProgress = false;
                    scrollbar.scrollAmount = scrollbar.scrollAnimation.endPos;
                }
                else
                {
                    float moveProgress;
                    if (scrollbar.scrollAnimation.progressFunction)
                        moveProgress = scrollbar.scrollAnimation.progressFunction(timeProgress);
                    else
                        moveProgress = 1.0f - powf(timeProgress - 1.0f, 2.0f);

                    int startPos = scrollbar.scrollAnimation.startPos;
                    int endPos = scrollbar.scrollAnimation.endPos;
                    scrollbar.scrollAmount = startPos + (endPos - startPos) * moveProgress;
                }
                _RecalculateLayout(GetWidth(), GetHeight());
                // TODO: only resend mouse move messages if mouse is in the component
                OnMouseMove(GetMousePosX(), GetMousePosY());
            }
        }

#pragma region base_class
    protected:
        void _OnUpdate()
        {
            _UpdateScrollbar(Scrollbar::VERTICAL);
            _UpdateScrollbar(Scrollbar::HORIZONTAL);

            bool vHovered = ScrollbarHovered(Scrollbar::VERTICAL) && _verticalScrollbar.interactable;
            if (vHovered != _verticalScrollbar.hovered)
            {
                _verticalScrollbar.hovered = vHovered;
                InvokeRedraw();
            }
            bool hHovered = ScrollbarHovered(Scrollbar::HORIZONTAL) && _horizontalScrollbar.interactable;
            if (hHovered != _horizontalScrollbar.hovered)
            {
                _horizontalScrollbar.hovered = hHovered;
                InvokeRedraw();
            }

            for (auto& item : _items)
            {
                item.item->Update();
            }
        }

        bool _Redraw()
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

        void _OnDraw(Graphics g)
        {
            // Recalculate layout if child layouts changed
            //if (_layoutChanged)
            //{
            //    _RecalculateLayout(GetWidth(), GetHeight());
            //    OnMouseMove(GetMousePosX(), GetMousePosY());
            //    _layoutChanged = false;
            //}

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

            // Draw the bitmaps
            for (auto& it : bitmaps)
            {
                if (it.second->GetOpacity() <= 0.0f || !it.second->GetVisible())
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
                        it.second->GetX() - _horizontalScrollbar.scrollAmount,
                        it.second->GetY() - _verticalScrollbar.scrollAmount,
                        it.second->GetX() - _horizontalScrollbar.scrollAmount + it.second->GetWidth(),
                        it.second->GetY() - _verticalScrollbar.scrollAmount + it.second->GetHeight()
                    ),
                    it.second->GetOpacity()
                );
            }

            // Draw vertical scrollbar
            if ((MaxScroll(Scrollbar::VERTICAL) > 0 && _verticalScrollbar.visible) || _verticalScrollbar.backgroundVisible)
            {
                D2D1_RECT_F hitbox = ScrollbarHitbox(Scrollbar::VERTICAL);

                // Draw scroll background
                if (_verticalScrollbar.backgroundVisible)
                {
                    D2D1_RECT_F backgroundRect{};
                    backgroundRect.left = (float)GetWidth() - ScrollbarWidth(Scrollbar::VERTICAL);
                    backgroundRect.right = (float)GetWidth();
                    backgroundRect.top = 0.0f;
                    backgroundRect.bottom = (float)GetHeight();

                    ID2D1SolidColorBrush* brush = nullptr;
                    g.target->CreateSolidColorBrush(_scrollbarBackgroundColor, &brush);
                    g.target->FillRectangle(backgroundRect, brush);
                    brush->Release();
                }

                // Draw scrollbar
                D2D1_COLOR_F color = _scrollbarColor;
                if (MaxScroll(Scrollbar::VERTICAL) == 0)
                {
                    color = _scrollbarDisabledColor;
                }
                else
                {
                    if (_verticalScrollbar.hovered || _verticalScrollbar.held)
                        color.a = 1.0f;
                    else
                        color.a = 0.5f;
                    if (!_verticalScrollbar.backgroundVisible)
                        color.a *= _verticalScrollbar.opacity;
                }
                ID2D1SolidColorBrush* brush = nullptr;
                g.target->CreateSolidColorBrush(color, &brush);
                D2D1_ROUNDED_RECT scrollbarRect{};
                scrollbarRect.radiusX = 2.0f;
                scrollbarRect.radiusY = 2.0f;
                scrollbarRect.rect = hitbox;
                scrollbarRect.rect.left += 2.0f;
                scrollbarRect.rect.top += 2.0f;
                scrollbarRect.rect.right -= 2.0f;
                scrollbarRect.rect.bottom -= 2.0f;
                g.target->FillRoundedRectangle(scrollbarRect, brush);
                brush->Release();
            }
            // Draw horizontal scrollbar
            if ((MaxScroll(Scrollbar::HORIZONTAL) > 0 && _horizontalScrollbar.visible) || _horizontalScrollbar.backgroundVisible)
            {
                D2D1_RECT_F hitbox = ScrollbarHitbox(Scrollbar::HORIZONTAL);

                // Draw scroll background
                if (_horizontalScrollbar.backgroundVisible)
                {
                    D2D1_RECT_F backgroundRect{};
                    backgroundRect.left = 0.0f;
                    backgroundRect.right = (float)GetWidth();
                    backgroundRect.top = (float)GetHeight() - ScrollbarWidth(Scrollbar::HORIZONTAL);
                    backgroundRect.bottom = (float)GetHeight();

                    ID2D1SolidColorBrush* brush = nullptr;
                    g.target->CreateSolidColorBrush(_scrollbarBackgroundColor, &brush);
                    g.target->FillRectangle(backgroundRect, brush);
                    brush->Release();
                }

                // Draw scrollbar
                D2D1_COLOR_F color = _scrollbarColor;
                if (MaxScroll(Scrollbar::HORIZONTAL) == 0)
                {
                    color = _scrollbarDisabledColor;
                }
                else
                {
                    if (_horizontalScrollbar.hovered || _horizontalScrollbar.held)
                        color.a = 1.0f;
                    else
                        color.a = 0.5f;
                    if (!_horizontalScrollbar.backgroundVisible)
                        color.a *= _horizontalScrollbar.opacity;
                }
                ID2D1SolidColorBrush* brush = nullptr;
                g.target->CreateSolidColorBrush(color, &brush);
                D2D1_ROUNDED_RECT scrollbarRect{};
                scrollbarRect.radiusX = 2.0f;
                scrollbarRect.radiusY = 2.0f;
                scrollbarRect.rect = hitbox;
                scrollbarRect.rect.left += 2.0f;
                scrollbarRect.rect.top += 2.0f;
                scrollbarRect.rect.right -= 2.0f;
                scrollbarRect.rect.bottom -= 2.0f;
                g.target->FillRoundedRectangle(scrollbarRect, brush);
                brush->Release();
            }
        }

        void _OnResize(int width, int height)
        {
            _RecalculateLayout(width, height);
        }

        void _OnWindowPosChange(int x, int y)
        {
            _SetWindowPositions();
        }

        EventTargets _OnMouseMove(int deltaX, int deltaY)
        {
            // Scroll
            if (_verticalScrollbar.held)
            {
                if (deltaY != 0)
                {
                    auto workArea = ScrollbarWorkArea(Scrollbar::VERTICAL);
                    auto hitbox = ScrollbarHitbox(Scrollbar::VERTICAL);
                    float scrollableLength = (workArea.second - workArea.first) - (hitbox.bottom - hitbox.top);
                    float scrolledLength = GetMousePosY() - workArea.first - _verticalScrollbar.holdPos;
                    float scrollRatio = scrolledLength / scrollableLength;
                    ScrollPosition(Scrollbar::VERTICAL, std::roundf(MaxScroll(Scrollbar::VERTICAL) * scrollRatio));
                }
                return EventTargets().Add(this, GetMousePosX(), GetMousePosY());
            }
            if (_horizontalScrollbar.held)
            {
                if (deltaX != 0)
                {
                    auto workArea = ScrollbarWorkArea(Scrollbar::HORIZONTAL);
                    auto hitbox = ScrollbarHitbox(Scrollbar::HORIZONTAL);
                    float scrollableLength = (workArea.second - workArea.first) - (hitbox.right - hitbox.left);
                    float scrolledLength = GetMousePosX() - workArea.first - _horizontalScrollbar.holdPos;
                    float scrollRatio = scrolledLength / scrollableLength;
                    ScrollPosition(Scrollbar::HORIZONTAL, std::roundf(MaxScroll(Scrollbar::HORIZONTAL) * scrollRatio));
                }
                return EventTargets().Add(this, GetMousePosX(), GetMousePosY());
            }

            std::vector<Component*> hoveredComponents;

            // Adjust coordinates for scroll
            int adjX = GetMousePosX() + _horizontalScrollbar.scrollAmount;
            int adjY = GetMousePosY() + _verticalScrollbar.scrollAmount;

            //Base* handledItem = nullptr;
            bool itemHandled = false;
            EventTargets targets;
            for (auto& _item : _items)
            {
                Component* item = _item.item;

                if (!item->GetVisible()) continue;

                if (adjX >= item->GetX() && adjX < item->GetX() + item->GetWidth() &&
                    adjY >= item->GetY() && adjY < item->GetY() + item->GetHeight())
                {
                    if (item->GetMouseLeftClicked() || item->GetMouseRightClicked())
                    {
                        if (!itemHandled)
                        {
                            targets = item->OnMouseMove(adjX - item->GetX(), adjY - item->GetY());
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
                                targets = item->OnMouseMove(adjX - item->GetX(), adjY - item->GetY());
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
                return std::move(targets.Add(this, GetMousePosX(), GetMousePosY()));

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
                        result = item->OnMouseMove(adjX - item->GetX(), adjY - item->GetY());
                        if (!result.Empty())
                        {
                            eventHandled = true;
                            continue;
                        }
                    }
                    if (item->GetMouseInside())
                        item->OnMouseLeave();
                }
                return std::move(result.Add(this, GetMousePosX(), GetMousePosY()));

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
                return std::move(targets.Add(this, GetMousePosX(), GetMousePosY()));

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

        void _OnMouseLeave()
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

        void _OnMouseEnter()
        {

            //std::cout << "Mouse enter\n";
        }

        void _OnMouseLeaveArea()
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

        void _OnMouseEnterArea()
        {

            //std::cout << "Mouse enter\n";
        }

        EventTargets _OnLeftPressed(int x, int y)
        {
            // Grab scrollbar
            if (ScrollbarHovered(Scrollbar::VERTICAL) && _verticalScrollbar.backgroundVisible && _verticalScrollbar.interactable)
            {
                _verticalScrollbar.held = true;
                _verticalScrollbar.holdPos = y - ScrollbarHitbox(Scrollbar::VERTICAL).top;
                // Stop scroll animation
                _verticalScrollbar.scrollAnimation.inProgress = false;
            }
            else if (ScrollbarHovered(Scrollbar::HORIZONTAL) && _horizontalScrollbar.backgroundVisible && _horizontalScrollbar.interactable)
            {
                _horizontalScrollbar.held = true;
                _horizontalScrollbar.holdPos = x - ScrollbarHitbox(Scrollbar::HORIZONTAL).left;
                // Stop scroll animation
                _horizontalScrollbar.scrollAnimation.inProgress = false;
            }

            // Adjust coordinates for scroll
            int adjX = x + _horizontalScrollbar.scrollAmount;
            int adjY = y + _verticalScrollbar.scrollAmount;

            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->GetVisible())
                    continue;
                if (item->GetMouseInside())
                    return item->OnLeftPressed(adjX - item->GetX(), adjY - item->GetY()).Add(this, x, y);
            }
            return EventTargets().Add(this, x, y);

            // CODE BELOW CONTAINS A SOLUTION FOR A FIXED* PROBLEM
            // * might still have edge cases, hence why I'm not removing the code
            // * the fix at the moment is calling 'OnMouseMove' from 'OnLeft/RightClick'
            // * if the click mouse pos doesn't match stored mouse position

            // Find top hovered item
            //
            // The code here somewhat duplicates the code in 'OnMouseMove'.
            // While I could just check for which item 'GetMouseInside' returns true
            // (and did so for a while), such a solution runs into problems in specific
            // edge cases. In particular, when passing mouse click messages from overlay
            // scene with an open menu to other scenes. While the menu is open, an
            // occlusion panel is also open, which blocks mouse move events from
            // going through to other scenes, thus 'GetMouseInside' will always return
            // false. Closing the menu on click and passing the mouse click message to
            // other scenes will therefore run into problems because no mouse move
            // events were sent to make any items clickable (aka. mouseInside = true).
            //
            // All this means that mouse clicks (both right and left) need to do their
            // own hit test calculations when 'GetMouseInside' returns false for all
            // items. If any item returns true, the click must be passed to it, because
            // of special cases of drag-clicking outside of the visual item area.

            // Get all hovered items
            std::vector<Component*> hoveredComponents;
            for (auto& _item : _items)
            {
                Component* item = _item.item;

                if (!item->GetVisible())
                    continue;

                if (item->GetMouseInside())
                    return item->OnLeftPressed(adjX - item->GetX(), adjY - item->GetY()).Add(this, x, y);

                if (adjX >= item->GetX() && adjX < item->GetX() + item->GetWidth() &&
                    adjY >= item->GetY() && adjY < item->GetY() + item->GetHeight())
                    hoveredComponents.push_back(item);
            }
            if (!hoveredComponents.empty())
            {
                // Find item with highest z-index
                Component* topmost = hoveredComponents[0];
                for (int i = 1; i < hoveredComponents.size(); i++)
                    if (hoveredComponents[i]->GetZIndex() > topmost->GetZIndex())
                        topmost = hoveredComponents[i];

                if (!topmost->GetMouseInside())
                    topmost->OnMouseEnter();
                return topmost->OnLeftPressed(adjX - topmost->GetX(), adjY - topmost->GetY()).Add(this, GetMousePosX(), GetMousePosY());
            }
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnLeftReleased(int x, int y)
        {
            // Release scrollbars
            if (_verticalScrollbar.held)
            {
                _verticalScrollbar.held = false;
                InvokeRedraw();
            }
            if (_horizontalScrollbar.held)
            {
                _horizontalScrollbar.held = false;
                InvokeRedraw();
            }

            // Adjust coordinates for scroll
            int adjX = x + _horizontalScrollbar.scrollAmount;
            int adjY = y + _verticalScrollbar.scrollAmount;

            EventTargets targets;
            for (auto& item : _items)
            {
                if (!item.item->GetVisible()) continue;

                if (item.item->GetMouseInside())
                {
                    targets = item.item->OnLeftReleased(adjX - item.item->GetX(), adjY - item.item->GetY()).Add(this, x, y);
                    break;
                }
            }
            if (targets.Empty())
                targets.Add(this, x, y);
            return targets;
        }

        EventTargets _OnRightPressed(int x, int y)
        {
            // Adjust coordinates for scroll
            int adjX = x + _horizontalScrollbar.scrollAmount;
            int adjY = y + _verticalScrollbar.scrollAmount;

            for (auto& _item : _items)
            {
                Component* item = _item.item;
                if (!item->GetVisible())
                    continue;
                if (item->GetMouseInside())
                    return item->OnRightPressed(adjX - item->GetX(), adjY - item->GetY()).Add(this, x, y);
            }
            return EventTargets().Add(this, x, y);

            // CODE BELOW CONTAINS A SOLUTION FOR A FIXED* PROBLEM
            // * might still have edge cases, hence why I'm not removing the code
            // * the fix at the moment is calling 'OnMouseMove' from 'OnLeft/RightClick'
            // * if the click mouse pos doesn't match stored mouse position

            // Find top hovered item
            // ** In depth explanation in '_OnLeftPressed' **

            // Get all hovered items
            std::vector<Component*> hoveredComponents;
            for (auto& _item : _items)
            {
                Component* item = _item.item;

                if (!item->GetVisible())
                    continue;

                if (item->GetMouseInside())
                    return item->OnRightPressed(adjX - item->GetX(), adjY - item->GetY()).Add(this, x, y);

                if (adjX >= item->GetX() && adjX < item->GetX() + item->GetWidth() &&
                    adjY >= item->GetY() && adjY < item->GetY() + item->GetHeight())
                    hoveredComponents.push_back(item);
            }
            if (!hoveredComponents.empty())
            {
                // Find item with highest z-index
                Component* topmost = hoveredComponents[0];
                for (int i = 1; i < hoveredComponents.size(); i++)
                    if (hoveredComponents[i]->GetZIndex() > topmost->GetZIndex())
                        topmost = hoveredComponents[i];

                if (!topmost->GetMouseInside())
                    topmost->OnMouseEnter();
                return topmost->OnRightPressed(adjX - topmost->GetX(), adjY - topmost->GetY()).Add(this, GetMousePosX(), GetMousePosY());
            }
            return EventTargets().Add(this, x, y);
        }

        EventTargets _OnRightReleased(int x, int y)
        {
            // Adjust coordinates for scroll
            int adjX = x + _horizontalScrollbar.scrollAmount;
            int adjY = y + _verticalScrollbar.scrollAmount;

            EventTargets targets;
            for (auto& item : _items)
            {
                if (!item.item->GetVisible()) continue;

                if (item.item->GetMouseInside())
                {
                    targets = item.item->OnRightReleased(adjX - item.item->GetX(), adjY - item.item->GetY()).Add(this, x, y);
                    break;
                }
            }
            if (targets.Empty())
                targets.Add(this, x, y);
            return targets;
        }

        EventTargets _OnWheelUp(int x, int y)
        {
            // Adjust coordinates for scroll
            int adjX = x + _horizontalScrollbar.scrollAmount;
            int adjY = y + _verticalScrollbar.scrollAmount;

            EventTargets targets;
            for (auto& item : _items)
            {
                if (!item.item->GetVisible()) continue;

                if (item.item->GetMouseInside())
                {
                    targets = item.item->OnWheelUp(adjX - item.item->GetX(), adjY - item.item->GetY());
                    break;
                }
            }

            if (targets.Empty())
            {
                if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                {
                    if (_horizontalScrollbar.scrollable)
                    {
                        Scrollbar dir = Scrollbar::HORIZONTAL;
                        Scroll(dir, ScrollPosition(dir) - ScrollStepSize(dir));
                        return EventTargets().Add(this, x, y);
                    }
                }
                else
                {
                    if (_verticalScrollbar.scrollable)
                    {
                        Scrollbar dir = Scrollbar::VERTICAL;
                        Scroll(dir, ScrollPosition(dir) - ScrollStepSize(dir));
                        return EventTargets().Add(this, x, y);
                    }
                }
            }
            return EventTargets();
        }

        EventTargets _OnWheelDown(int x, int y)
        {
            // Adjust coordinates for scroll
            int adjX = x + _horizontalScrollbar.scrollAmount;
            int adjY = y + _verticalScrollbar.scrollAmount;

            EventTargets targets;
            for (auto& item : _items)
            {
                if (!item.item->GetVisible()) continue;

                if (item.item->GetMouseInside())
                {
                    targets = item.item->OnWheelDown(adjX - item.item->GetX(), adjY - item.item->GetY());
                    break;
                }
            }

            if (targets.Empty())
            {
                if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                {
                    if (_horizontalScrollbar.scrollable)
                    {
                        Scrollbar dir = Scrollbar::HORIZONTAL;
                        Scroll(dir, ScrollPosition(dir) + ScrollStepSize(dir));
                        return EventTargets().Add(this, x, y);
                    }
                }
                else
                {
                    if (_verticalScrollbar.scrollable)
                    {
                        Scrollbar dir = Scrollbar::VERTICAL;
                        Scroll(dir, ScrollPosition(dir) + ScrollStepSize(dir));
                        return EventTargets().Add(this, x, y);
                    }
                }
            }
            return EventTargets();
        }

        void _OnSelected(bool reverse)
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
        std::list<Component*> GetChildren()
        {
            std::list<Component*> children;
            for (auto& item : _items)
            {
                children.push_back(item.item);
            }
            return children;
        }

        std::list<Component*> GetAllChildren()
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

        Component* IterateTab(bool reverse)
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

        const char* GetName() const { return Name(); }
        static const char* Name() { return "panel"; }
#pragma endregion

    protected:
        virtual void _RecalculateLayout(int width, int height)
        {
            // When scrollbars are visible, whey might intrude into draw area
            int widthWithPadding = GetWidth() - (_verticalScrollbar.backgroundVisible ? 10 : 0);
            int heightWithPadding = GetHeight() - (_horizontalScrollbar.backgroundVisible ? 10 : 0);

            // Calculate item sizes and positions
            int maxRightEdge = 0;
            int maxBottomEdge = 0;
            for (auto& _item : _items)
            {
                Component* item = _item.item;

                int newWidth = (int)std::round(widthWithPadding * item->GetParentWidthPercent()) + item->GetBaseWidth();
                int newHeight = (int)std::round(heightWithPadding * item->GetParentHeightPercent()) + item->GetBaseHeight();
                if (newWidth < 1)
                    newWidth = 1;
                if (newHeight < 1)
                    newHeight = 1;

                int newPosX = 0;
                if (item->GetHorizontalAlignment() == Alignment::START)
                {
                    newPosX += std::round((widthWithPadding - newWidth) * item->GetHorizontalOffsetPercent());
                }
                else if (item->GetHorizontalAlignment() == Alignment::CENTER)
                {
                    newPosX = (widthWithPadding - newWidth) / 2;
                }
                else if (item->GetHorizontalAlignment() == Alignment::END)
                {
                    newPosX = widthWithPadding - newWidth;
                    newPosX -= std::round((widthWithPadding - newWidth) * item->GetHorizontalOffsetPercent());
                }
                newPosX += item->GetHorizontalOffsetPixels();
                newPosX += _margins.left;
                // Alternative (no branching):
                // int align = item->GetHorizontalAlignment() == Alignment::END;
                // newPosX += align * (_width - item->GetWidth());
                // newPosX += (-1 * align) * std::round((_width - item->GetWidth()) * item->GetHorizontalOffsetPercent());
                // newPosX += item->GetHorizontalOffsetPixels();
                int newPosY = 0;
                if (item->GetVerticalAlignment() == Alignment::START)
                {
                    newPosY += std::round((heightWithPadding - newHeight) * item->GetVerticalOffsetPercent());
                }
                else if (item->GetVerticalAlignment() == Alignment::CENTER)
                {
                    newPosY = (heightWithPadding - newHeight) / 2;
                }
                else if (item->GetVerticalAlignment() == Alignment::END)
                {
                    newPosY = heightWithPadding - newHeight;
                    newPosY -= std::round((heightWithPadding - newHeight) * item->GetVerticalOffsetPercent());
                }
                newPosY += item->GetVerticalOffsetPixels();
                newPosY += _margins.top;

                item->SetPosition(newPosX, newPosY);
                item->Resize(newWidth, newHeight);

                if (newPosX + newWidth > maxRightEdge)
                    maxRightEdge = newPosX + newWidth;
                if (newPosY + newHeight > maxBottomEdge)
                    maxBottomEdge = newPosY + newHeight;
            }
            _contentWidth = maxRightEdge + _margins.right;
            _contentHeight = maxBottomEdge + _margins.bottom;

            if (_verticalScrollbar.scrollAmount > MaxScroll(Scrollbar::VERTICAL))
                _verticalScrollbar.scrollAmount = MaxScroll(Scrollbar::VERTICAL);
            if (_horizontalScrollbar.scrollAmount > MaxScroll(Scrollbar::HORIZONTAL))
                _horizontalScrollbar.scrollAmount = MaxScroll(Scrollbar::HORIZONTAL);

            _SetWindowPositions();
            InvokeRedraw();
        }

        void _SetWindowPositions()
        {
            for (auto& _item : _items)
            {
                Component* item = _item.item;
                item->SetWindowPosition(
                    GetWindowX() + item->GetX() - _horizontalScrollbar.scrollAmount,
                    GetWindowY() + item->GetY() - _verticalScrollbar.scrollAmount
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

        // Margins
        RECT _margins = { 0, 0, 0, 0 };

        // Auto child resize
        bool _deferUpdates = false;
        bool _updatesDeferred = false;

        bool _fallthroughMouseEvents = false;

        // Scrolling
        struct _ScrollAnimation
        {
            bool inProgress = false;
            TimePoint startTime = 0;
            Duration duration = 0;
            int startPos = 0;
            int endPos = 0;
            std::function<float(float)> progressFunction;
        };
        struct _Scrollbar
        {
            bool scrollable = false;
            int scrollAmount = 0;
            int stepSize = 100;
            int width = 10;
            bool alwaysVisible = false;
            bool backgroundVisible = false;
            bool visibleOnScroll = true;
            bool interactable = true;
            bool hovered = false;
            bool held = false;
            int holdPos = 0;
            bool visible = false;
            TimePoint showTime = 0;
            Duration hangDuration = Duration(1, SECONDS);
            Duration fadeDuration = Duration(150, MILLISECONDS);
            float opacity = 1.0f;
            _ScrollAnimation scrollAnimation = {};
        };
        int _contentWidth = 0;
        int _contentHeight = 0;
        _Scrollbar _horizontalScrollbar;
        _Scrollbar _verticalScrollbar;
        D2D1_COLOR_F _scrollbarBackgroundColor = D2D1::ColorF(0.07f, 0.07f, 0.07f);
        D2D1_COLOR_F _scrollbarDisabledColor = D2D1::ColorF(0.2f, 0.2f, 0.2f);
        D2D1_COLOR_F _scrollbarColor = D2D1::ColorF(0.7f, 0.7f, 0.7f);

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
        void _AddItem(Component* item, bool transferOwnership)
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
                _RecalculateLayout(GetWidth(), GetHeight());
                if (GetMouseInside())
                    OnMouseMove(GetMousePosX(), GetMousePosY());
            });

            // Add selection handler for scrolling
            _items.back().selectHandler = item->SubscribeOnSelected([&](zcom::Component* srcItem, bool reverse)
            {
                // Propagate up, to allow scrolling to nested items
                _onSelected->InvokeAll(this, reverse);

                ScrollToItem(srcItem);
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
                _RecalculateLayout(GetWidth(), GetHeight());
                if (GetMouseInside())
                    OnMouseMove(GetMousePosX(), GetMousePosY());
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

        RECT GetMargins() const
        {
            return _margins;
        }

        void SetMargins(RECT margins)
        {
            if (_margins != margins)
            {
                _margins = margins;
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

        // Scrolling

    private:
        const _Scrollbar& _GetConstScrollbar(Scrollbar direction) const
        {
            if (direction == Scrollbar::VERTICAL)
                return _verticalScrollbar;
            else
                return _horizontalScrollbar;
        }

        _Scrollbar& _GetScrollbar(Scrollbar direction)
        {
            if (direction == Scrollbar::VERTICAL)
                return _verticalScrollbar;
            else
                return _horizontalScrollbar;
        }
    public:

        bool Scrollable(Scrollbar direction) const
        {
            return _Scrollable(_GetConstScrollbar(direction));
        }

        void Scrollable(Scrollbar direction, bool scrollable)
        {
            _Scrollable(_GetScrollbar(direction), scrollable);
        }

        int MaxScroll(Scrollbar direction) const
        {
            if (direction == Scrollbar::VERTICAL)
            {
                int maxScroll = _contentHeight - GetHeight() + (_horizontalScrollbar.backgroundVisible ? 10 : 0);
                if (maxScroll < 0)
                    maxScroll = 0;
                return maxScroll;
            }
            else
            {
                int maxScroll = _contentWidth - GetWidth() + (_verticalScrollbar.backgroundVisible ? 10 : 0);
                if (maxScroll < 0)
                    maxScroll = 0;
                return maxScroll;
            }
        }

        int ScrollPosition(Scrollbar direction) const
        {
            return _ScrollPosition(_GetConstScrollbar(direction));
        }

        int VisualScrollPosition(Scrollbar direction) const
        {
            return _VisualScrollPosition(_GetConstScrollbar(direction));
        }

        void ScrollPosition(Scrollbar direction, int position)
        {
            if (direction == Scrollbar::VERTICAL)
            {
                if (!_verticalScrollbar.scrollable)
                    return;

                _verticalScrollbar.scrollAmount = position;
                int maxScroll = MaxScroll(direction);
                if (_verticalScrollbar.scrollAmount > maxScroll)
                    _verticalScrollbar.scrollAmount = maxScroll;
                else if (_verticalScrollbar.scrollAmount < 0)
                    _verticalScrollbar.scrollAmount = 0;
                _SetWindowPositions();
                InvokeRedraw();
            }
            else
            {
                if (!_horizontalScrollbar.scrollable)
                    return;

                _horizontalScrollbar.scrollAmount = position;
                int maxScroll = MaxScroll(direction);
                if (_horizontalScrollbar.scrollAmount > maxScroll)
                    _horizontalScrollbar.scrollAmount = maxScroll;
                else if (_horizontalScrollbar.scrollAmount < 0)
                    _horizontalScrollbar.scrollAmount = 0;
                _SetWindowPositions();
                InvokeRedraw();
            }
        }

        Duration ScrollbarHangDuration(Scrollbar direction) const
        {
            return _ScrollbarHangDuration(_GetConstScrollbar(direction));
        }

        void ScrollbarHangDuration(Scrollbar direction, Duration duration)
        {
            _ScrollbarHangDuration(_GetScrollbar(direction), duration);
        }

        bool ScrollBackgroundVisible(Scrollbar direction) const
        {
            return _ScrollBackgroundVisible(_GetConstScrollbar(direction));
        }

        void ScrollBackgroundVisible(Scrollbar direction, bool visible)
        {
            _ScrollBackgroundVisible(_GetScrollbar(direction), visible);
        }

        D2D1_COLOR_F ScrollBackgroundColor() const
        {
            return _scrollbarBackgroundColor;
        }

        void ScrollBackgroundColor(D2D1_COLOR_F color)
        {
            if (_scrollbarBackgroundColor == color)
                return;
            _scrollbarBackgroundColor = color;
            InvokeRedraw();
        }

        int ScrollStepSize(Scrollbar direction) const
        {
            return _ScrollStepSize(_GetConstScrollbar(direction));
        }

        void ScrollStepSize(Scrollbar direction, int stepSize)
        {
            _ScrollStepSize(_GetScrollbar(direction), stepSize);
        }

        int ScrollbarWidth(Scrollbar direction) const
        {
            return _ScrollbarWidth(_GetConstScrollbar(direction));
        }

        void ScrollbarWidth(Scrollbar direction, int width)
        {
            _ScrollbarWidth(_GetScrollbar(direction), width);
        }

        std::pair<float, float> ScrollbarWorkArea(Scrollbar direction) const
        {
            if (direction == Scrollbar::VERTICAL)
            {
                float topOffset = 10.0f;
                float bottomOffset = ScrollbarWidth(Scrollbar::HORIZONTAL);
                if (_verticalScrollbar.backgroundVisible)
                {
                    topOffset = 0.0f;
                    if (!_horizontalScrollbar.backgroundVisible)
                        bottomOffset = 0.0f;
                }
                return { topOffset, GetHeight() - bottomOffset };
            }
            else
            {
                float leftOffset = 10.0f;
                float rightOffset = ScrollbarWidth(Scrollbar::VERTICAL);
                if (_horizontalScrollbar.backgroundVisible)
                {
                    leftOffset = 0.0f;
                    if (!_verticalScrollbar.backgroundVisible)
                        rightOffset = 0.0f;
                }
                return { leftOffset, GetWidth() - rightOffset };
            }
        }

        D2D1_RECT_F ScrollbarHitbox(Scrollbar direction) const
        {
            if (direction == Scrollbar::VERTICAL)
            {
                auto workArea = ScrollbarWorkArea(direction);
                float startPos = workArea.first;
                float endPos = workArea.second;
                float hitboxWidth = ScrollbarWidth(Scrollbar::VERTICAL);

                // Calculate scrollbar layout
                float heightToContentRatio = GetHeight() / (float)_contentHeight;
                if (heightToContentRatio > 1.0f)
                    heightToContentRatio = 1.0f;
                float scrollBarMaxLength = endPos - startPos;
                float scrollBarLength = scrollBarMaxLength * heightToContentRatio;
                float scrollBarPosition = 0.0f;
                if (MaxScroll(Scrollbar::VERTICAL) != 0)
                    scrollBarPosition = (scrollBarMaxLength - scrollBarLength) * (_verticalScrollbar.scrollAmount / (float)MaxScroll(Scrollbar::VERTICAL));

                D2D1_RECT_F hitbox{};
                hitbox.left = GetWidth() - hitboxWidth;
                hitbox.right = GetWidth();
                hitbox.top = startPos + scrollBarPosition;
                hitbox.bottom = hitbox.top + scrollBarLength;
                return hitbox;
            }
            else
            {
                auto workArea = ScrollbarWorkArea(direction);
                float startPos = workArea.first;
                float endPos = workArea.second;
                float hitboxWidth = ScrollbarWidth(Scrollbar::HORIZONTAL);

                // Calculate scrollbar layout
                float widthToContentRatio = GetWidth() / (float)_contentWidth;
                if (widthToContentRatio > 1.0f)
                    widthToContentRatio = 1.0f;
                float scrollBarMaxLength = endPos - startPos;
                float scrollBarLength = scrollBarMaxLength * widthToContentRatio;
                float scrollBarPosition = 0.0f;
                if (MaxScroll(Scrollbar::HORIZONTAL) != 0)
                    scrollBarPosition = (scrollBarMaxLength - scrollBarLength) * (_horizontalScrollbar.scrollAmount / (float)MaxScroll(Scrollbar::HORIZONTAL));

                D2D1_RECT_F hitbox{};
                hitbox.left = startPos + scrollBarPosition;
                hitbox.right = hitbox.left + scrollBarLength;
                hitbox.top = GetHeight() - hitboxWidth;
                hitbox.bottom = GetHeight();
                return hitbox;
            }
        }

        bool ScrollbarHovered(Scrollbar direction) const
        {
            auto hitbox = ScrollbarHitbox(direction);
            return GetMouseInside() &&
                GetMousePosX() >= hitbox.left &&
                GetMousePosX() < hitbox.right &&
                GetMousePosY() >= hitbox.top &&
                GetMousePosY() < hitbox.bottom;
        }

        void Scroll(
            Scrollbar direction,
            int to,
            Duration scrollDuration = Duration(150, MILLISECONDS),
            std::function<float(float)> progressFunction = std::function<float(float)>())
        {
            if (direction == Scrollbar::VERTICAL)
            {
                if (to < 0)
                    to = 0;
                else if (to > MaxScroll(direction))
                    to = MaxScroll(direction);

                _verticalScrollbar.scrollAnimation.inProgress = true;
                _verticalScrollbar.scrollAnimation.startTime = ztime::Main();
                _verticalScrollbar.scrollAnimation.duration = scrollDuration;
                _verticalScrollbar.scrollAnimation.startPos = _verticalScrollbar.scrollAmount;
                _verticalScrollbar.scrollAnimation.endPos = to;
                _verticalScrollbar.scrollAnimation.progressFunction = progressFunction;

                if (_verticalScrollbar.visibleOnScroll)
                {
                    _verticalScrollbar.visible = true;
                    _verticalScrollbar.showTime = ztime::Main();
                }
            }
            else
            {
                if (to < 0)
                    to = 0;
                else if (to > MaxScroll(direction))
                    to = MaxScroll(direction);

                _horizontalScrollbar.scrollAnimation.inProgress = true;
                _horizontalScrollbar.scrollAnimation.startTime = ztime::Main();
                _horizontalScrollbar.scrollAnimation.duration = scrollDuration;
                _horizontalScrollbar.scrollAnimation.startPos = _horizontalScrollbar.scrollAmount;
                _horizontalScrollbar.scrollAnimation.endPos = to;
                _horizontalScrollbar.scrollAnimation.progressFunction = progressFunction;

                if (_horizontalScrollbar.visibleOnScroll)
                {
                    _horizontalScrollbar.visible = true;
                    _horizontalScrollbar.showTime = ztime::Main();
                }
            }
        }

        // Scrolls to the specified item.
        // If 'force' == true, scrolling is done even if item is already fully visible (currently unimplemented)
        void ScrollToItem(zcom::Component* item, bool force = false)
        {
            // TODO: add 'force' == true functionality

            // Vertically
            if (_verticalScrollbar.scrollable)
            {
                if (item->GetY() < _verticalScrollbar.scrollAmount)
                    Scroll(zcom::Scrollbar::VERTICAL, item->GetY());
                else if (item->GetY() + item->GetHeight() > _verticalScrollbar.scrollAmount + GetHeight())
                    Scroll(zcom::Scrollbar::VERTICAL, item->GetY() + item->GetHeight() - GetHeight());
            }
            // Horizontally
            if (_horizontalScrollbar.scrollable)
            {
                if (item->GetX() < _horizontalScrollbar.scrollAmount)
                    Scroll(zcom::Scrollbar::HORIZONTAL, item->GetX());
                else if (item->GetX() + item->GetWidth() > _horizontalScrollbar.scrollAmount + GetWidth())
                    Scroll(zcom::Scrollbar::HORIZONTAL, item->GetX() + item->GetWidth() - GetWidth());
            }
        }

    private:
        bool _Scrollable(const _Scrollbar& scrollbar) const
        {
            return scrollbar.scrollable;
        }

        void _Scrollable(_Scrollbar& scrollbar, bool scrollable)
        {
            scrollbar.scrollable = scrollable;
        }

        int _ScrollPosition(const _Scrollbar& scrollbar) const
        {
            if (scrollbar.scrollAnimation.inProgress)
                return scrollbar.scrollAnimation.endPos;
            else
                return scrollbar.scrollAmount;
        }

        int _VisualScrollPosition(const _Scrollbar& scrollbar) const
        {
            return scrollbar.scrollAmount;
        }

        Duration _ScrollbarHangDuration(const _Scrollbar& scrollbar) const
        {
            return scrollbar.hangDuration;
        }

        void _ScrollbarHangDuration(_Scrollbar& scrollbar, Duration duration)
        {
            scrollbar.hangDuration = duration;
        }

        bool _ScrollBackgroundVisible(const _Scrollbar& scrollbar) const
        {
            return scrollbar.backgroundVisible;
        }

        void _ScrollBackgroundVisible(_Scrollbar& scrollbar, bool visible)
        {
            if (scrollbar.backgroundVisible == visible)
                return;
            scrollbar.backgroundVisible = visible;
            _RecalculateLayout(GetWidth(), GetHeight());
            InvokeRedraw();
        }

        int _ScrollStepSize(const _Scrollbar& scrollbar) const
        {
            return scrollbar.stepSize;
        }

        void _ScrollStepSize(_Scrollbar& scrollbar, int stepSize)
        {
            scrollbar.stepSize = stepSize;
        }

        int _ScrollbarWidth(const _Scrollbar& scrollbar) const
        {
            return scrollbar.width;
        }

        void _ScrollbarWidth(_Scrollbar& scrollbar, int width)
        {
            if (scrollbar.width == width)
                return;
            scrollbar.width = width;
            _RecalculateLayout(GetWidth(), GetHeight());
            InvokeRedraw();
        }
    };
}