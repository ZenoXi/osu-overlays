#include "Checkbox.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

EventSubscription<void, bool> zcom::Checkbox::SubscribeOnStateChanged(const std::function<void(bool)>& handler)
{
    return _onStateChanged->Subscribe(handler);
}

void zcom::Checkbox::Init(bool checked)
{
    this->checked = checked;
    checkColor = Color(0x999999);

    _customInactiveDraw = true;
    cursorIcon = zwnd::CursorIcon::HAND;
    selectable = true;
    border.visible = true;
    border.color = Color(0x4D4D4D);
    border.cornerRadius = 5.0f;
    backgroundColor = Color(0x1A1A1A);
}

void zcom::Checkbox::_OnDraw(Graphics* g)
{
    if (!checked)
        return;

    Color finalCheckColor = checkColor;
    if (disabled)
    {
        finalCheckColor.r = uint8_t(finalCheckColor.r * 0.5f);
        finalCheckColor.g = uint8_t(finalCheckColor.g * 0.5f);
        finalCheckColor.b = uint8_t(finalCheckColor.b * 0.5f);
    }

    if (border.cornerRadius > 5.0f)
    {
        RoundedRect rrect{};
        rrect.radiusX = border.cornerRadius - 5.0f;
        rrect.radiusY = border.cornerRadius - 5.0f;
        rrect.rect = { 5.0f, 5.0f, size_->width - 5.0f, size_->height - 5.0f };
        g->FillRoundedRectangle(rrect, finalCheckColor);
    }
    else
    {
        RectF rect = { 5.0f, 5.0f, size_->width - 5.0f, size_->height - 5.0f };
        g->FillRectangle(rect, finalCheckColor);
    }
}

zcom::EventContext zcom::Checkbox::_OnLeftPressed(Point point)
{
    checked = !checked;
    _onStateChanged->InvokeAll(checked);
    return EventContext().Add(this, point);
}

bool zcom::Checkbox::_OnKeyDown(BYTE vkCode)
{
    if (vkCode == VK_RETURN)
    {
        checked = !checked;
        _onStateChanged->InvokeAll(checked);
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