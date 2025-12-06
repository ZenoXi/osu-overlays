#include "CursorTrailOverlay.h"
#include "CursorTrailSetupComponent.h"
#include "CursorTrailOverlayComponent.h"

std::unique_ptr<zcom::Component> CursorTrailOverlay::CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::CursorTrailSetupComponent>(sharedThis);
}

std::unique_ptr<zcom::Component> CursorTrailOverlay::CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::CursorTrailOverlayComponent>(sharedThis);
}
