
#include "Window/WindowsEx.h"
//#include <WinSock2.h>
//#include <conio.h>
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
