#pragma once

#include "Overlays/Overlay.h"

class EnhancedSmokeOverlay : public Overlay
{
public:
    std::unique_ptr<zcom::Component> CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const override;
    std::unique_ptr<zcom::Component> CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const override;
    std::wstring GetTitle() const override { return L"Enhanced smoke"; }
    bool RequiresGameData() const override { return false; }
    bool RequiresUncappedFramerate() const override { return false; }
    bool RequiresPrecisePointerData() const override { return true; }
};