#pragma once

#include "OverlayEvent.h"
#include "UICore/Helper/Value.h"
#include "Components/Base/ComponentBase.h"

// The purpose of this class is to keep track whether an overlay with a specific id is enabled or not.
// Value<T> objects for the overlay state are provided, which get updated on owner component post-update event
class OverlayView
{
public:
    OverlayView(uint64_t overlayId, zcom::Component* owner);
    Value<bool> enabled_ = false;
private:
    const uint64_t _overlayId;
    EventSubscription<void> _updateEventSubscription;
    std::unique_ptr<AsyncEventSubscription<void, OverlayEvent>> _overlayEventSubscription;
};