#pragma once

#include "UICore/Components/Base/ScrollPanel.h"

#include "Overlays/Overlay.h"
#include "Overlays/OverlayView.h"

namespace zcom
{
    class ComboCounterSetupComponent : public ScrollPanel
    {
        DEFINE_COMPONENT(ComboCounterSetupComponent, ScrollPanel)
        DEFAULT_DESTRUCTOR(ComboCounterSetupComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        std::shared_ptr<const Overlay> _overlay;
        std::optional<OverlayView> _overlayView;
    };
}