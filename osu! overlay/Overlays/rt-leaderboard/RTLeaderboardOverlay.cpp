#include "RTLeaderboardOverlay.h"
#include "RTLeaderboardSetupComponent.h"
#include "RTLeaderboardOverlayComponent.h"

std::unique_ptr<zcom::Component> RTLeaderboardOverlay::CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::RTLeaderboardSetupComponent>(sharedThis);
}

std::unique_ptr<zcom::Component> RTLeaderboardOverlay::CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const
{
    return parent->Create<zcom::RTLeaderboardOverlayComponent>(sharedThis);
}