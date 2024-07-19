#include "Slider.h"
#include "App.h"
#include "Scenes/Scene.h"

void zcom::Slider::Init()
{
    Panel::Init();

    _bodyPlaceholder = Create<Dummy>();
    _bodyPlaceholder->SetVisible(false);
    _anchorPlaceholder = Create<Dummy>();
    _anchorPlaceholder->SetVisible(false);
    AddItem(_bodyPlaceholder.get());
    AddItem(_anchorPlaceholder.get());
}

void zcom::Slider::SetBodyComponent(Component* body)
{
    RemoveItem(0);
    InsertItem(body ? body : _bodyPlaceholder.get(), 0);
}

void zcom::Slider::SetBodyComponent(std::unique_ptr<Component> body)
{
    RemoveItem(0);
    if (body)
        InsertItem(std::move(body), 0);
    else
        InsertItem(_bodyPlaceholder.get(), 0);
}

void zcom::Slider::SetAnchorComponent(Component* anchor)
{
    RemoveItem(1);
    InsertItem(anchor ? anchor : _anchorPlaceholder.get(), 1);
}

void zcom::Slider::SetAnchorComponent(std::unique_ptr<Component> anchor)
{
    RemoveItem(1);
    if (anchor)
        InsertItem(std::move(anchor), 1);
    else
        InsertItem(_anchorPlaceholder.get(), 1);
    _PositionAnchor();
}

void zcom::Slider::SetSliderBodyStartOffset(int offset)
{
    SetSliderBodyOffset(offset, _endOffset);
}

void zcom::Slider::SetSliderBodyEndOffset(int offset)
{
    SetSliderBodyOffset(_startOffset, offset);
}

void zcom::Slider::SetSliderBodyOffset(int start, int end)
{
    _startOffset = start;
    _endOffset = end;
    _PositionAnchor();
}

void zcom::Slider::SetAnchorOffset(int offset)
{
    _anchorOffset = offset;
    _PositionAnchor();
}

void zcom::Slider::SetInteractionAreaMargins(RECT margins)
{
    _interactionAreaMargins = margins;
}

void zcom::Slider::SetValue(float value, bool emitChangeEvent)
{
    if (_currentValue == value)
        return;

    if (value < 0.0f)
        value = 0.0f;
    if (value > 1.0f)
        value = 1.0f;

    if (emitChangeEvent)
        _onValueChanged->InvokeAll(this, &value);
    _currentValue = value;
    _PositionAnchor();
}

zcom::EventTargets zcom::Slider::_OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    if (x >= _interactionAreaMargins.left && x < GetWidth() - _interactionAreaMargins.right &&
        y >= _interactionAreaMargins.top && y < GetHeight() - _interactionAreaMargins.bottom)
    {
        _SetInsideInteractionArea(true);
    }
    else if (!_holding)
    {
        _SetInsideInteractionArea(false);
    }
    if (_holding)
        _HandleMouseMove(x);
    return Panel::_OnMouseMove(x, y, deltaX, deltaY);
}

void zcom::Slider::_OnMouseLeave()
{
    _SetInsideInteractionArea(false);
    Panel::_OnMouseLeave();
}

zcom::EventTargets zcom::Slider::_OnLeftPressed(int x, int y)
{
    if (_insideInteractionArea)
    {
        _holding = true;
        _onSliderPressed->InvokeAll(this);
        _HandleMouseMove(x);
    }
    return Panel::_OnLeftPressed(x, y);
}

zcom::EventTargets zcom::Slider::_OnLeftReleased(int x, int y)
{
    if (_holding)
    {
        _holding = false;
        _onSliderReleased->InvokeAll(this);
    }
    return Panel::_OnLeftReleased(x, y);
}

zcom::EventTargets zcom::Slider::_OnWheelUp(int x, int y)
{
    // TODO: FIX (remove these overrides in favor of simply always eating the event)
    // Also rework post event handlers to allow modifying the EventTargets object in handler

    // If event eating is set to true, assume that inner components don't need scroll events
    return (GetEatScrollEvents() && _currentValue < 1.0f) ? EventTargets().Add(this, x, y) : Panel::_OnWheelUp(x, y);
}

zcom::EventTargets zcom::Slider::_OnWheelDown(int x, int y)
{
    return (GetEatScrollEvents() && _currentValue > 0.0f) ? EventTargets().Add(this, x, y) : Panel::_OnWheelDown(x, y);
}

void zcom::Slider::_OnResize(int width, int height)
{
    _PositionAnchor();
}

void zcom::Slider::_HandleMouseMove(int position)
{
    int sliderPosition = position - _startOffset;
    int maxPosition = GetWidth() - _startOffset - _endOffset;
    if (sliderPosition < 0)
        sliderPosition = 0;
    if (sliderPosition > maxPosition - 1)
        sliderPosition = maxPosition - 1;

    SetValue(sliderPosition / float(maxPosition - 1));
}

void zcom::Slider::_PositionAnchor()
{
    int maxPosition = GetWidth() - _startOffset - _endOffset;
    int currentPosition = int(_currentValue * (maxPosition - 1));

    GetItem(1)->SetHorizontalOffsetPixels(_startOffset + currentPosition + _anchorOffset);
}

void zcom::Slider::_SetInsideInteractionArea(bool value)
{
    if (value == _insideInteractionArea)
        return;

    _insideInteractionArea = value;
    if (_insideInteractionArea)
        _onEnterInteractionArea->InvokeAll(this);
    else
        _onLeaveInteractionArea->InvokeAll(this);
}