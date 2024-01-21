#pragma once

#include <string>
#include <optional>

namespace zcom
{
    struct Rect
    {
        int left;
        int top;
        int right;
        int bottom;
    };

    struct TooltipParams
    {
        std::wstring text = L"";
        int xPos = 0;
        int yPos = 0;
        int maxWidth = 600;
        std::optional<Rect> mouseMovementBounds = std::nullopt;
        // Makes the tooltip non-transparent, allowing interaction (e.g. selecting the text)
        // *NOT YET IMPLEMENTED*
        bool interactable = false;

        // If a tooltip with the same display id is attempted to be shown while one is already visible, the attempt will be ignored
        // This is used primarily to prevent mouse movement within a component with hover text from repeatedly opening the tooltip at different positions
        std::optional<uint64_t> displayId = std::nullopt;
    };
}