#pragma once

#include "UICore/Components/Base/ScrollPanel.h"

#include "Overlays/Overlay.h"
#include "Overlays/OverlayView.h"

namespace zcom
{
    class RTLeaderboardSetupComponent : public ScrollPanel
    {
        DEFINE_COMPONENT(RTLeaderboardSetupComponent, ScrollPanel)
        DEFAULT_DESTRUCTOR(RTLeaderboardSetupComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        std::shared_ptr<const Overlay> _overlay;
        std::optional<OverlayView> _overlayView;

        Value<bool> _leaderboardOptionsChanged = false;
    };
}
