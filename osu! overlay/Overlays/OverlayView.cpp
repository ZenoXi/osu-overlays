#include "App.h"
#include "SharedContext.h"
#include "OverlayView.h"

OverlayView::OverlayView(uint64_t overlayId, zcom::Component* owner) : _overlayId(overlayId)
{
    _updateEventSubscription = owner->SubscribePostUpdate([=]() {

        // Create copy of overlay events since that should be very cheap compared to potentially updating lots of UI and hogging the mutex when values change
        std::vector<OverlayEvent> overlayUpdates;
        _overlayEventSubscription->HandlePendingEvents([&](const OverlayEvent& e) {
            overlayUpdates.push_back(e);
            });

        for (auto& update : overlayUpdates)
        {
            if (update.affectedOverlay->Id() != _overlayId)
                continue;

            if (update.type == OverlayEvent::OVERLAY_ENABLED)
                enabled_ = true;
            else if (update.type == OverlayEvent::OVERLAY_DISABLED)
                enabled_ = false;
        }
    });

    enabled_ = owner->GetScene()->GetApp()->Shared<SharedContext*>()->overlayManager.OverlayEnabled(overlayId);
    _overlayEventSubscription = owner->GetScene()->GetApp()->Shared<SharedContext*>()->overlayManager.SubscribeToOverlayEvents();
}
