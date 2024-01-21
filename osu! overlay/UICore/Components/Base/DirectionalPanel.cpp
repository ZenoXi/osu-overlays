#include "DirectionalPanel.h"

void zcom::DirectionalPanel::_RecalculateLayout(int width, int height)
{
    DeferLayoutUpdates();

    int offset = 0;
    for (auto& _item : _items)
    {
        Component* item = _item.item;
        // Position items
        if (_direction == PanelDirection::DOWN)
        {
            item->SetVerticalAlignment(Alignment::START);
            item->SetParentHeightPercent(0.0f);
            item->SetVerticalOffsetPercent(0.0f);
            item->SetVerticalOffsetPixels(offset);
            offset += item->GetBaseHeight() + _spacing;
        }
        else if (_direction == PanelDirection::UP)
        {
            item->SetVerticalAlignment(Alignment::END);
            item->SetParentHeightPercent(0.0f);
            item->SetVerticalOffsetPercent(0.0f);
            item->SetVerticalOffsetPixels(-offset);
            offset += item->GetBaseHeight() + _spacing;
        }
        else if (_direction == PanelDirection::RIGHT)
        {
            item->SetHorizontalAlignment(Alignment::START);
            item->SetParentWidthPercent(0.0f);
            item->SetHorizontalOffsetPercent(0.0f);
            item->SetHorizontalOffsetPixels(offset);
            offset += item->GetBaseWidth() + _spacing;
        }
        else if (_direction == PanelDirection::LEFT)
        {
            item->SetHorizontalAlignment(Alignment::END);
            item->SetParentWidthPercent(0.0f);
            item->SetHorizontalOffsetPercent(0.0f);
            item->SetHorizontalOffsetPixels(-offset);
            offset += item->GetBaseWidth() + _spacing;
        }
    }

    ResumeLayoutUpdates(false);

    Panel::_RecalculateLayout(width, height);
}