#pragma once

#include "Window/WindowId.h"
#include "Window/WindowType.h"
#include "Window/WindowProperties.h"

#include <optional>

struct WindowEvent
{
    enum Type
    {
        CREATED,
        CLOSED
    };

    Type eventType;
    zwnd::WindowId windowId;
    std::optional<zwnd::WindowType> windowType;
    std::optional<zwnd::WindowProperties> windowProperties;
};