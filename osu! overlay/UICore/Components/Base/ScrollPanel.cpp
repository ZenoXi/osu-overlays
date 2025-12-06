#include "ScrollPanel.h"

int zcom::ScrollPanel::MaxScroll(Scrollbar& scrollbar) const
{
    if (_Vertical(scrollbar))
    {
        int maxScroll = contentSize_->height - size_->height + (xScrollbar.backgroundVisible ? xScrollbar.width.Get() : 0);
        if (maxScroll < 0)
            maxScroll = 0;
        return maxScroll;
    }
    else
    {
        int maxScroll = contentSize_->width - size_->width + (yScrollbar.backgroundVisible ? yScrollbar.width.Get() : 0);
        if (maxScroll < 0)
            maxScroll = 0;
        return maxScroll;
    }
}

std::pair<float, float> zcom::ScrollPanel::ScrollbarWorkArea(Scrollbar& scrollbar) const
{
    float startOffset = 10.0f;
    float endOffset = (float)_OppositeOf(scrollbar).width;
    if (scrollbar.backgroundVisible)
    {
        startOffset = 0.0f;
        if (!_OppositeOf(scrollbar).backgroundVisible)
            endOffset = 0.0f;
    }
    return { startOffset, (_Vertical(scrollbar) ? size_->height : size_->width) - endOffset };
}

zcom::RectF zcom::ScrollPanel::ScrollbarHitbox(Scrollbar& scrollbar) const
{
    auto workArea = ScrollbarWorkArea(scrollbar);
    float startPos = workArea.first;
    float endPos = workArea.second;
    float hitboxWidth = (float)scrollbar.width;

    // Calculate scrollbar layout
    float sizeToContentRatio = _Vertical(scrollbar)
        ? size_->height / (float)contentSize_->height
        : size_->width / (float)contentSize_->width;
    if (sizeToContentRatio > 1.0f)
        sizeToContentRatio = 1.0f;
    float scrollbarMaxLength = endPos - startPos;
    float scrollbarLength = scrollbarMaxLength * sizeToContentRatio;
    float scrollbarPosition = 0.0f;
    if (MaxScroll(scrollbar) != 0)
        scrollbarPosition = (scrollbarMaxLength - scrollbarLength) * (scrollbar.visualScrollAmount / (float)MaxScroll(scrollbar));

    if (_Vertical(scrollbar))
    {
        RectF hitbox{};
        hitbox.left = size_->width - hitboxWidth;
        hitbox.right = (float)size_->width;
        hitbox.top = startPos + scrollbarPosition;
        hitbox.bottom = hitbox.top + scrollbarLength;
        return hitbox;
    }
    else
    {
        RectF hitbox{};
        hitbox.left = startPos + scrollbarPosition;
        hitbox.right = hitbox.left + scrollbarLength;
        hitbox.top = size_->height - hitboxWidth;
        hitbox.bottom = (float)size_->height;
        return hitbox;
    }
}

bool zcom::ScrollPanel::ScrollbarHovered(Scrollbar& scrollbar) const
{
    auto hitbox = ScrollbarHitbox(scrollbar);
    return hovered_ &&
        mousePosition_->x >= hitbox.left &&
        mousePosition_->x < hitbox.right &&
        mousePosition_->y >= hitbox.top &&
        mousePosition_->y < hitbox.bottom;
}

void zcom::ScrollPanel::Scroll(Scrollbar& scrollbar, int to, Duration scrollDuration, std::function<float(float)> progressFunction)
{
    if (!scrollbar.scrollable)
        return;

    if (to < 0)
        to = 0;
    else if (to > MaxScroll(scrollbar))
        to = MaxScroll(scrollbar);

    scrollbar.scrollStartTime_ = ztime::Main();
    scrollbar.scrollDuration_ = scrollDuration;
    // 'scrolling_' should be set to true after the following line because it is used in 'scrollAmount_' calculation
    // and setting it to true assigns 'scrollEndPos_' to 'scrollAmount_' leading to bad 'scrollStartPos_'
    scrollbar.scrollStartPos_ = scrollbar.scrollAmount_.Get();
    scrollbar.scrollEndPos_ = to;
    scrollbar.progressFunction_ = progressFunction;
    scrollbar.scrolling_ = true;

    if (scrollbar.visibleOnScroll)
    {
        scrollbar.visible_ = true;
        scrollbar.showTime_ = ztime::Main();
    }

    InvokeRedraw();
}

void zcom::ScrollPanel::ScrollToItem(Component* item, bool force)
{
    // TODO: add 'force' == true functionality

    std::optional<Point> itemPos = FindChildRelativeOffset(item);
    if (!itemPos)
        return;
    int itemPosX = itemPos.value().x;
    int itemPosY = itemPos.value().y;

    // Vertically
    if (yScrollbar.scrollable)
    {
        if (itemPosY < yScrollbar.scrollAmount_)
            Scroll(yScrollbar, itemPosY);
        else if (itemPosY + item->size_->height > yScrollbar.scrollAmount_ + size_->height)
            Scroll(yScrollbar, itemPosY + item->size_->height - size_->height);
    }
    // Horizontally
    if (xScrollbar.scrollable)
    {
        if (itemPosX < xScrollbar.scrollAmount_)
            Scroll(xScrollbar, itemPosX);
        else if (itemPosX + item->size_->width > xScrollbar.scrollAmount_ + size_->width)
            Scroll(xScrollbar, itemPosX + item->size_->width - size_->width);
    }
}

void zcom::ScrollPanel::_UpdateScrollbar(Scrollbar& scrollbar)
{
    if (scrollbar.hangDuration == 0)
    {
        if (MaxScroll(scrollbar) != 0 && !scrollbar.visible_)
        {
            scrollbar.visible_ = true;
            InvokeRedraw();
        }
        else if (MaxScroll(scrollbar) == 0 && scrollbar.visible_)
        {
            scrollbar.visible_ = false;
            InvokeRedraw();
        }
    }

    // Fade scrollbar
    if (scrollbar.visible_)
    {
        if (scrollbar.hangDuration == 0)
        {
            if (scrollbar.opacity_ != 1.0f)
            {
                scrollbar.opacity_ = 1.0f;
                InvokeRedraw();
            }
        }
        else
        {
            Duration timeElapsed = ztime::Main() - scrollbar.showTime_;
            if (timeElapsed > scrollbar.hangDuration)
            {
                timeElapsed -= scrollbar.hangDuration;
                if (timeElapsed > scrollbar.fadeDuration)
                {
                    scrollbar.visible_ = false;
                    scrollbar.opacity_ = 0.0f;
                }
                else
                {
                    float fadeProgress = timeElapsed.GetDuration() / (float)scrollbar.fadeDuration->GetDuration();
                    scrollbar.opacity_ = 1.0f - powf(fadeProgress, 0.5f);
                }
                InvokeRedraw();
            }
            else
            {
                scrollbar.opacity_ = 1.0f;
            }
        }
    }

    // Animate scroll
    if (scrollbar.scrolling_)
    {
        float timeProgress = (ztime::Main() - scrollbar.scrollStartTime_).GetDuration() / (float)scrollbar.scrollDuration_->GetDuration();
        if (timeProgress >= 1.0f)
        {
            scrollbar.scrolling_ = false;
            scrollbar.visualScrollAmount = scrollbar.scrollEndPos_.Get();
        }
        else
        {
            float moveProgress;
            if (scrollbar.progressFunction_)
                moveProgress = scrollbar.progressFunction_(timeProgress);
            else
                moveProgress = 1.0f - powf(timeProgress - 1.0f, 2.0f);

            int startPos = scrollbar.scrollStartPos_;
            int endPos = scrollbar.scrollEndPos_;
            scrollbar.visualScrollAmount = int(startPos + (endPos - startPos) * moveProgress);
        }
        _RecalculateLayout();
        // TODO: only resend mouse move messages if mouse is in the component
        OnMouseMove(mousePosition_);
    }
}