#pragma once

#include "Panel.h"

#include "Helper/Time.h"

namespace zcom
{
    enum class ScrollbarDirection
    {
        VERTICAL,
        HORIZONTAL
    };

    class ScrollPanel : public Panel
    {
        DEFINE_COMPONENT(ScrollPanel, Panel)
    protected:
        void Init()
        {
            Panel::Init();

            xScrollbar.scrollAmount_.ComputedFrom([](bool scrolling, int endPos, int visualScrollAmount) {
                return scrolling ? endPos : visualScrollAmount;
            }, xScrollbar.scrolling_, xScrollbar.scrollEndPos_, xScrollbar.visualScrollAmount);
            xScrollbar.visualScrollAmount.SetSetter([=](int& currentValue, const int& scrollAmount) {
                if (!xScrollbar.scrollable)
                    return;
                currentValue = scrollAmount;
                int maxScroll = MaxScroll(xScrollbar);
                if (currentValue > maxScroll)
                    currentValue = maxScroll;
                else if (currentValue < 0)
                    currentValue = 0;
                _SetWindowPositions();
                InvokeRedraw();
            });
            xScrollbar.color.SetSetter([=](Color& currentValue, const Color& color) {
                currentValue = color;
                InvokeRedraw();
            });
            xScrollbar.backgroundColor.SetSetter([=](Color& currentValue, const Color& color) {
                currentValue = color;
                InvokeRedraw();
            });
            xScrollbar.disabledColor.SetSetter([=](Color& currentValue, const Color& color) {
                currentValue = color;
                InvokeRedraw();
            });
            xScrollbar.hovered_.SetSetter([=](bool& currentValue, const bool& hovered) {
                currentValue = hovered;
                InvokeRedraw();
            });

            yScrollbar.scrollAmount_.ComputedFrom([](bool scrolling, int endPos, int visualScrollAmount) {
                return scrolling ? endPos : visualScrollAmount;
            }, yScrollbar.scrolling_, yScrollbar.scrollEndPos_, yScrollbar.visualScrollAmount);
            yScrollbar.visualScrollAmount.SetSetter([=](int& currentValue, const int& scrollAmount) {
                if (!yScrollbar.scrollable)
                    return;
                currentValue = scrollAmount;
                int maxScroll = MaxScroll(yScrollbar);
                if (currentValue > maxScroll)
                    currentValue = maxScroll;
                else if (currentValue < 0)
                    currentValue = 0;
                _SetWindowPositions();
                InvokeRedraw();
            });
            yScrollbar.color.SetSetter([=](Color& currentValue, const Color& color) {
                currentValue = color;
                InvokeRedraw();
            });
            yScrollbar.backgroundColor.SetSetter([=](Color& currentValue, const Color& color) {
                currentValue = color;
                InvokeRedraw();
            });
            yScrollbar.disabledColor.SetSetter([=](Color& currentValue, const Color& color) {
                currentValue = color;
                InvokeRedraw();
            });
            yScrollbar.hovered_.SetSetter([=](bool& currentValue, const bool& hovered) {
                currentValue = hovered;
                InvokeRedraw();
            });

            Panel::padding.ComputedFrom([](Rect padding, bool xBgVisible, int xWidth, bool yBgVisible, int yWidth) {
                padding.right += yBgVisible ? yWidth : 0;
                padding.bottom += xBgVisible ? xWidth : 0;
                return padding;
            }, padding, xScrollbar.backgroundVisible, xScrollbar.width, yScrollbar.backgroundVisible, yScrollbar.width);
        }

    public:
        struct Scrollbar
        {
            const ScrollbarDirection direction;
            Value<bool> scrollable = false;
            Value<int> scrollAmount_ = 0;
            Value<int> visualScrollAmount = 0;
            Value<int> stepSize = 100;
            Value<int> width = 10;
            Value<bool> backgroundVisible = false;
            Value<Color> color = Color(0xB4B4B4);
            Value<Color> backgroundColor = Color(0x121212);
            Value<Color> disabledColor = Color(0x343434);
            Value<bool> visibleOnScroll = true;
            Value<Duration> hangDuration = Duration(1, SECONDS);
            Value<Duration> fadeDuration = Duration(150, MILLISECONDS);
            Value<bool> interactable = true;
            Value<bool> hovered_ = false;
            Value<bool> held_ = false;
            Value<int> holdPos_ = 0;
            Value<bool> visible_ = false;
            Value<TimePoint> showTime_ = TimePoint();
            Value<float> opacity_ = 1.0f;
            Value<bool> scrolling_ = false;
            Value<TimePoint> scrollStartTime_ = TimePoint();
            Value<Duration> scrollDuration_ = Duration();
            Value<int> scrollStartPos_ = 0;
            Value<int> scrollEndPos_ = 0;
            std::function<float(float)> progressFunction_;
        };

        Scrollbar xScrollbar = { .direction = ScrollbarDirection::HORIZONTAL };
        Scrollbar yScrollbar = { .direction = ScrollbarDirection::VERTICAL };
        Value<Rect> padding = Rect{ 0, 0, 0, 0 };

        // While scrolling, if the cursor hovers an inner component which can also be scrolled, scrolling will switch to that inner component which is jarring to the user.
        // This can be avoided by locking out inner item scrolling for a short duration after scrolling, and scroll focus lifetime controls how long is the lockout.
        // Setting the value to 0, will disable this behavior.
        Value<Duration> scrollFocusLifetime = Duration(250, MILLISECONDS);

        int MaxScroll(Scrollbar& scrollbar) const;
        std::pair<float, float> ScrollbarWorkArea(Scrollbar& scrollbar) const;
        RectF ScrollbarHitbox(Scrollbar& scrollbar) const;
        bool ScrollbarHovered(Scrollbar& scrollbar) const;
        void Scroll(
            Scrollbar& scrollbar,
            int to,
            Duration scrollDuration = Duration(150, MILLISECONDS),
            std::function<float(float)> progressFunction = std::function<float(float)>());
        // Scrolls to the specified item.
        // If 'force' == true, scrolling is done even if item is already fully visible (currently unimplemented)
        void ScrollToItem(Component* item, bool force = false);

    private:
        TimePoint _lastScrollEvent = TimePoint(0) - scrollFocusLifetime;

        void _UpdateScrollbar(Scrollbar& scrollbar);

        bool _Vertical(const Scrollbar& scrollbar) const
        {
            return scrollbar.direction == ScrollbarDirection::VERTICAL;
        }
        ScrollbarDirection _OppositeOf(ScrollbarDirection direction) const
        {
            return direction == ScrollbarDirection::VERTICAL ? ScrollbarDirection::HORIZONTAL : ScrollbarDirection::VERTICAL;
        }
        const Scrollbar& _OppositeOf(Scrollbar& scrollbar) const
        {
            return _Vertical(scrollbar) ? xScrollbar : yScrollbar;
        }

    protected:
        void _OnUpdate() override
        {
            _UpdateScrollbar(xScrollbar);
            _UpdateScrollbar(yScrollbar);

            xScrollbar.hovered_ = ScrollbarHovered(xScrollbar) && xScrollbar.interactable;
            yScrollbar.hovered_ = ScrollbarHovered(yScrollbar) && yScrollbar.interactable;

            Panel::_OnUpdate();
        }

        void _OnDraw(Graphics* g) override
        {
            DrawParams params{};
            params.contentOffset = { (float)-xScrollbar.visualScrollAmount, (float)-yScrollbar.visualScrollAmount };
            Panel::_OnDraw(g, params);

            // Draw vertical scrollbar
            if ((MaxScroll(yScrollbar) > 0 && yScrollbar.visible_) || yScrollbar.backgroundVisible)
            {
                // Draw scroll background
                if (yScrollbar.backgroundVisible)
                {
                    RectF backgroundRect{};
                    backgroundRect.left = (float)size_->width - yScrollbar.width;
                    backgroundRect.right = (float)size_->width;
                    backgroundRect.top = 0.0f;
                    backgroundRect.bottom = (float)size_->height;
                    g->FillRectangle(backgroundRect, yScrollbar.backgroundColor);
                }

                // Draw scrollbar
                Color color = yScrollbar.color;
                if (MaxScroll(yScrollbar) == 0)
                {
                    color = yScrollbar.disabledColor;
                }
                else
                {
                    if (yScrollbar.hovered_ || yScrollbar.held_)
                        color.a = uint8_t(1.0f * 255.0f);
                    else
                        color.a = uint8_t(0.5f * 255.0f);
                    if (!yScrollbar.backgroundVisible)
                        color.a = uint8_t(color.a * yScrollbar.opacity_);
                }
                RoundedRect scrollbarRect{};
                scrollbarRect.radiusX = 2.0f;
                scrollbarRect.radiusY = 2.0f;
                scrollbarRect.rect = ScrollbarHitbox(yScrollbar).ShrunkBy(2.0f);
                g->FillRoundedRectangle(scrollbarRect, color);
            }
            // Draw horizontal scrollbar
            if ((MaxScroll(xScrollbar) > 0 && xScrollbar.visible_) || xScrollbar.backgroundVisible)
            {
                // Draw scroll background
                if (xScrollbar.backgroundVisible)
                {
                    RectF backgroundRect{};
                    backgroundRect.left = 0.0f;
                    backgroundRect.right = (float)size_->width;
                    backgroundRect.top = (float)size_->height - xScrollbar.width;
                    backgroundRect.bottom = (float)size_->height;
                    g->FillRectangle(backgroundRect, xScrollbar.backgroundColor);
                }

                // Draw scrollbar
                Color color = xScrollbar.color;
                if (MaxScroll(xScrollbar) == 0)
                {
                    color = xScrollbar.disabledColor;
                }
                else
                {
                    if (xScrollbar.hovered_ || xScrollbar.held_)
                        color.a = uint8_t(1.0f * 255.0f);
                    else
                        color.a = uint8_t(0.5f * 255.0f);
                    if (!xScrollbar.backgroundVisible)
                        color.a = uint8_t(color.a * xScrollbar.opacity_);
                }
                RoundedRect scrollbarRect{};
                scrollbarRect.radiusX = 2.0f;
                scrollbarRect.radiusY = 2.0f;
                scrollbarRect.rect = ScrollbarHitbox(xScrollbar).ShrunkBy(2.0f);
                g->FillRoundedRectangle(scrollbarRect, color);
            }
        }

        EventContext _OnMouseMove(Point point, Point deltaPos) override
        {
            // Scroll
            if (yScrollbar.held_)
            {
                if (deltaPos.y != 0)
                {
                    auto workArea = ScrollbarWorkArea(yScrollbar);
                    auto hitbox = ScrollbarHitbox(yScrollbar);
                    float scrollableLength = (workArea.second - workArea.first) - (hitbox.bottom - hitbox.top);
                    float scrolledLength = mousePosition_->y - workArea.first - yScrollbar.holdPos_;
                    float scrollRatio = scrolledLength / scrollableLength;
                    yScrollbar.visualScrollAmount = (int)std::roundf(MaxScroll(yScrollbar) * scrollRatio);
                }
                return EventContext().Add(this, point);
            }
            if (xScrollbar.held_)
            {
                if (deltaPos.x != 0)
                {
                    auto workArea = ScrollbarWorkArea(xScrollbar);
                    auto hitbox = ScrollbarHitbox(xScrollbar);
                    float scrollableLength = (workArea.second - workArea.first) - (hitbox.right - hitbox.left);
                    float scrolledLength = mousePosition_->x - workArea.first - xScrollbar.holdPos_;
                    float scrollRatio = scrolledLength / scrollableLength;
                    xScrollbar.visualScrollAmount = (int)std::roundf(MaxScroll(xScrollbar) * scrollRatio);
                }
                return EventContext().Add(this, point);
            }

            auto targets = Panel::_OnMouseMove(point + Point{ xScrollbar.visualScrollAmount, yScrollbar.visualScrollAmount }, deltaPos);
            targets.RemoveLast();
            // Overwrite parent return, since it contains scroll adjusted coordinates
            return std::move(targets.Add(this, point));
        }

        EventContext _OnLeftPressed(Point point) override
        {
            // Grab scrollbar
            if (ScrollbarHovered(yScrollbar) && yScrollbar.backgroundVisible && yScrollbar.interactable)
            {
                yScrollbar.held_ = true;
                yScrollbar.holdPos_ = int(point.y - ScrollbarHitbox(yScrollbar).top);
                // Stop scroll animation
                yScrollbar.scrolling_ = false;
            }
            else if (ScrollbarHovered(xScrollbar) && xScrollbar.backgroundVisible && xScrollbar.interactable)
            {
                xScrollbar.held_ = true;
                xScrollbar.holdPos_ = int(point.x - ScrollbarHitbox(xScrollbar).left);
                // Stop scroll animation
                xScrollbar.scrolling_ = false;
            }

            auto targets = Panel::_OnLeftPressed(point + Point{ xScrollbar.visualScrollAmount, yScrollbar.visualScrollAmount });
            targets.RemoveLast();
            // Overwrite parent return, since it contains scroll adjusted coordinates
            return std::move(targets.Add(this, point));
        }

        EventContext _OnLeftReleased(std::optional<Point> point) override
        {
            // Release scrollbars
            if (yScrollbar.held_)
            {
                yScrollbar.held_ = false;
                InvokeRedraw();
            }
            if (xScrollbar.held_)
            {
                xScrollbar.held_ = false;
                InvokeRedraw();
            }

            auto targets = Panel::_OnLeftReleased(point.has_value()
                ? std::optional(point.value() + Point{ xScrollbar.visualScrollAmount, yScrollbar.visualScrollAmount })
                : std::nullopt);
            targets.RemoveLast();
            return std::move(targets.Add(this, point));
        }

        EventContext _OnRightPressed(Point point) override
        {
            auto targets = Panel::_OnRightPressed(point + Point{ xScrollbar.visualScrollAmount, yScrollbar.visualScrollAmount });
            targets.RemoveLast();
            return std::move(targets.Add(this, point));
        }

        EventContext _OnRightReleased(std::optional<Point> point) override
        {
            auto targets = Panel::_OnRightReleased(point.has_value()
                ? std::optional(point.value() + Point{ xScrollbar.visualScrollAmount, yScrollbar.visualScrollAmount })
                : std::nullopt);
            targets.RemoveLast();
            return std::move(targets.Add(this, point));
        }

        EventContext _OnWheelUp(Point point) override
        {
            auto targets = EventContext();
            if (ztime::Main() > _lastScrollEvent + scrollFocusLifetime)
                targets = Panel::_OnWheelUp(point + Point{ xScrollbar.visualScrollAmount, yScrollbar.visualScrollAmount });

            if (targets.Empty())
            {
                if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                {
                    if (xScrollbar.scrollable)
                    {
                        Scroll(xScrollbar, xScrollbar.scrollAmount_ - xScrollbar.stepSize);
                        _lastScrollEvent = ztime::Main();
                        return EventContext().Add(this, point);
                    }
                }
                else
                {
                    if (yScrollbar.scrollable)
                    {
                        Scroll(yScrollbar, yScrollbar.scrollAmount_ - yScrollbar.stepSize);
                        _lastScrollEvent = ztime::Main();
                        return EventContext().Add(this, point);
                    }
                }
            }
            return targets;
        }

        EventContext _OnWheelDown(Point point) override
        {
            auto targets = EventContext();
            if (ztime::Main() > _lastScrollEvent + scrollFocusLifetime)
                targets = Panel::_OnWheelDown(point + Point{ xScrollbar.visualScrollAmount, yScrollbar.visualScrollAmount });

            if (targets.Empty())
            {
                if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                {
                    if (xScrollbar.scrollable)
                    {
                        Scroll(xScrollbar, xScrollbar.scrollAmount_ + xScrollbar.stepSize);
                        _lastScrollEvent = ztime::Main();
                        return EventContext().Add(this, point);
                    }
                }
                else
                {
                    if (yScrollbar.scrollable)
                    {
                        Scroll(yScrollbar, yScrollbar.scrollAmount_ + yScrollbar.stepSize);
                        _lastScrollEvent = ztime::Main();
                        return EventContext().Add(this, point);
                    }
                }
            }
            return targets;
        }

        void _ComputeItemLayout() override
        {
            Panel::_ComputeItemLayout();

            if (yScrollbar.visualScrollAmount > MaxScroll(yScrollbar))
                yScrollbar.visualScrollAmount = MaxScroll(yScrollbar);
            if (xScrollbar.visualScrollAmount > MaxScroll(xScrollbar))
                xScrollbar.visualScrollAmount = MaxScroll(xScrollbar);
        }

        void _SetWindowPositions() override
        {
            //Panel::_SetWindowPositions();

            for (auto& _item : _items)
            {
                Component* item = _item.item;
                item->SetWindowPosition(windowPosition_.Get() + item->position_ - Point{xScrollbar.visualScrollAmount, yScrollbar.visualScrollAmount});
            }
        }

    protected:
        void _AddItem(Component* item, size_t position, bool transferOwnership) override
        {
            Panel::_AddItem(item, position, transferOwnership);

            // Intercept component selection event
            _items.back().selectHandler = item->SubscribeOnSelected([&](zcom::Component* srcItem, bool reverse)
            {
                // Do not touch standard bubbling behaviour
                _onSelected->InvokeAll(this, reverse);

                ScrollToItem(srcItem);
            });
        }

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;

            values.push_back(Rect::LeftValueProxy("left padding", std::make_any<Value<Rect>*>(&padding)));
            values.push_back(Rect::TopValueProxy("top padding", std::make_any<Value<Rect>*>(&padding)));
            values.push_back(Rect::RightValueProxy("right padding", std::make_any<Value<Rect>*>(&padding)));
            values.push_back(Rect::BottomValueProxy("bottom padding", std::make_any<Value<Rect>*>(&padding)));
            values.push_back(DurationValueProxy("scroll focus lifetime (ms)", std::make_any<Value<Duration>*>(&scrollFocusLifetime), MILLISECONDS));

            values.push_back(ValueProxy::BasicBoolValueProxy("y: scrollable", std::make_any<Value<bool>*>(&yScrollbar.scrollable)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("y: actual scroll amount", std::make_any<Value<int>*>(&yScrollbar.scrollAmount_)).Computed());
            values.push_back(ValueProxy::BasicIntValueProxy<int>("y: visual scroll amount", std::make_any<Value<int>*>(&yScrollbar.visualScrollAmount)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("y: step size", std::make_any<Value<int>*>(&yScrollbar.stepSize)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("y: width", std::make_any<Value<int>*>(&yScrollbar.width)));
            values.push_back(ValueProxy::BasicBoolValueProxy("y: background visible", std::make_any<Value<bool>*>(&yScrollbar.backgroundVisible)));
            values.push_back(ValueProxy::BasicColorValueProxy("y: color", std::make_any<Value<Color>*>(&yScrollbar.color)));
            values.push_back(ValueProxy::BasicColorValueProxy("y: background color", std::make_any<Value<Color>*>(&yScrollbar.backgroundColor)));
            values.push_back(ValueProxy::BasicColorValueProxy("y: disabled color", std::make_any<Value<Color>*>(&yScrollbar.disabledColor)));
            values.push_back(ValueProxy::BasicBoolValueProxy("y: visible on scroll", std::make_any<Value<bool>*>(&yScrollbar.visibleOnScroll)));
            values.push_back(DurationValueProxy("y: hang duration (ms)", std::make_any<Value<Duration>*>(&yScrollbar.hangDuration), MILLISECONDS));
            values.push_back(DurationValueProxy("y: fade duration (ms)", std::make_any<Value<Duration>*>(&yScrollbar.fadeDuration), MILLISECONDS));
            values.push_back(ValueProxy::BasicBoolValueProxy("y: interactable", std::make_any<Value<bool>*>(&yScrollbar.interactable)));
            values.push_back(ValueProxy::BasicBoolValueProxy("y: hovered", std::make_any<Value<bool>*>(&yScrollbar.hovered_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("y: held", std::make_any<Value<bool>*>(&yScrollbar.held_)).Computed());
            values.push_back(ValueProxy::BasicIntValueProxy<int>("y: hold position", std::make_any<Value<int>*>(&yScrollbar.holdPos_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("y: visible", std::make_any<Value<bool>*>(&yScrollbar.visible_)).Computed());
            values.push_back(TimePointValueProxy("y: show time (ms)", std::make_any<Value<TimePoint>*>(&yScrollbar.showTime_), MILLISECONDS).Computed());
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("y: opacity", std::make_any<Value<float>*>(&yScrollbar.opacity_), 3).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("y: scrolling", std::make_any<Value<bool>*>(&yScrollbar.scrolling_)).Computed());
            values.push_back(TimePointValueProxy("y: scroll start time (ms)", std::make_any<Value<TimePoint>*>(&yScrollbar.scrollStartTime_), MILLISECONDS).Computed());
            values.push_back(DurationValueProxy("y: scroll duration (ms)", std::make_any<Value<Duration>*>(&yScrollbar.scrollDuration_), MILLISECONDS).Computed());
            values.push_back(ValueProxy::BasicIntValueProxy<int>("y: scroll start position", std::make_any<Value<int>*>(&yScrollbar.scrollStartPos_)).Computed());
            values.push_back(ValueProxy::BasicIntValueProxy<int>("y: scroll end position", std::make_any<Value<int>*>(&yScrollbar.scrollEndPos_)).Computed());

            values.push_back(ValueProxy::BasicBoolValueProxy("x: scrollable", std::make_any<Value<bool>*>(&yScrollbar.scrollable)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("x: actual scroll amount", std::make_any<Value<int>*>(&yScrollbar.scrollAmount_)).Computed());
            values.push_back(ValueProxy::BasicIntValueProxy<int>("x: visual scroll amount", std::make_any<Value<int>*>(&yScrollbar.visualScrollAmount)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("x: step size", std::make_any<Value<int>*>(&yScrollbar.stepSize)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("x: width", std::make_any<Value<int>*>(&yScrollbar.width)));
            values.push_back(ValueProxy::BasicBoolValueProxy("x: background visible", std::make_any<Value<bool>*>(&yScrollbar.backgroundVisible)));
            values.push_back(ValueProxy::BasicColorValueProxy("x: color", std::make_any<Value<Color>*>(&yScrollbar.color)));
            values.push_back(ValueProxy::BasicColorValueProxy("x: background color", std::make_any<Value<Color>*>(&yScrollbar.backgroundColor)));
            values.push_back(ValueProxy::BasicColorValueProxy("x: disabled color", std::make_any<Value<Color>*>(&yScrollbar.disabledColor)));
            values.push_back(ValueProxy::BasicBoolValueProxy("x: visible on scroll", std::make_any<Value<bool>*>(&yScrollbar.visibleOnScroll)));
            values.push_back(DurationValueProxy("x: hang duration (ms)", std::make_any<Value<Duration>*>(&yScrollbar.hangDuration), MILLISECONDS));
            values.push_back(DurationValueProxy("x: fade duration (ms)", std::make_any<Value<Duration>*>(&yScrollbar.fadeDuration), MILLISECONDS));
            values.push_back(ValueProxy::BasicBoolValueProxy("x: interactable", std::make_any<Value<bool>*>(&yScrollbar.interactable)));
            values.push_back(ValueProxy::BasicBoolValueProxy("x: hovered", std::make_any<Value<bool>*>(&yScrollbar.hovered_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("x: held", std::make_any<Value<bool>*>(&yScrollbar.held_)).Computed());
            values.push_back(ValueProxy::BasicIntValueProxy<int>("x: hold position", std::make_any<Value<int>*>(&yScrollbar.holdPos_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("x: visible", std::make_any<Value<bool>*>(&yScrollbar.visible_)).Computed());
            values.push_back(TimePointValueProxy("x: show time (ms)", std::make_any<Value<TimePoint>*>(&yScrollbar.showTime_), MILLISECONDS).Computed());
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("x: opacity", std::make_any<Value<float>*>(&yScrollbar.opacity_), 3).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("x: scrolling", std::make_any<Value<bool>*>(&yScrollbar.scrolling_)).Computed());
            values.push_back(TimePointValueProxy("x: scroll start time (ms)", std::make_any<Value<TimePoint>*>(&yScrollbar.scrollStartTime_), MILLISECONDS).Computed());
            values.push_back(DurationValueProxy("x: scroll duration (ms)", std::make_any<Value<Duration>*>(&yScrollbar.scrollDuration_), MILLISECONDS).Computed());
            values.push_back(ValueProxy::BasicIntValueProxy<int>("x: scroll start position", std::make_any<Value<int>*>(&yScrollbar.scrollStartPos_)).Computed());
            values.push_back(ValueProxy::BasicIntValueProxy<int>("x: scroll end position", std::make_any<Value<int>*>(&yScrollbar.scrollEndPos_)).Computed());

            auto data = Panel::GetReflectionData();
            data.insert(data.begin(), { "Scroll panel", std::move(values) });
            return data;
        }
    };
}