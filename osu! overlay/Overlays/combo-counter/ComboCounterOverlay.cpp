#include "ComboCounterOverlay.h"
#include "ComboCounterSetupComponent.h"
#include "ComboCounterOverlayComponent.h"

std::unique_ptr<zcom::Component> ComboCounterOverlay::CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::ComboCounterSetupComponent>(sharedThis);
}

std::unique_ptr<zcom::Component> ComboCounterOverlay::CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::ComboCounterOverlayComponent>(sharedThis);
}
