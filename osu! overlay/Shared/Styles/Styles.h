#pragma once

#include "Components/Base/ComponentBase.h"
#include "Components/Base/Button.h"
#include "Components/Base/Panel.h"

namespace zcom
{
    const D2D1_COLOR_F SEPARATOR_COLOR = D2D1::ColorF(0x404040);
    const D2D1_COLOR_F ON_INDICATOR_COLOR = D2D1::ColorF(0x30B020);
    const D2D1_COLOR_F OFF_INDICATOR_COLOR = D2D1::ColorF(0xB03020);


    struct RoundedLiftedButtonStyle
    {
        static void Apply(Button* button)
        {
            button->SetBorderVisibility(false);
            button->SetCornerRounding(3.0f);
            button->SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));
            button->SetProperty(PROP_Shadow{});
        }
    };

    struct NeutralButtonStyle
    {
        static void Apply(Button* button)
        {
            RoundedLiftedButtonStyle::Apply(button);
            button->SetBackgroundColor(D2D1::ColorF(0x303030));
            button->SetButtonColor(D2D1::ColorF(0, 0.0f));
            button->SetButtonHoverColor(D2D1::ColorF(0xFFFFFF, 0.1f));
            button->SetButtonClickColor(D2D1::ColorF(0x000000, 0.1f));
        }
    };

    struct EnableButtonStyle
    {
        static void Apply(Button* button)
        {
            RoundedLiftedButtonStyle::Apply(button);
            button->Label()->SetFontColor(D2D1::ColorF(0xEAEAEA));
            button->SetButtonColor(D2D1::ColorF(0x307020));
            button->SetButtonHoverColor(D2D1::ColorF(0x309020));
            button->SetButtonClickColor(D2D1::ColorF(0x308020));
        }
    };

    struct DisableButtonStyle
    {
        static void Apply(Button* button)
        {
            RoundedLiftedButtonStyle::Apply(button);
            button->Label()->SetFontColor(D2D1::ColorF(0xEAEAEA));
            button->SetButtonColor(D2D1::ColorF(0x703020));
            button->SetButtonHoverColor(D2D1::ColorF(0x903020));
            button->SetButtonClickColor(D2D1::ColorF(0x803020));
        }
    };

    struct HorizontalSeparatorStyle
    {
        static void Apply(Component* item, int horizontalMargins = 0)
        {
            item->SetParentWidthPercent(1.0f);
            item->SetBaseSize(horizontalMargins * -2, 1);
            item->SetHorizontalAlignment(Alignment::CENTER);
            item->SetBackgroundColor(SEPARATOR_COLOR);
        }
    };

    struct VerticalSeparatorStyle
    {
        static void Apply(Component* item, int verticalMargins = 0)
        {
            item->SetParentHeightPercent(1.0f);
            item->SetBaseSize(1, verticalMargins * -2);
            item->SetVerticalAlignment(Alignment::CENTER);
            item->SetBackgroundColor(SEPARATOR_COLOR);
        }
    };
}