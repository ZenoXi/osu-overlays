#include "Checkbox.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

void zcom::Checkbox::Checked(bool checked)
{
    if (_checked == checked)
    return;

    _checked = checked;
    InvokeRedraw();
}

void zcom::Checkbox::SetCheckColor(D2D1_COLOR_F checkColor)
{
    if (_checkColor == checkColor)
        return;

    _checkColor = checkColor;
    if (_checked)
        InvokeRedraw();
}

EventSubscription<void, bool> zcom::Checkbox::SubscribeOnStateChanged(const std::function<void(bool)>& handler)
{
    return _onStateChanged->Subscribe(handler);
}

void zcom::Checkbox::Init(bool checked)
{
    _checked = checked;
    _checkColor = D2D1::ColorF(0.6f, 0.6f, 0.6f);

    _customInactiveDraw = true;
    SetDefaultCursor(zwnd::CursorIcon::HAND);
    SetSelectable(true);
    SetCornerRounding(5.0f);
    SetBorderVisibility(true);
    SetBorderColor(D2D1::ColorF(0.3f, 0.3f, 0.3f));
    SetBackgroundColor(D2D1::ColorF(0.1f, 0.1f, 0.1f));
}

void zcom::Checkbox::_OnDraw(Graphics g)
{
    if (!Checked())
        return;

    D2D1_COLOR_F finalCheckColor = _checkColor;
    if (!GetActive())
    {
        finalCheckColor.r *= 0.5f;
        finalCheckColor.g *= 0.5f;
        finalCheckColor.b *= 0.5f;
    }

    if (GetCornerRounding() > 5.0f)
    {
        auto size = g.target->GetSize();
        D2D1_ROUNDED_RECT rrect;
        rrect.radiusX = GetCornerRounding() - 5.0f;
        rrect.radiusY = GetCornerRounding() - 5.0f;
        rrect.rect = { 5.0f, 5.0f, size.width - 5.0f, size.height - 5.0f };
        ID2D1SolidColorBrush* brush = nullptr;
        g.target->CreateSolidColorBrush(finalCheckColor, &brush);
        g.target->FillRoundedRectangle(rrect, brush);
        brush->Release();
    }
    else
    {
        auto size = g.target->GetSize();
        D2D1_RECT_F rect = { 5.0f, 5.0f, size.width - 5.0f, size.height - 5.0f };
        ID2D1SolidColorBrush* brush = nullptr;
        g.target->CreateSolidColorBrush(finalCheckColor, &brush);
        g.target->FillRectangle(rect, brush);
        brush->Release();
    }
}

zcom::EventTargets zcom::Checkbox::_OnLeftPressed(int x, int y)
{
    Checked(!Checked());
    _onStateChanged->InvokeAll(Checked());
    InvokeRedraw();
    return EventTargets().Add(this, x, y);
}

bool zcom::Checkbox::_OnKeyDown(BYTE vkCode)
{
    if (vkCode == VK_RETURN)
    {
        Checked(!Checked());
        _onStateChanged->InvokeAll(Checked());
        return true;
    }
    return false;
}

void zcom::Checkbox::_OnSelected(bool reverse)
{
    _scene->GetWindow()->keyboardManager.SetExclusiveHandler(this);
}

void zcom::Checkbox::_OnDeselected()
{
    _scene->GetWindow()->keyboardManager.ResetExclusiveHandler();
}