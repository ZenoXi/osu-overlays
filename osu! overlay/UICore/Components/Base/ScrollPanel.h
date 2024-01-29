#pragma once

#include "Panel.h"

#include "Helper/Time.h"

namespace zcom
{
    enum class Scrollbar
    {
        VERTICAL,
        HORIZONTAL
    };

    class ScrollPanel : public Panel
    {
    public:
        bool Scrollable(Scrollbar direction) const;
        void Scrollable(Scrollbar direction, bool scrollable);
        int MaxScroll(Scrollbar direction) const;
        int ScrollPosition(Scrollbar direction) const;
        int VisualScrollPosition(Scrollbar direction) const;
        void ScrollPosition(Scrollbar direction, int position);
        Duration ScrollbarHangDuration(Scrollbar direction) const;
        void ScrollbarHangDuration(Scrollbar direction, Duration duration);
        bool ScrollBackgroundVisible(Scrollbar direction) const;
        void ScrollBackgroundVisible(Scrollbar direction, bool visible);
        D2D1_COLOR_F ScrollBackgroundColor() const;
        void ScrollBackgroundColor(D2D1_COLOR_F color);
        int ScrollStepSize(Scrollbar direction) const;
        void ScrollStepSize(Scrollbar direction, int stepSize);
        int ScrollbarWidth(Scrollbar direction) const;
        void ScrollbarWidth(Scrollbar direction, int width);
        std::pair<float, float> ScrollbarWorkArea(Scrollbar direction) const;
        D2D1_RECT_F ScrollbarHitbox(Scrollbar direction) const;
        bool ScrollbarHovered(Scrollbar direction) const;
        void Scroll(
            Scrollbar direction,
            int to,
            Duration scrollDuration = Duration(150, MILLISECONDS),
            std::function<float(float)> progressFunction = std::function<float(float)>());
        // Scrolls to the specified item.
        // If 'force' == true, scrolling is done even if item is already fully visible (currently unimplemented)
        void ScrollToItem(Component* item, bool force = false);

    private:
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
        _Scrollbar _horizontalScrollbar;
        _Scrollbar _verticalScrollbar;
        D2D1_COLOR_F _scrollbarBackgroundColor = D2D1::ColorF(0.07f, 0.07f, 0.07f);
        D2D1_COLOR_F _scrollbarDisabledColor = D2D1::ColorF(0.2f, 0.2f, 0.2f);
        D2D1_COLOR_F _scrollbarColor = D2D1::ColorF(0.7f, 0.7f, 0.7f);
        RECT _basePadding = {};

        void _UpdateScrollbar(Scrollbar direction);
        const _Scrollbar& _GetConstScrollbar(Scrollbar direction) const;
        _Scrollbar& _GetScrollbar(Scrollbar direction);
        bool _Scrollable(const _Scrollbar& scrollbar) const;
        void _Scrollable(_Scrollbar& scrollbar, bool scrollable);
        int _ScrollPosition(const _Scrollbar& scrollbar) const;
        int _VisualScrollPosition(const _Scrollbar& scrollbar) const;
        Duration _ScrollbarHangDuration(const _Scrollbar& scrollbar) const;
        void _ScrollbarHangDuration(_Scrollbar& scrollbar, Duration duration);
        bool _ScrollBackgroundVisible(const _Scrollbar& scrollbar) const;
        void _ScrollBackgroundVisible(_Scrollbar& scrollbar, bool visible);
        int _ScrollStepSize(const _Scrollbar& scrollbar) const;
        void _ScrollStepSize(_Scrollbar& scrollbar, int stepSize);
        int _ScrollbarWidth(const _Scrollbar& scrollbar) const;
        void _ScrollbarWidth(_Scrollbar& scrollbar, int width);

    protected:
        friend class Scene;
        friend class Component;
        ScrollPanel(Scene* scene) : Panel(scene) {}
        void Init()
        {
            Panel::Init();
        }
    public:
        ~ScrollPanel() {}
        ScrollPanel(ScrollPanel&&) = delete;
        ScrollPanel& operator=(ScrollPanel&&) = delete;
        ScrollPanel(const ScrollPanel&) = delete;
        ScrollPanel& operator=(const ScrollPanel&) = delete;

#pragma region base_class
    protected:
        void _OnUpdate() override
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

            Panel::_OnUpdate();
        }

        void _OnDraw(Graphics g) override
        {
            D2D1_MATRIX_3X2_F originalTransform;
            g.target->GetTransform(&originalTransform);
            g.target->SetTransform(originalTransform * D2D1::Matrix3x2F::Translation((float) -_horizontalScrollbar.scrollAmount, (float) -_verticalScrollbar.scrollAmount));
            Panel::_OnDraw(g);
            g.target->SetTransform(originalTransform);

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

        EventTargets _OnMouseMove(int x, int y, int deltaX, int deltaY) override
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

            auto targets = Panel::_OnMouseMove(x + _horizontalScrollbar.scrollAmount, y + _verticalScrollbar.scrollAmount, deltaX, deltaY);
            targets.RemoveLast();
            // Overwrite parent return, since it contains scroll adjusted coordinates
            return std::move(targets.Add(this, x, y));
        }

        EventTargets _OnLeftPressed(int x, int y) override
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

            auto targets = Panel::_OnLeftPressed(x + _horizontalScrollbar.scrollAmount, y + _verticalScrollbar.scrollAmount);
            targets.RemoveLast();
            // Overwrite parent return, since it contains scroll adjusted coordinates
            return std::move(targets.Add(this, x, y));
        }

        EventTargets _OnLeftReleased(int x, int y) override
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

            auto targets = Panel::_OnLeftReleased(x + _horizontalScrollbar.scrollAmount, y + _verticalScrollbar.scrollAmount);
            targets.RemoveLast();
            return std::move(targets.Add(this, x, y));
        }

        EventTargets _OnRightPressed(int x, int y) override
        {
            auto targets = Panel::_OnRightPressed(x + _horizontalScrollbar.scrollAmount, y + _verticalScrollbar.scrollAmount);
            targets.RemoveLast();
            return std::move(targets.Add(this, x, y));
        }

        EventTargets _OnRightReleased(int x, int y) override
        {
            auto targets = Panel::_OnRightReleased(x + _horizontalScrollbar.scrollAmount, y + _verticalScrollbar.scrollAmount);
            targets.RemoveLast();
            return std::move(targets.Add(this, x, y));
        }

        EventTargets _OnWheelUp(int x, int y) override
        {
            auto targets = Panel::_OnWheelUp(x + _horizontalScrollbar.scrollAmount, y + _verticalScrollbar.scrollAmount);
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
            return targets;
        }

        EventTargets _OnWheelDown(int x, int y) override
        {
            auto targets = Panel::_OnWheelDown(x + _horizontalScrollbar.scrollAmount, y + _verticalScrollbar.scrollAmount);
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
            return targets;
        }

        void _RecalculateLayout(int width, int height) override
        {
            Panel::_RecalculateLayout(width, height);

            if (_verticalScrollbar.scrollAmount > MaxScroll(Scrollbar::VERTICAL))
                _verticalScrollbar.scrollAmount = MaxScroll(Scrollbar::VERTICAL);
            if (_horizontalScrollbar.scrollAmount > MaxScroll(Scrollbar::HORIZONTAL))
                _horizontalScrollbar.scrollAmount = MaxScroll(Scrollbar::HORIZONTAL);
        }

        void _SetWindowPositions() override
        {
            Panel::_SetWindowPositions();

            for (auto& _item : _items)
            {
                Component* item = _item.item;
                item->SetWindowPosition(
                    GetWindowX() - _horizontalScrollbar.scrollAmount,
                    GetWindowY() - _verticalScrollbar.scrollAmount
                );
            }
        }

    public:
        void SetPadding(RECT padding)
        {
            _basePadding = padding;
            padding.right += _verticalScrollbar.backgroundVisible ? _verticalScrollbar.width : 0;
            padding.bottom += _horizontalScrollbar.backgroundVisible ? _horizontalScrollbar.width : 0;
            Panel::SetPadding(padding);
        }

    public:
        const char* GetName() const override { return Name(); }
        static const char* Name() { return "scroll_panel"; }
#pragma endregion

    protected:
        void _AddItem(Component* item, bool transferOwnership) override
        {
            Panel::_AddItem(item, transferOwnership);

            // Intercept component selection event
            _items.back().selectHandler = item->SubscribeOnSelected([&](zcom::Component* srcItem, bool reverse)
            {
                // Do not touch standard bubbling behaviour
                _onSelected->InvokeAll(this, reverse);

                ScrollToItem(srcItem);
            });
        }
    };
}