#include "Slider.h"
#include "App.h"
#include "Scenes/Scene.h"

void zcom::Slider::Init()
{
    Panel::Init();

    _bodyPlaceholder = Create<Dummy>();
    _bodyPlaceholder->visible = false;
    _anchorPlaceholder = Create<Dummy>();
    _anchorPlaceholder->visible = false;
    AddItem(_bodyPlaceholder.get());
    AddItem(_anchorPlaceholder.get());
    InvokeRedraw();
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

zcom::EventContext zcom::Slider::_OnMouseMove(Point point, Point deltaPos)
{
    if (point.x >= interactionAreaMargins->left && point.x < size_->width - interactionAreaMargins->right &&
        point.y >= interactionAreaMargins->top && point.y < size_->height - interactionAreaMargins->bottom)
    {
        insideInteractionArea_ = true;
    }
    else if (!holding_)
    {
        insideInteractionArea_ = false;
    }
    if (holding_)
        _HandleMouseMove(point.x);
    return Panel::_OnMouseMove(point, deltaPos);
}

void zcom::Slider::_OnMouseLeave()
{
    insideInteractionArea_ = false;
    Panel::_OnMouseLeave();
}

zcom::EventContext zcom::Slider::_OnLeftPressed(Point point)
{
    if (insideInteractionArea_)
    {
        holding_ = true;
        _HandleMouseMove(point.x);
    }
    return Panel::_OnLeftPressed(point);
}

zcom::EventContext zcom::Slider::_OnLeftReleased(std::optional<Point> point)
{
    if (holding_)
    {
        holding_ = false;
    }
    return Panel::_OnLeftReleased(point);
}

zcom::EventContext zcom::Slider::_OnWheelUp(Point point)
{
    // TODO: FIX (remove these overrides in favor of simply always eating the event)
    // Also rework post event handlers to allow modifying the EventContext object in handler

    // If event eating is set to true, assume that inner components don't need scroll events
    return (eatScrollEvents && value < 1.0f) ? EventContext().Add(this, point) : Panel::_OnWheelUp(point);
}

zcom::EventContext zcom::Slider::_OnWheelDown(Point point)
{
    return (eatScrollEvents && value > 0.0f) ? EventContext().Add(this, point) : Panel::_OnWheelDown(point);
}

void zcom::Slider::_OnResize(Size size)
{
    Panel::_OnResize(size);
    _PositionAnchor();
}

void zcom::Slider::_HandleMouseMove(int position)
{
    int sliderPosition = position - bodyStartOffset;
    int maxPosition = size_->width - bodyStartOffset - bodyEndOffset;
    if (sliderPosition < 0)
        sliderPosition = 0;
    if (sliderPosition > maxPosition - 1)
        sliderPosition = maxPosition - 1;

    float newValue = sliderPosition / float(maxPosition - 1);
    if (newValue != value)
        _onValueChanged->InvokeAll(this, &newValue);
    value = newValue;
}

void zcom::Slider::_PositionAnchor()
{
    int maxPosition = size_->width - bodyStartOffset - bodyEndOffset;
    int currentPosition = int(value * (maxPosition - 1));

    GetItem(1)->position = { bodyStartOffset + currentPosition + anchorOffset, GetItem(1)->position->y };
}
