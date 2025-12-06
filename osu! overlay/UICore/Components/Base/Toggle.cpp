#include "Toggle.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

#include "Helper/AnimationHelper.h"

void zcom::Toggle::Init(bool toggledOn)
{
    this->toggledOn = toggledOn;

    _customInactiveDraw = true;
    cursorIcon = zwnd::CursorIcon::HAND;
    selectable = true;
    border.cornerRadius = 5.0f;
    border.visible = true;
    border.color = Color(0x4D4D4D);
    backgroundColor = Color(0x1A1A1A);
}

void zcom::Toggle::_OnUpdate()
{
    if (animating_)
    {
        animationProgress_ = (ztime::Main() - animationStartTime_).GetTicks() / (float)animationDuration->GetTicks();
        if (animationProgress_ >= 1.0f)
        {
            animating_ = false;
            if (toggledOn)
                backgroundColor = toggledOnBackgroundColor.Get();
            else
                backgroundColor = toggledOffBackgroundColor.Get();
        }
        else
        {
            if (toggledOn)
            {
                float x = zanim::EaseOutQuad(animationProgress_.Get());
                Color color = {};
                color.r = zanim::Interpolate(toggledOffBackgroundColor->r, toggledOnBackgroundColor->r, x);
                color.g = zanim::Interpolate(toggledOffBackgroundColor->g, toggledOnBackgroundColor->g, x);
                color.b = zanim::Interpolate(toggledOffBackgroundColor->b, toggledOnBackgroundColor->b, x);
                color.a = zanim::Interpolate(toggledOffBackgroundColor->a, toggledOnBackgroundColor->a, x);
                backgroundColor = color;
            }
            else
            {
                float x = zanim::EaseOutQuad(animationProgress_.Get());
                Color color = {};
                color.r = zanim::Interpolate(toggledOnBackgroundColor->r, toggledOffBackgroundColor->r, x);
                color.g = zanim::Interpolate(toggledOnBackgroundColor->g, toggledOffBackgroundColor->g, x);
                color.b = zanim::Interpolate(toggledOnBackgroundColor->b, toggledOffBackgroundColor->b, x);
                color.a = zanim::Interpolate(toggledOnBackgroundColor->a, toggledOffBackgroundColor->a, x);
                backgroundColor = color;
            }
        }

        InvokeRedraw();
    }
}

void zcom::Toggle::_OnDraw(Graphics* g)
{
    Color finalToggledOnColor = toggledOnAnchorColor;
    Color finalToggledOffColor = toggledOffAnchorColor;
    if (disabled)
    {
        finalToggledOnColor.r = uint8_t(finalToggledOnColor.r * 0.5f);
        finalToggledOnColor.g = uint8_t(finalToggledOnColor.g * 0.5f);
        finalToggledOnColor.b = uint8_t(finalToggledOnColor.b * 0.5f);
        finalToggledOffColor.r = uint8_t(finalToggledOffColor.r * 0.5f);
        finalToggledOffColor.g = uint8_t(finalToggledOffColor.g * 0.5f);
        finalToggledOffColor.b = uint8_t(finalToggledOffColor.b * 0.5f);
    }

    float innerRounding = border.cornerRadius - marginToBorder;
    if (innerRounding < 0.0f)
        innerRounding = 0.0f;

    float toggleSize = size_->height - 2 * marginToBorder;
    float xOffset = 0.0f;
    float maxOffset = size_->width - 2 * marginToBorder - toggleSize;
    if (toggledOn)
    {
        xOffset = maxOffset;
        if (animating_)
        {
            float x = zanim::EaseOutQuad(animationProgress_.Get());
            xOffset = maxOffset * x;
            finalToggledOnColor.r = zanim::Interpolate(finalToggledOffColor.r, finalToggledOnColor.r, x);
            finalToggledOnColor.g = zanim::Interpolate(finalToggledOffColor.g, finalToggledOnColor.g, x);
            finalToggledOnColor.b = zanim::Interpolate(finalToggledOffColor.b, finalToggledOnColor.b, x);
            finalToggledOnColor.a = zanim::Interpolate(finalToggledOffColor.a, finalToggledOnColor.a, x);
        }
    }
    else
    {
        xOffset = 0.0f;
        if (animating_)
        {
            float x = zanim::EaseOutQuad(animationProgress_.Get());
            xOffset = maxOffset * (1.0f - x);
            finalToggledOffColor.r = zanim::Interpolate(finalToggledOnColor.r, finalToggledOffColor.r, x);
            finalToggledOffColor.g = zanim::Interpolate(finalToggledOnColor.g, finalToggledOffColor.g, x);
            finalToggledOffColor.b = zanim::Interpolate(finalToggledOnColor.b, finalToggledOffColor.b, x);
            finalToggledOffColor.a = zanim::Interpolate(finalToggledOnColor.a, finalToggledOffColor.a, x);
        }
    }

    RoundedRect rrect{};
    rrect.radiusX = innerRounding;
    rrect.radiusY = innerRounding;
    rrect.rect = {
        marginToBorder + xOffset,
        marginToBorder,
        marginToBorder + toggleSize + xOffset,
        marginToBorder + toggleSize
    };
    g->FillRoundedRectangle(rrect, toggledOn ? finalToggledOnColor : finalToggledOffColor);
}

zcom::EventContext zcom::Toggle::_OnLeftPressed(Point point)
{
    bool newValue = !toggledOn;
    _onToggled->InvokeAll(&newValue);
    toggledOn = newValue;
    return EventContext().Add(this, point);
}

bool zcom::Toggle::_OnKeyDown(BYTE vkCode)
{
    if (vkCode == VK_RETURN)
    {
        bool newValue = !toggledOn;
        _onToggled->InvokeAll(&newValue);
        toggledOn = newValue;
        return true;
    }
    return false;
}

void zcom::Toggle::_OnSelected(bool reverse)
{
    _scene->GetWindow()->keyboardManager.SetExclusiveHandler(this);
}

void zcom::Toggle::_OnDeselected()
{
    _scene->GetWindow()->keyboardManager.ResetExclusiveHandler();
}