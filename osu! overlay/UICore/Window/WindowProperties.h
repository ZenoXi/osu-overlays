#pragma once

#include "WindowsEx.h"
#include "WindowDisplayType.h"
#include <string>
#include <optional>

namespace zwnd
{
    // Struct containing window creation properties
    struct WindowProperties
    {
        //
        // General properties

        // A unique window class name. For details refer to the Microsoft documentation
        std::wstring windowClassName;
        // An icon that will be used in the taskbar and executable file
        HICON windowIcon = NULL;
        // Initial width of the window
        int initialWidth = 1280;
        // Initial height of the window
        int initialHeight = 720;
        // Initial offset from the target monitor left coordinate
        std::optional<int> initialXOffset = std::nullopt;
        // Initial offset from the target monitor top coordinate
        std::optional<int> initialYOffset = std::nullopt;
        // Minimum/Maximum window sizes
        int minWidth = 300;
        int minHeight = 160;
        int maxWidth = 10000;
        int maxHeight = 10000;
        // Whether the initial width is final
        bool fixedWidth = false;
        // Whether the initial height is final
        bool fixedHeight = false;
        // Whether to consider the taskbar area of the monitor when calculating window placement
        bool ignoreTaskbarForPlacement = false;
        // How to display the window right after creation
        WindowDisplayType initialDisplay = WindowDisplayType::NORMAL;
        // Top-most windows are placed at the top of the z-order
        bool topMost = false;
        // Disable the ability to maximize the window
        bool disableMaximizing = false;
        // Disable the ability to minimize the window
        bool disableMinimizing = false;
        // Disable window animation when maximizing/minimizing/restoring
        bool disableWindowAnimations = false;
        // Disable ability for window to become activated
        bool disableWindowActivation = false;
        // Disable ability to interact with the window with a mouse
        bool disableMouseInteraction = false;
        // Creating a window by default creates a hidden window for tooltips, which doesn't need to be created every time a tooltip is shown, increasing speed
        // This behavior can be disabled to reduce init time for windows which don't need this optimization
        bool disableFastTooltips = false;

        //
        // Top level window properties

        // Indicates whether the window is a "main" window
        bool mainWindow = false;

        //
        // Child window properties

        // Indicates whether the window blocks interaction with the parent window
        bool blockParent = false;

        
        WindowProperties& WindowClassName(std::wstring name) { windowClassName = name; return *this; }
        WindowProperties& WindowIcon(HICON icon) { windowIcon = icon; return *this; }
        WindowProperties& InitialWidth(int width) { initialWidth = width; return *this; }
        WindowProperties& InitialHeight(int height) { initialHeight = height; return *this; }
        WindowProperties& InitialSize(int width, int height) { return InitialWidth(width).InitialHeight(height); }
        WindowProperties& InitialXOffset(int offset) { initialXOffset = offset; return *this; }
        WindowProperties& InitialYOffset(int offset) { initialYOffset = offset; return *this; }
        WindowProperties& InitialOffset(int xOffset, int yOffset) { return InitialXOffset(xOffset).InitialYOffset(yOffset); }
        WindowProperties& MinWidth(int minWidth) { this->minWidth = minWidth; return *this; }
        WindowProperties& MinHeight(int minHeight) { this->minHeight = minHeight; return *this; }
        WindowProperties& MinSize(int minWidth, int minHeight) { return MinWidth(minWidth).MinHeight(minHeight); }
        WindowProperties& MaxWidth(int maxWidth) { this->maxWidth = maxWidth; return *this; }
        WindowProperties& MaxHeight(int maxHeight) { this->maxHeight = maxHeight; return *this; }
        WindowProperties& MaxSize(int maxWidth, int maxHeight) { return MaxWidth(maxWidth).MaxHeight(maxHeight); }
        WindowProperties& FixedWidth() { this->fixedWidth = true; return *this; }
        WindowProperties& FixedHeight() { this->fixedHeight = true; return *this; }
        WindowProperties& FixedSize() { return FixedWidth().FixedHeight(); }
        WindowProperties& IgnoreTaskbarForPlacement() { ignoreTaskbarForPlacement = true; return *this; }
        WindowProperties& InitialDisplay(WindowDisplayType initialDisplay) { this->initialDisplay = initialDisplay; return *this; }
        WindowProperties& TopMost() { topMost = true; return *this; }
        WindowProperties& DisableMaximizing() { disableMaximizing = true; return *this; }
        WindowProperties& DisableMinimizing() { disableMinimizing = true; return *this; }
        WindowProperties& DisableWindowAnimations() { disableWindowAnimations = true; return *this; }
        WindowProperties& DisableWindowActivation() { disableWindowActivation = true; return *this; }
        WindowProperties& DisableMouseInteraction() { disableMouseInteraction = true; return *this; }
        WindowProperties& DisableFastTooltips() { disableFastTooltips = true; return *this; }
        WindowProperties& MainWindow() { mainWindow = true; return *this; }
        WindowProperties& BlockParent() { blockParent = true; return *this; }
    };

}