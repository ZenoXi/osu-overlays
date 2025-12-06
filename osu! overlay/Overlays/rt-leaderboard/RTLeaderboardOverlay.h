#pragma once

#include "Overlays/Overlay.h"

class RTLeaderboardOverlay : public Overlay
{
public:
    std::unique_ptr<zcom::Component> CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const override;
    std::unique_ptr<zcom::Component> CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const override;
    std::wstring GetTitle() const override { return L"Real-time leaderboard"; }
    bool RequiresGameData() const override { return true; }
    bool RequiresUncappedFramerate() const override { return false; }
};