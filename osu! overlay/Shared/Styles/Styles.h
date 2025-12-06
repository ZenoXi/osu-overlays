#pragma once

#include "Components/Base/ComponentBase.h"
#include "Components/Base/Button.h"
#include "Components/Base/Panel.h"

namespace zcom
{
    const Color SEPARATOR_COLOR = Color(0x404040);
    const Color ON_INDICATOR_COLOR = Color(0x30B020);
    const Color OFF_INDICATOR_COLOR = Color(0xB03020);


    struct RoundedLiftedButtonStyle
    {
        static void Apply(Button* button)
        {
            button->border.visible = false;
            button->border.cornerRadius = 3.0f;
            button->border.selectedColor = Color(0, 0.0f);
            button->SetProperty(Shadow().WithColor(Color(0, 0.3f)));
        }
    };

    struct NeutralButtonStyle
    {
        static void Apply(Button* button)
        {
            RoundedLiftedButtonStyle::Apply(button);
            button->backgroundColor.ComputedFrom([](bool disabled) { return disabled ? Color(0x484848) : Color(0x303030); }, button->disabled);
            button->ValueFromButtonState<Color>(button->buttonColor, Color(0, 0.0f), Color(0xFFFFFF, 0.1f), Color(0x000000, 0.1f));
        }
    };

    struct EnableButtonStyle
    {
        static void Apply(Button* button)
        {
            RoundedLiftedButtonStyle::Apply(button);
            button->Label()->fontColor = Color(0xEAEAEA);
            button->ValueFromButtonState<Color>(button->buttonColor, Color(0x307020), Color(0x309020), Color(0x308020));
        }
    };

    struct DisableButtonStyle
    {
        static void Apply(Button* button)
        {
            RoundedLiftedButtonStyle::Apply(button);
            button->Label()->fontColor = Color(0xEAEAEA);
            button->ValueFromButtonState<Color>(button->buttonColor, Color(0x703020), Color(0x903020), Color(0x803020));
        }
    };

    struct HorizontalSeparatorStyle
    {
        static void Apply(Component* item, int horizontalMargins = 0)
        {
            item->parentSize = { 1.0f, 0.0f };
            item->size = { horizontalMargins * -2, 1 };
            item->xAlign = Alignment::CENTER;
            item->backgroundColor = SEPARATOR_COLOR;
        }
    };

    struct VerticalSeparatorStyle
    {
        static void Apply(Component* item, int verticalMargins = 0)
        {
            item->parentSize = { 0.0f, 1.0f };
            item->size = { 1, verticalMargins * -2 };
            item->yAlign = Alignment::CENTER;
            item->backgroundColor = SEPARATOR_COLOR;
        }
    };
}