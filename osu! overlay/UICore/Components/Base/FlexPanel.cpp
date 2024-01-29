#include "FlexPanel.h"

#include <numeric>

void zcom::FlexPanel::_RecalculateLayout(int width, int height)
{
    bool horizontalFlex = (_direction == FlexDirection::LEFT || _direction == FlexDirection::RIGHT);
    bool reversedFlex = (_direction == FlexDirection::LEFT || _direction == FlexDirection::UP);

    // Layout is first calculated using coordinates aligned with the flex direction for simpler evaluation
    // Here 'along' means x when panel direction is horizontal and y when vertical
    // 'perp' (perpendicular) means y when panel direction is horizontal and x when vertical
    struct ItemLayoutDesc
    {
        // Input properties

        int baseSizeAlong;
        int baseSizePerp;
        float parentSizeRatioAlong;
        float parentSizeRatioPerp;
        int offsetAlong;
        int offsetPerp;
        float offsetRatioAlong;
        float offsetRatioPerp;
        Alignment alignmentAlong;
        Alignment alignmentPerp;
        std::optional<float> flexGrow = std::nullopt;
        std::optional<float> flexShrink = std::nullopt;
        std::optional<int> flexMaxSize = std::nullopt;
        std::optional<int> flexMinSize = std::nullopt;
        std::optional<Alignment> flexAlign = std::nullopt;

        // Output properties

        int calculatedSizeAlong;
        int sizeAlong;
        int sizePerp;
        int posAlong;
        int posPerp;

        // Other
        bool last = false;
    };

    // Calculate positions using only visible items (opacity of 0 can be used to provide invisibility without layout changes)
    std::vector<Component*> visibleItems;
    for (auto& item : _items)
        if (item.item->GetVisible())
            visibleItems.push_back(item.item);

    std::vector<ItemLayoutDesc> layoutDescs(visibleItems.size());
    for (int i = 0; i < visibleItems.size(); i++)
    {
        Component* item = visibleItems[i];
        layoutDescs[i].baseSizeAlong = item->GetBaseWidth();
        layoutDescs[i].baseSizePerp = item->GetBaseHeight();
        layoutDescs[i].parentSizeRatioAlong = item->GetParentWidthPercent();
        layoutDescs[i].parentSizeRatioPerp = item->GetParentHeightPercent();
        layoutDescs[i].offsetAlong = item->GetHorizontalOffsetPixels();
        layoutDescs[i].offsetPerp = item->GetVerticalOffsetPixels();
        layoutDescs[i].offsetRatioAlong = item->GetHorizontalOffsetPercent();
        layoutDescs[i].offsetRatioPerp = item->GetVerticalOffsetPercent();
        layoutDescs[i].alignmentAlong = item->GetHorizontalAlignment();
        layoutDescs[i].alignmentPerp = item->GetVerticalAlignment();
        if (!horizontalFlex)
        {
            std::swap(layoutDescs[i].baseSizeAlong, layoutDescs[i].baseSizePerp);
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

        if (i == visibleItems.size() - 1)
            layoutDescs[i].last = true;
    }
    RECT padding = GetPadding();
    int sizeAlong = GetWidth();
    int sizePerp = GetHeight();
    int paddingAlong = padding.left + padding.right;
    int paddingPerp = padding.top + padding.bottom;
    int sizeWithoutPaddingAlong = sizeAlong - paddingAlong;
    int sizeWithoutPaddingPerp = sizePerp - paddingPerp;
    bool sizeFixedAlong = IsWidthFixed();
    bool sizeFixedPerp = IsHeightFixed();
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
            item.calculatedSizeAlong = (int)std::round(item.parentSizeRatioAlong * sizeWithoutPaddingAlong) + item.baseSizeAlong;
            item.sizeAlong = item.calculatedSizeAlong; // Will be overwritten if flex grow/shrink is applied
            totalCalculatedSizeAlong += item.calculatedSizeAlong + (!item.last ? _spacing : 0);
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
            item.sizeAlong = item.baseSizeAlong;
        if (sizeFixedPerp)
            item.sizePerp = (int)std::round(item.parentSizeRatioPerp * sizeWithoutPaddingPerp) + item.baseSizePerp;
        else
            item.sizePerp = item.baseSizePerp;
    }

    // Offset along flex direction
    int layoutSizeAlong = 0;
    for (auto& item : layoutDescs)
        layoutSizeAlong += item.sizeAlong + (!item.last ? _spacing : 0);
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
            item.posAlong = offset;
        else
            item.posAlong = reversedStartPos - offset - item.sizeAlong;
        offset += item.sizeAlong + _spacing;
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
        if (_itemAlignment.has_value())
        {
            alignment = _itemAlignment.value();
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
    for (int i = 0; i < visibleItems.size(); i++)
    {
        Component* item = visibleItems[i];
        if (!horizontalFlex)
        {
            std::swap(layoutDescs[i].posAlong, layoutDescs[i].posPerp);
            std::swap(layoutDescs[i].sizeAlong, layoutDescs[i].sizePerp);
        }
        item->SetPosition(
            layoutDescs[i].posAlong + item->GetHorizontalOffsetPixels() + padding.left,
            layoutDescs[i].posPerp + item->GetVerticalOffsetPixels() + padding.top
        );
        item->Resize(layoutDescs[i].sizeAlong, layoutDescs[i].sizePerp);
    }

    // Panel size
    int newSizeAlong = sizeAlong;
    int newSizePerp = sizePerp;
    if (!sizeFixedAlong)
        newSizeAlong = layoutSizeAlong + paddingAlong;
    if (!sizeFixedPerp)
        newSizePerp = layoutSizePerp + paddingPerp;

    if (newSizeAlong != sizeAlong || newSizePerp != sizePerp)
    {
        if (horizontalFlex)
        {
            // Force base size to remain unchanged if size in that dimension is fixed
            // This is necessary, because sizeAlong/sizePerp do not match user set base size values right after creating the panel
            SetBaseSize(
                IsWidthFixed() ? GetBaseWidth() : newSizeAlong,
                IsHeightFixed() ? GetBaseHeight() : newSizePerp
            );
        }
        else
        {
            SetBaseSize(
                IsWidthFixed() ? GetBaseWidth() : newSizePerp,
                IsHeightFixed() ? GetBaseHeight() : newSizeAlong
            );
        }
    }

    _SetWindowPositions();
    InvokeRedraw();
}