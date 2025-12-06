#include "EnhancedSmokeOverlay.h"
#include "EnhancedSmokeSetupComponent.h"
#include "../SmokeSimOverlayComponent.h"
#include "../SmokeSimType.h"

std::unique_ptr<zcom::Component> EnhancedSmokeOverlay::CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::EnhancedSmokeSetupComponent>(sharedThis);
}

std::unique_ptr<zcom::Component> EnhancedSmokeOverlay::CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::SmokeSimOverlayComponent>(sharedThis, zcom::SmokeSimType::ENHANCED_SMOKE);
}