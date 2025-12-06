#include "PPCounterOverlay.h"
#include "PPCounterSetupComponent.h"
#include "PPCounterOverlayComponent.h"

std::unique_ptr<zcom::Component> PPCounterOverlay::CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::PPCounterSetupComponent>(sharedThis);
}

std::unique_ptr<zcom::Component> PPCounterOverlay::CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::PPCounterOverlayComponent>(sharedThis);
}
