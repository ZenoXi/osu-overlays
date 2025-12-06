#include "FlexPanel.h"

#include <numeric>

void zcom::FlexPanel::_ComputeItemLayout()
{
    bool horizontalFlex = (direction == FlexDirection::LEFT || direction == FlexDirection::RIGHT);
    bool reversedFlex = (direction == FlexDirection::LEFT || direction == FlexDirection::UP);

    // Layout is first calculated using coordinates aligned with the flex direction for simpler evaluation
    // Here 'along' means x when panel direction is horizontal and y when vertical
    // 'perp' (perpendicular) means y when panel direction is horizontal and x when vertical
    struct ItemLayoutDesc
    {
        // Input properties

        int baseSizeAlong = 0;
        int baseSizePerp = 0;
        int selfSizeAlong = 0;
        int selfSizePerp = 0;
        float parentSizeRatioAlong = 0;
        float parentSizeRatioPerp = 0;
        int offsetAlong = 0;
        int offsetPerp = 0;
        float offsetRatioAlong = 0;
        float offsetRatioPerp = 0;
        Alignment alignmentAlong = Alignment::START;
        Alignment alignmentPerp = Alignment::START;
        std::optional<float> flexGrow = std::nullopt;
        std::optional<float> flexShrink = std::nullopt;
        std::optional<int> flexMaxSize = std::nullopt;
        std::optional<int> flexMinSize = std::nullopt;
        std::optional<Alignment> flexAlign = std::nullopt;
        int flexMarginBefore = 0;
        int flexMarginAfter = 0;

        // Output properties

        int calculatedSizeAlong = 0;
        int sizeAlong = 0;
        int sizePerp = 0;
        int posAlong = 0;
        int posPerp = 0;

        // Other
        bool first = false;
        bool last = false;
    };

    std::vector<Component*> flexItems;
    std::vector<Component*> nonFlexItems;
    for (auto& item : _items)
    {
        // Calculate positions using only visible items (opacity of 0 can be used to provide invisibility without layout changes)
        if (!item.item->visible)
            continue;

        // Items with FlexIgnore property do not participate in flex layout
        if (item.item->GetProperty<FlexIgnore>().valid)
            nonFlexItems.push_back(item.item);
        else
            flexItems.push_back(item.item);
    }

    std::vector<ItemLayoutDesc> layoutDescs(flexItems.size());
    for (int i = 0; i < flexItems.size(); i++)
    {
        Component* item = flexItems[i];
        layoutDescs[i].baseSizeAlong = item->size->width;
        layoutDescs[i].baseSizePerp = item->size->height;
        layoutDescs[i].selfSizeAlong = item->selfSize_->width;
        layoutDescs[i].selfSizePerp = item->selfSize_->height;
        layoutDescs[i].parentSizeRatioAlong = item->parentSize->width;
        layoutDescs[i].parentSizeRatioPerp = item->parentSize->height;
        layoutDescs[i].offsetAlong = item->position->x;
        layoutDescs[i].offsetPerp = item->position->y;
        layoutDescs[i].offsetRatioAlong = item->parentPosition->x;
        layoutDescs[i].offsetRatioPerp = item->parentPosition->y;
        layoutDescs[i].alignmentAlong = item->xAlign;
        layoutDescs[i].alignmentPerp = item->yAlign;
        if (!horizontalFlex)
        {
            std::swap(layoutDescs[i].baseSizeAlong, layoutDescs[i].baseSizePerp);
            std::swap(layoutDescs[i].selfSizeAlong, layoutDescs[i].selfSizePerp);
            std::swap(layoutDescs[i].parentSizeRatioAlong, layoutDescs[i].parentSizeRatioPerp);
            std::swap(layoutDescs[i].offsetAlong, layoutDescs[i].offsetPerp);
            std::swap(layoutDescs[i].offsetRatioAlong, layoutDescs[i].offsetRatioPerp);
            std::swap(layoutDescs[i].alignmentAlong, layoutDescs[i].alignmentPerp);
        }
        FlexGrow flexGrowProp = item->GetProperty<FlexGrow>();
        FlexShrink flexShrinkProp = item->GetProperty<FlexShrink>();
        FlexMaxSize flexMaxSizeProp = item->GetProperty<FlexMaxSize>();
        FlexMinSize flexMinSizeProp = item->GetProperty<FlexMinSize>();
        FlexAlign flexAlignProp = item->GetProperty<FlexAlign>();
        FlexMarginBefore flexMarginBeforeProp = item->GetProperty<FlexMarginBefore>();
        FlexMarginAfter flexMarginAfterProp = item->GetProperty<FlexMarginAfter>();
        if (flexGrowProp.valid)
            layoutDescs[i].flexGrow = flexGrowProp.ratio;
        if (flexShrinkProp.valid)
            layoutDescs[i].flexShrink = flexShrinkProp.ratio;
        if (flexMaxSizeProp.valid)
            layoutDescs[i].flexMaxSize = flexMaxSizeProp.value;
        if (flexMinSizeProp.valid)
            layoutDescs[i].flexMinSize = flexMinSizeProp.value;
        if (flexAlignProp.valid)
            layoutDescs[i].flexAlign = flexAlignProp.value;
        if (flexMarginBeforeProp.valid)
            layoutDescs[i].flexMarginBefore = flexMarginBeforeProp.value;
        if (flexMarginAfterProp.valid)
            layoutDescs[i].flexMarginAfter = flexMarginAfterProp.value;

        if (i == 0)
            layoutDescs[i].first = true;
        if (i == flexItems.size() - 1)
            layoutDescs[i].last = true;
    }
    Rect padding = this->padding;
    int sizeAlong = size_->width;
    int sizePerp = size_->height;
    int paddingAlong = padding.left + padding.right;
    int paddingPerp = padding.top + padding.bottom;
    int sizeWithoutPaddingAlong = sizeAlong - paddingAlong;
    int sizeWithoutPaddingPerp = sizePerp - paddingPerp;
    bool sizeFixedAlong = !autoWidth;
    bool sizeFixedPerp = !autoHeight;
    if (!horizontalFlex)
    {
        std::swap(sizeAlong, sizePerp);
        std::swap(paddingAlong, paddingPerp);
        std::swap(sizeWithoutPaddingAlong, sizeWithoutPaddingPerp);
        std::swap(sizeFixedAlong, sizeFixedPerp);
    }

    // Flex parameters apply, calculate sizes with grow/shrink
    if (sizeFixedAlong)
    {
        int totalCalculatedSizeAlong = 0;
        float flexGrowRatioSum = 0.0f;
        float flexShrinkRatioSum = 0.0f;
        for (auto& item : layoutDescs)
        {
            item.calculatedSizeAlong = (int)std::round(item.parentSizeRatioAlong * sizeWithoutPaddingAlong) + item.baseSizeAlong + item.selfSizeAlong;
            item.sizeAlong = item.calculatedSizeAlong; // Will be overwritten if flex grow/shrink is applied
            totalCalculatedSizeAlong += item.calculatedSizeAlong + (!item.last ? spacing.Get() : 0) + item.flexMarginBefore + item.flexMarginAfter;
            flexGrowRatioSum += item.flexGrow.value_or(0.0f);
            flexShrinkRatioSum += item.flexShrink.value_or(0.0f);
        }
        if (totalCalculatedSizeAlong < sizeWithoutPaddingAlong && flexGrowRatioSum != 0.0f)
        {
            int growAmount = sizeWithoutPaddingAlong - totalCalculatedSizeAlong;
            for (auto& item : layoutDescs)
            {
                if (!item.flexGrow.has_value())
                    continue;

                float growRatio = item.flexGrow.value() / flexGrowRatioSum;
                int newSize = (int)std::round(item.calculatedSizeAlong + growAmount * growRatio);
                int maxSize = item.flexMaxSize.value_or(std::numeric_limits<int>::max());
                item.sizeAlong = std::min(newSize, maxSize);
            }
        }
        else if (totalCalculatedSizeAlong > sizeWithoutPaddingAlong && flexShrinkRatioSum != 0.0f)
        {
            int shrinkAmount = totalCalculatedSizeAlong - sizeWithoutPaddingAlong;
            for (auto& item : layoutDescs)
            {
                if (!item.flexShrink.has_value())
                    continue;

                float shrinkRatio = item.flexShrink.value() / flexShrinkRatioSum;
                int newSize = (int)std::round(item.calculatedSizeAlong - shrinkAmount * shrinkRatio);
                int minSize = item.flexMinSize.value_or(0);
                item.sizeAlong = std::max(newSize, minSize);
            }
        }
    }
    // Other sizing cases
    for (auto& item : layoutDescs)
    {
        if (!sizeFixedAlong)
            item.sizeAlong = item.baseSizeAlong + item.selfSizeAlong;
        if (sizeFixedPerp)
            item.sizePerp = (int)std::round(item.parentSizeRatioPerp * sizeWithoutPaddingPerp) + item.baseSizePerp + item.selfSizePerp;
        else
            item.sizePerp = item.baseSizePerp + item.selfSizePerp;
    }

    // Offset along flex direction
    int layoutSizeAlong = 0;
    for (auto& item : layoutDescs)
        layoutSizeAlong += item.sizeAlong + (!item.last ? spacing.Get() : 0) + item.flexMarginBefore + item.flexMarginAfter;
    int offset = 0;
    int reversedStartPos = 0;
    if (reversedFlex)
    {
        if (!sizeFixedAlong)
            reversedStartPos = layoutSizeAlong;
        else
            reversedStartPos = sizeWithoutPaddingAlong;
    }
    for (auto& item : layoutDescs)
    {
        if (!reversedFlex)
            item.posAlong = offset + item.flexMarginBefore;
        else
            item.posAlong = reversedStartPos - offset - item.flexMarginBefore - item.sizeAlong;
        offset += item.sizeAlong + spacing.Get() + item.flexMarginBefore + item.flexMarginAfter;
    }
    // Offset perpendicular to flex direction
    int layoutSizePerp = sizeWithoutPaddingPerp;
    if (!sizeFixedPerp)
    {
        layoutSizePerp = 0;
        for (auto& item : layoutDescs)
        {
            if (item.sizePerp > layoutSizePerp)
                layoutSizePerp = item.sizePerp;
        }
    }
    for (auto& item : layoutDescs)
    {
        Alignment alignment;
        if (itemAlignment->has_value())
        {
            alignment = itemAlignment->value();
            if (item.flexAlign.has_value())
                alignment = item.flexAlign.value();
        }
        else
        {
            alignment = item.alignmentPerp;
        }

        if (alignment == Alignment::START)
            item.posPerp = (int)std::round((layoutSizePerp - item.sizePerp) * item.offsetRatioPerp);
        else if (alignment == Alignment::CENTER)
            item.posPerp = (layoutSizePerp - item.sizePerp) / 2;
        else if (alignment == Alignment::END)
            item.posPerp = (int)std::round((layoutSizePerp - item.sizePerp) * (1.0f - item.offsetRatioPerp));
    }

    // Translate values back into x/y space
    for (int i = 0; i < flexItems.size(); i++)
    {
        Component* item = flexItems[i];
        if (!horizontalFlex)
        {
            std::swap(layoutDescs[i].posAlong, layoutDescs[i].posPerp);
            std::swap(layoutDescs[i].sizeAlong, layoutDescs[i].sizePerp);
        }
        item->SetPosition({
            layoutDescs[i].posAlong + item->position->x + padding.left,
            layoutDescs[i].posPerp + item->position->y + padding.top
        });
        item->Resize({ layoutDescs[i].sizeAlong, layoutDescs[i].sizePerp });
    }

    // Panel size
    int newSizeAlong = sizeAlong;
    int newSizePerp = sizePerp;
    if (!sizeFixedAlong)
        newSizeAlong = layoutSizeAlong + paddingAlong;
    if (!sizeFixedPerp)
        newSizePerp = layoutSizePerp + paddingPerp;

    if (horizontalFlex)
    {
        selfSize_ = {
            autoWidth ? newSizeAlong : 0,
            autoHeight ? newSizePerp : 0
        };
    }
    else
    {
        selfSize_ = {
            autoWidth ? newSizePerp : 0,
            autoHeight ? newSizeAlong : 0
        };
    }

    Size flexSize = {
        autoWidth ? size->width + selfSize_->width : size_->width,
        autoHeight ? size->height + selfSize_->height : size_->height
    };

    int widthWithoutPadding = flexSize.width - padding.left - padding.right;
    int heightWithoutPadding = flexSize.height - padding.top - padding.bottom;

    // Calculate item sizes and positions
    for (auto& item : nonFlexItems)
    {
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
        newPosX += padding.left;

        int newPosY = 0;
        if (item->yAlign == Alignment::START)
            newPosY = (int)std::round((heightWithoutPadding - newHeight) * item->parentPosition->y);
        else if (item->yAlign == Alignment::CENTER)
            newPosY = (heightWithoutPadding - newHeight) / 2;
        else if (item->yAlign == Alignment::END)
            newPosY = (int)std::round((heightWithoutPadding - newHeight) * (1.0f - item->parentPosition->y));
        newPosY += item->position->y;
        newPosY += padding.top;

        item->SetPosition({ newPosX, newPosY });
        item->Resize({ newWidth, newHeight });
    }

    _SetWindowPositions();
    InvokeRedraw();
}