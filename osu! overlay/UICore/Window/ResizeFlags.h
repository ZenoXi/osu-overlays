#pragma once

namespace zwnd
{
    struct ResizeFlags
    {
        bool windowMaximized = false;
        bool windowMinimized = false;
        bool windowRestored = false;
        bool windowFullscreened = false;
    };
}