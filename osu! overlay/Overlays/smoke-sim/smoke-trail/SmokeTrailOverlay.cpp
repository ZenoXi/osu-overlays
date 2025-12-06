#include "SmokeTrailOverlay.h"
#include "SmokeTrailSetupComponent.h"
#include "../SmokeSimOverlayComponent.h"
#include "../SmokeSimType.h"

std::unique_ptr<zcom::Component> SmokeTrailOverlay::CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::SmokeTrailSetupComponent>(sharedThis);
}

std::unique_ptr<zcom::Component> SmokeTrailOverlay::CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::SmokeSimOverlayComponent>(sharedThis, zcom::SmokeSimType::CURSOR_TRAIL);
}