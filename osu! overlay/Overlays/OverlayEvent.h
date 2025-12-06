#pragma once

#include "Overlay.h"

struct OverlayEvent
{
    enum Type
    {
        OVERLAY_ENABLED,
        OVERLAY_DISABLED
    };

    Type type;
    std::shared_ptr<const Overlay> affectedOverlay;
};