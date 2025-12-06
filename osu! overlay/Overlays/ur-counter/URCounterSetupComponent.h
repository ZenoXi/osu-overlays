#pragma once

#include "UICore/Components/Base/ScrollPanel.h"

#include "Overlays/Overlay.h"
#include "Overlays/OverlayView.h"

namespace zcom
{
    class URCounterSetupComponent : public ScrollPanel
    {
        DEFINE_COMPONENT(URCounterSetupComponent, ScrollPanel)
        DEFAULT_DESTRUCTOR(URCounterSetupComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        std::shared_ptr<const Overlay> _overlay;
        std::optional<OverlayView> _overlayView;
    };
}