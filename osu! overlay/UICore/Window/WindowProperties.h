#pragma once

#include "WindowsEx.h"
#include "WindowDisplayType.h"
#include <string>

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
        // How to display the window right after creation
        WindowDisplayType initialDisplay = WindowDisplayType::NORMAL;
        // Top-most windows are placed at the top of the z-order
        bool topMost = false;
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
        WindowProperties& InitialSize(int width, int height) { initialWidth = width; initialHeight = height; return *this; }
        WindowProperties& InitialDisplay(WindowDisplayType initialDisplay) { this->initialDisplay = initialDisplay; return *this; }
        WindowProperties& TopMost() { topMost = true; return *this; }
        WindowProperties& DisableWindowAnimations() { disableWindowAnimations = true; return *this; }
        WindowProperties& DisableWindowActivation() { disableWindowActivation = true; return *this; }
        WindowProperties& DisableMouseInteraction() { disableMouseInteraction = true; return *this; }
        WindowProperties& DisableFastTooltips() { disableFastTooltips = true; return *this; }
        WindowProperties& MainWindow() { mainWindow = true; return *this; }
        WindowProperties& BlockParent() { blockParent = true; return *this; }
    };

}