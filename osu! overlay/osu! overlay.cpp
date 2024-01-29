
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
    // Set working directory
    std::wstring dir;
    dir.resize(MAX_PATH);
    GetModuleFileName(NULL, dir.data(), MAX_PATH);
    auto pos = dir.find_last_of(L"\\/");
    std::wstring runDir = dir.substr(0, pos);
    std::wcout << "Executable path: " << dir << '\n';

    if (!SetCurrentDirectory(runDir.data()))
    {
        std::cout << "Directory set failed\n";
        return -1;
    }

    TCHAR path[MAX_PATH] = { 0 };
    DWORD a = GetCurrentDirectory(MAX_PATH, path);
    std::wcout << "New working directory: " << path << '\n';

    // Read arguments
    std::vector<std::wstring> args;
    int argCount;
    LPWSTR* pArgs = CommandLineToArgvW(cmdLine, &argCount);
    for (int i = 0; i < argCount; i++)
    {
        args.push_back(std::wstring(pArgs[i]));
        std::wcout << args[i] << '\n';
    }
    LocalFree(pArgs);

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
