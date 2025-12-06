#pragma once

#include "UICore/Components/Base/ScrollPanel.h"

#include "Overlays/Overlay.h"
#include "Overlays/OverlayView.h"

namespace zcom
{
    class EnhancedSmokeSetupComponent : public ScrollPanel
    {
        DEFINE_COMPONENT(EnhancedSmokeSetupComponent, ScrollPanel)
        DEFAULT_DESTRUCTOR(EnhancedSmokeSetupComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        std::shared_ptr<const Overlay> _overlay;
        std::optional<OverlayView> _overlayView;
    };
}