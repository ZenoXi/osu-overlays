
#include "Window/WindowsEx.h"
#include <WinSock2.h>
#include <conio.h>
// Necessary for clsid's to work when creating custom effects
#include <Mmsystem.h>

#include "Scenes/DefaultNonClientAreaScene.h"
#include "Scenes/DefaultTitleBarScene.h"
#include "Scenes/EntryScene.h"

#include "Window/Window.h"
#include "App.h"

#include <iostream>
#include <vector>

int WINAPI main(HINSTANCE hInst, HINSTANCE, LPWSTR cmdLine, INT)
{
    // Enable networking
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cout << "WSAStartup failed\n";
        // TODO: Logging
    }

    App app(hInst);

    std::optional<zwnd::WindowId> id = app.CreateTopWindow(
        zwnd::WindowProperties()
            .WindowClassName(L"wndClass")
            .InitialSize(720, 720)
            .DisableFastTooltips()
            .MainWindow(),
        [](zwnd::Window* wnd)
        {
            wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
            wnd->resourceManager.InitAllImages();
            wnd->LoadNonClientAreaScene<zcom::DefaultNonClientAreaScene>(nullptr);
            zcom::DefaultTitleBarSceneOptions opt;
            opt.windowIconResourceName = "cursor_icon";
            opt.windowTitle = L"Overlay engine";
            opt.darkMode = true;
            wnd->LoadTitleBarScene<zcom::DefaultTitleBarScene>(&opt);
            wnd->LoadStartingScene<zcom::EntryScene>(nullptr);
        }
    );

    while (true)
    {
        if (app.WindowsClosed())
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
