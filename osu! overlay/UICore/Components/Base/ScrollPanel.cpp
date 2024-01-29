#include "ScrollPanel.h"

bool zcom::ScrollPanel::Scrollable(Scrollbar direction) const
{
    return _Scrollable(_GetConstScrollbar(direction));
}

void zcom::ScrollPanel::Scrollable(Scrollbar direction, bool scrollable)
{
    _Scrollable(_GetScrollbar(direction), scrollable);
}

int zcom::ScrollPanel::MaxScroll(Scrollbar direction) const
{
    if (direction == Scrollbar::VERTICAL)
    {
        int maxScroll = GetContentHeight() - GetHeight() + (_horizontalScrollbar.backgroundVisible ? 10 : 0);
        if (maxScroll < 0)
            maxScroll = 0;
        return maxScroll;
    }
    else
    {
        int maxScroll = GetContentWidth() - GetWidth() + (_verticalScrollbar.backgroundVisible ? 10 : 0);
        if (maxScroll < 0)
            maxScroll = 0;
        return maxScroll;
    }
}

int zcom::ScrollPanel::ScrollPosition(Scrollbar direction) const
{
    return _ScrollPosition(_GetConstScrollbar(direction));
}

int zcom::ScrollPanel::VisualScrollPosition(Scrollbar direction) const
{
    return _VisualScrollPosition(_GetConstScrollbar(direction));
}

void zcom::ScrollPanel::ScrollPosition(Scrollbar direction, int position)
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

Duration zcom::ScrollPanel::ScrollbarHangDuration(Scrollbar direction) const
{
    return _ScrollbarHangDuration(_GetConstScrollbar(direction));
}

void zcom::ScrollPanel::ScrollbarHangDuration(Scrollbar direction, Duration duration)
{
    _ScrollbarHangDuration(_GetScrollbar(direction), duration);
}

bool zcom::ScrollPanel::ScrollBackgroundVisible(Scrollbar direction) const
{
    return _ScrollBackgroundVisible(_GetConstScrollbar(direction));
}

void zcom::ScrollPanel::ScrollBackgroundVisible(Scrollbar direction, bool visible)
{
    _ScrollBackgroundVisible(_GetScrollbar(direction), visible);
}

D2D1_COLOR_F zcom::ScrollPanel::ScrollBackgroundColor() const
{
    return _scrollbarBackgroundColor;
}

void zcom::ScrollPanel::ScrollBackgroundColor(D2D1_COLOR_F color)
{
    if (_scrollbarBackgroundColor == color)
        return;
    _scrollbarBackgroundColor = color;
    InvokeRedraw();
}

int zcom::ScrollPanel::ScrollStepSize(Scrollbar direction) const
{
    return _ScrollStepSize(_GetConstScrollbar(direction));
}

void zcom::ScrollPanel::ScrollStepSize(Scrollbar direction, int stepSize)
{
    _ScrollStepSize(_GetScrollbar(direction), stepSize);
}

int zcom::ScrollPanel::ScrollbarWidth(Scrollbar direction) const
{
    return _ScrollbarWidth(_GetConstScrollbar(direction));
}

void zcom::ScrollPanel::ScrollbarWidth(Scrollbar direction, int width)
{
    _ScrollbarWidth(_GetScrollbar(direction), width);
}

std::pair<float, float> zcom::ScrollPanel::ScrollbarWorkArea(Scrollbar direction) const
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

D2D1_RECT_F zcom::ScrollPanel::ScrollbarHitbox(Scrollbar direction) const
{
    if (direction == Scrollbar::VERTICAL)
    {
        auto workArea = ScrollbarWorkArea(direction);
        float startPos = workArea.first;
        float endPos = workArea.second;
        float hitboxWidth = ScrollbarWidth(Scrollbar::VERTICAL);

        // Calculate scrollbar layout
        float heightToContentRatio = GetHeight() / (float)GetContentHeight();
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
        float widthToContentRatio = GetWidth() / (float)GetContentWidth();
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

bool zcom::ScrollPanel::ScrollbarHovered(Scrollbar direction) const
{
    auto hitbox = ScrollbarHitbox(direction);
    return GetMouseInside() &&
        GetMousePosX() >= hitbox.left &&
        GetMousePosX() < hitbox.right &&
        GetMousePosY() >= hitbox.top &&
        GetMousePosY() < hitbox.bottom;
}

void zcom::ScrollPanel::Scroll(Scrollbar direction, int to, Duration scrollDuration, std::function<float(float)> progressFunction)
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

        InvokeRedraw();
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

        InvokeRedraw();
    }
}

void zcom::ScrollPanel::ScrollToItem(Component* item, bool force)
{
    // TODO: add 'force' == true functionality

    std::optional<std::pair<int, int>> itemPos = FindChildRelativeOffset(item);
    if (!itemPos)
        return;
    int itemPosX = itemPos.value().first;
    int itemPosY = itemPos.value().second;

    // Vertically
    if (_verticalScrollbar.scrollable)
    {
        if (itemPosY < _verticalScrollbar.scrollAmount)
            Scroll(Scrollbar::VERTICAL, itemPosY);
        else if (itemPosY + item->GetHeight() > _verticalScrollbar.scrollAmount + GetHeight())
            Scroll(Scrollbar::VERTICAL, itemPosY + item->GetHeight() - GetHeight());
    }
    // Horizontally
    if (_horizontalScrollbar.scrollable)
    {
        if (itemPosX < _horizontalScrollbar.scrollAmount)
            Scroll(Scrollbar::HORIZONTAL, itemPosX);
        else if (itemPosX + item->GetWidth() > _horizontalScrollbar.scrollAmount + GetWidth())
            Scroll(Scrollbar::HORIZONTAL, itemPosX + item->GetWidth() - GetWidth());
    }
}

void zcom::ScrollPanel::_UpdateScrollbar(Scrollbar direction)
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

const zcom::ScrollPanel::_Scrollbar& zcom::ScrollPanel::_GetConstScrollbar(Scrollbar direction) const
{
    if (direction == Scrollbar::VERTICAL)
        return _verticalScrollbar;
    else
        return _horizontalScrollbar;
}

zcom::ScrollPanel::_Scrollbar& zcom::ScrollPanel::_GetScrollbar(Scrollbar direction)
{
    if (direction == Scrollbar::VERTICAL)
        return _verticalScrollbar;
    else
        return _horizontalScrollbar;
}

bool zcom::ScrollPanel::_Scrollable(const _Scrollbar& scrollbar) const
{
    return scrollbar.scrollable;
}

void zcom::ScrollPanel::_Scrollable(_Scrollbar& scrollbar, bool scrollable)
{
    scrollbar.scrollable = scrollable;
}

int zcom::ScrollPanel::_ScrollPosition(const _Scrollbar& scrollbar) const
{
    if (scrollbar.scrollAnimation.inProgress)
        return scrollbar.scrollAnimation.endPos;
    else
        return scrollbar.scrollAmount;
}

int zcom::ScrollPanel::_VisualScrollPosition(const _Scrollbar& scrollbar) const
{
    return scrollbar.scrollAmount;
}

Duration zcom::ScrollPanel::_ScrollbarHangDuration(const _Scrollbar& scrollbar) const
{
    return scrollbar.hangDuration;
}

void zcom::ScrollPanel::_ScrollbarHangDuration(_Scrollbar& scrollbar, Duration duration)
{
    scrollbar.hangDuration = duration;
}

bool zcom::ScrollPanel::_ScrollBackgroundVisible(const _Scrollbar& scrollbar) const
{
    return scrollbar.backgroundVisible;
}

void zcom::ScrollPanel::_ScrollBackgroundVisible(_Scrollbar& scrollbar, bool visible)
{
    if (scrollbar.backgroundVisible == visible)
        return;
    scrollbar.backgroundVisible = visible;
    SetPadding(_basePadding);
}

int zcom::ScrollPanel::_ScrollStepSize(const _Scrollbar& scrollbar) const
{
    return scrollbar.stepSize;
}

void zcom::ScrollPanel::_ScrollStepSize(_Scrollbar& scrollbar, int stepSize)
{
    scrollbar.stepSize = stepSize;
}

int zcom::ScrollPanel::_ScrollbarWidth(const _Scrollbar& scrollbar) const
{
    return scrollbar.width;
}

void zcom::ScrollPanel::_ScrollbarWidth(_Scrollbar& scrollbar, int width)
{
    if (scrollbar.width == width)
        return;
    scrollbar.width = width;
    SetPadding(_basePadding);
}