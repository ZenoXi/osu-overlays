#include "Toggle.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

#include "Helper/AnimationHelper.h"

void zcom::Toggle::SetToggledOn(bool toggledOn, bool emitChangeEvent)
{
    if (_isToggledOn == toggledOn)
        return;


    if (emitChangeEvent)
    {
        // Toggle can be overriden to introduce custom effects like delays or just disable toggling
        _onToggled->InvokeAll(&toggledOn);
        if (_isToggledOn == toggledOn)
            return;
    }

    _isToggledOn = toggledOn;
    _animating = true;
    _animationStartTime = ztime::Main();
    InvokeRedraw();
}

void zcom::Toggle::SetToggledOnAnchorColor(D2D1_COLOR_F color)
{
    if (_toggledOnAnchorColor == color)
        return;

    _toggledOnAnchorColor = color;
    if (IsToggledOn())
        InvokeRedraw();
}

void zcom::Toggle::SetToggledOffAnchorColor(D2D1_COLOR_F color)
{
    if (_toggledOffAnchorColor == color)
        return;

    _toggledOffAnchorColor = color;
    if (!IsToggledOn())
        InvokeRedraw();
}

void zcom::Toggle::SetToggledOnBackgroundColor(D2D1_COLOR_F color)
{
    if (_toggledOnBackgroundColor == color)
        return;

    _toggledOnBackgroundColor = color;
    if (IsToggledOn())
        SetBackgroundColor(color);
}

void zcom::Toggle::SetToggledOffBackgroundColor(D2D1_COLOR_F color)
{
    if (_toggledOffBackgroundColor == color)
        return;

    _toggledOffBackgroundColor = color;
    if (!IsToggledOn())
        SetBackgroundColor(color);
}

void zcom::Toggle::SetMarginToBorder(float margin)
{
    if (_marginToBorder == margin)
        return;

    _marginToBorder = margin;
    InvokeRedraw();
}

void zcom::Toggle::Init(bool toggledOn)
{
    _isToggledOn = toggledOn;

    _customInactiveDraw = true;
    SetDefaultCursor(zwnd::CursorIcon::HAND);
    SetSelectable(true);
    SetCornerRounding(5.0f);
    SetBorderVisibility(true);
    SetBorderColor(D2D1::ColorF(0.3f, 0.3f, 0.3f));
    SetBackgroundColor(D2D1::ColorF(0.1f, 0.1f, 0.1f));
}

void zcom::Toggle::_OnUpdate()
{
    if (_animating)
    {
        _animationProgress = (ztime::Main() - _animationStartTime).GetTicks() / (float)_animationDuration.GetTicks();
        if (_animationProgress >= 1.0f)
        {
            _animating = false;
            if (IsToggledOn())
                SetBackgroundColor(_toggledOnBackgroundColor);
            else
                SetBackgroundColor(_toggledOffBackgroundColor);
        }
        else
        {
            if (IsToggledOn())
            {
                float x = zanim::EaseOutQuad(_animationProgress);
                D2D1_COLOR_F color = {};
                color.r = zanim::Interpolate(_toggledOffBackgroundColor.r, _toggledOnBackgroundColor.r, x);
                color.g = zanim::Interpolate(_toggledOffBackgroundColor.g, _toggledOnBackgroundColor.g, x);
                color.b = zanim::Interpolate(_toggledOffBackgroundColor.b, _toggledOnBackgroundColor.b, x);
                color.a = zanim::Interpolate(_toggledOffBackgroundColor.a, _toggledOnBackgroundColor.a, x);
                SetBackgroundColor(color);
            }
            else
            {
                float x = zanim::EaseOutQuad(_animationProgress);
                D2D1_COLOR_F color = {};
                color.r = zanim::Interpolate(_toggledOnBackgroundColor.r, _toggledOffBackgroundColor.r, x);
                color.g = zanim::Interpolate(_toggledOnBackgroundColor.g, _toggledOffBackgroundColor.g, x);
                color.b = zanim::Interpolate(_toggledOnBackgroundColor.b, _toggledOffBackgroundColor.b, x);
                color.a = zanim::Interpolate(_toggledOnBackgroundColor.a, _toggledOffBackgroundColor.a, x);
                SetBackgroundColor(color);
            }
        }

        InvokeRedraw();
    }
}

void zcom::Toggle::_OnDraw(Graphics g)
{
    D2D1_COLOR_F finalToggledOnColor = _toggledOnAnchorColor;
    D2D1_COLOR_F finalToggledOffColor = _toggledOffAnchorColor;
    if (!GetActive())
    {
        finalToggledOnColor.r *= 0.5f;
        finalToggledOnColor.g *= 0.5f;
        finalToggledOnColor.b *= 0.5f;
        finalToggledOffColor.r *= 0.5f;
        finalToggledOffColor.g *= 0.5f;
        finalToggledOffColor.b *= 0.5f;
    }

    float innerRounding = GetCornerRounding() - _marginToBorder;
    if (innerRounding < 0.0f)
        innerRounding = 0.0f;

    auto size = g.target->GetSize();
    float toggleSize = size.height - 2 * _marginToBorder;
    float xOffset = 0.0f;
    float maxOffset = size.width - 2 * _marginToBorder - toggleSize;
    if (IsToggledOn())
    {
        xOffset = maxOffset;
        if (_animating)
        {
            float x = zanim::EaseOutQuad(_animationProgress);
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
        if (_animating)
        {
            float x = zanim::EaseOutQuad(_animationProgress);
            xOffset = maxOffset * (1.0f - x);
            finalToggledOffColor.r = zanim::Interpolate(finalToggledOnColor.r, finalToggledOffColor.r, x);
            finalToggledOffColor.g = zanim::Interpolate(finalToggledOnColor.g, finalToggledOffColor.g, x);
            finalToggledOffColor.b = zanim::Interpolate(finalToggledOnColor.b, finalToggledOffColor.b, x);
            finalToggledOffColor.a = zanim::Interpolate(finalToggledOnColor.a, finalToggledOffColor.a, x);
        }
    }

    D2D1_ROUNDED_RECT rrect = {};
    rrect.radiusX = innerRounding;
    rrect.radiusY = innerRounding;
    rrect.rect = {
        _marginToBorder + xOffset,
        _marginToBorder,
        _marginToBorder + toggleSize + xOffset,
        _marginToBorder + toggleSize,
    };
    ID2D1SolidColorBrush* brush = nullptr;
    g.target->CreateSolidColorBrush(IsToggledOn() ? finalToggledOnColor : finalToggledOffColor, &brush);
    if (brush)
    {
        g.target->FillRoundedRectangle(rrect, brush);
        brush->Release();
    }
    else
    {
        // TODO: Logging
    }
}

zcom::EventTargets zcom::Toggle::_OnLeftPressed(int x, int y)
{
    SetToggledOn(!IsToggledOn());
    return EventTargets().Add(this, x, y);
}

bool zcom::Toggle::_OnKeyDown(BYTE vkCode)
{
    if (vkCode == VK_RETURN)
    {
        SetToggledOn(!IsToggledOn());
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