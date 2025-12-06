#include "URCounterOverlay.h"
#include "URCounterSetupComponent.h"
#include "URCounterOverlayComponent.h"

std::unique_ptr<zcom::Component> URCounterOverlay::CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::URCounterSetupComponent>(sharedThis);
}

std::unique_ptr<zcom::Component> URCounterOverlay::CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::URCounterOverlayComponent>(sharedThis);
}
