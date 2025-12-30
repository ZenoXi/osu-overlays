
#include "Window/WindowsEx.h"
#include <WinSock2.h>
#include <conio.h>
// Necessary for clsid's to work when creating custom effects
#include <Mmsystem.h>

#include "Scenes/DefaultNonClientAreaScene.h"
#include "Shared/Scenes/TitleBarScene.h"
#include "Scenes/EntryScene.h"
#include "versioning/UpdateErrorScene.h"

#include "Window/Window.h"
#include "App.h"
#include "SharedContext.h"

#include <iostream>
#include <vector>

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR cmdLine, INT argc)
{
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

    // Find -update flag
    std::optional<DWORD> updateProcessId;
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == L"-update" && i + 1 < args.size())
        {
            std::wstring value = args[i + 1];
            try
            {
                updateProcessId = (DWORD)std::stoul(value);
                std::cout << updateProcessId.value() << '\n';
            }
            catch (...) {}
            break;
        }
    }

    // Find -update-finalize flag
    std::optional<DWORD> updateFinalizeProcessId;
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == L"-update-finalize" && i + 1 < args.size())
        {
            std::wstring value = args[i + 1];
            try
            {
                updateFinalizeProcessId = (DWORD)std::stoul(value);
                std::cout << updateFinalizeProcessId.value() << '\n';
            }
            catch (...) {}
            break;
        }
    }

    std::optional<zcom::UpdateErrorSceneOptions> errorSceneOpt;

    // On regular start, try to silently delete temporary update folder for cases where it failed to delete during update
    if (!updateProcessId && !updateFinalizeProcessId)
    {
        namespace fs = std::filesystem;
        try
        {
            fs::path mainPath = fs::current_path();
            std::cout << fs::remove_all(mainPath / ".updatetemp") << '\n';
        }
        catch (fs::filesystem_error e) { }
    }

    if (updateProcessId)
    {
        bool waitSuccessful = true;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, updateProcessId.value());
        if (hProcess)
        {
            while (true)
            {
                DWORD exitCode;
                BOOL result = GetExitCodeProcess(hProcess, &exitCode);
                if (!result)
                {
                    errorSceneOpt = zcom::UpdateErrorSceneOptions{};
                    errorSceneOpt->errorText = std::wstring(L"The following error occured while waiting for the old version to exit:\n\n")
                        + L"[" + std::to_wstring(GetLastError()) + L"] " + ToWinErrorString(GetLastError()).value_or(L"Unspecified error")
                        + L"\n\nRestart the application and try updating again. If errors keep occuring, you can download the latest version manually from GitHub";
                    errorSceneOpt->showClose = false;
                    errorSceneOpt->clearBackground = false;
                    waitSuccessful = false;
                    break;
                }

                if (exitCode == STILL_ACTIVE)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                else
                {
                    break;
                }
            }
            CloseHandle(hProcess);
        }

        if (waitSuccessful)
        {
            namespace fs = std::filesystem;

            fs::path tempPath = fs::current_path();
            fs::path mainPath = tempPath.parent_path();

            int attemptCount = 1;
            int maxAttempts = 5;
            bool deleteFailed = false;
            do
            {
                if (attemptCount > maxAttempts)
                {
                    errorSceneOpt = zcom::UpdateErrorSceneOptions{};
                    errorSceneOpt->errorText = L"Failed to fully remove current version application files\n\nTry launching the application again. If errors occur, you need to redownload the application (user settings can be preserved by copying the 'config' file to the new download)";
                    errorSceneOpt->showExit = false;
                    deleteFailed = true;
                    break;
                }

                try
                {
                    std::cout << fs::remove_all(mainPath / "bin") << '\n';
                    std::cout << fs::remove_all(mainPath / "Resources") << '\n';
                    std::cout << fs::remove(mainPath / "CudaSmokeSim.dll") << '\n';
                    std::cout << fs::remove(mainPath / "CursorTrailEffect.cso") << '\n';
                    std::cout << fs::remove(mainPath / "TintEffect.cso") << '\n';
                    std::cout << fs::remove(mainPath / "OverlayEngine.exe") << '\n';
                }
                catch (fs::filesystem_error e)
                {
                    std::cout << e.what() << '\n';
                }

                attemptCount++;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            while (
                fs::exists(mainPath / "bin") ||
                fs::exists(mainPath / "Resources") ||
                fs::exists(mainPath / "CudaSmokeSim.dll") ||
                fs::exists(mainPath / "CursorTrailEffect.cso") ||
                fs::exists(mainPath / "TintEffect.cso") ||
                fs::exists(mainPath / "OverlayEngine.exe")
            );

            if (!deleteFailed)
            {
                try
                {
                    fs::copy(tempPath / "bin", mainPath / "bin", fs::copy_options::recursive);
                    fs::copy(tempPath / "Resources", mainPath / "Resources", fs::copy_options::recursive);
                    fs::copy(tempPath / "CudaSmokeSim.dll", mainPath / "CudaSmokeSim.dll");
                    fs::copy(tempPath / "CursorTrailEffect.cso", mainPath / "CursorTrailEffect.cso");
                    fs::copy(tempPath / "TintEffect.cso", mainPath / "TintEffect.cso");
                    fs::copy(tempPath / "OverlayEngine.exe", mainPath / "OverlayEngine.exe");

                    STARTUPINFO info = { sizeof(info) };
                    PROCESS_INFORMATION processInfo;
                    fs::path exePath = mainPath / "OverlayEngine.exe";
                    std::wstring args = L"\"" + exePath.wstring() + L"\" -update-finalize " + std::to_wstring(GetCurrentProcessId());
                    std::wstring exePathStr = exePath.wstring();
                    std::wstring mainPathStr = mainPath.wstring();
                    if (CreateProcess(exePathStr.c_str(), args.data(), NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, mainPathStr.c_str(), &info, &processInfo))
                    {
                        CloseHandle(processInfo.hProcess);
                        CloseHandle(processInfo.hThread);
                        CloseHandle(hProcess);
                        return 0;
                    }
                    else
                    {
                        errorSceneOpt = zcom::UpdateErrorSceneOptions{};
                        errorSceneOpt->errorText = std::wstring(L"Failed to launch the updated version:\n\n")
                            + L"[" + std::to_wstring(GetLastError()) + L"] " + ToWinErrorString(GetLastError()).value_or(L"Unspecified error")
                            + L"\n\nTry launching the application again. If errors occur, you need to redownload the application (user settings can be preserved by copying the 'config' file to the new download)";
                        errorSceneOpt->showClose = false;
                        errorSceneOpt->clearBackground = false;
                    }
                }
                catch (fs::filesystem_error e)
                {
                    errorSceneOpt = zcom::UpdateErrorSceneOptions{};
                    errorSceneOpt->errorText = L"The following error occured while updating:\n\n"
                        + string_to_wstring(std::string(e.what()))
                        + L"\n\nTry launching the application again. If errors occur, you need to redownload the application (user settings can be preserved by copying the 'config' file to the new download)";
                    errorSceneOpt->showClose = false;
                    errorSceneOpt->clearBackground = false;
                }
            }
        }
    }

    if (updateFinalizeProcessId)
    {
        bool waitSuccessful = true;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, updateFinalizeProcessId.value());
        if (hProcess)
        {
            while (true)
            {
                DWORD exitCode;
                BOOL result = GetExitCodeProcess(hProcess, &exitCode);
                if (!result)
                {
                    errorSceneOpt = zcom::UpdateErrorSceneOptions{};
                    errorSceneOpt->errorText = std::wstring(L"The following error occured while waiting for the updater to exit:\n\n")
                        + L"[" + std::to_wstring(GetLastError()) + L"] " + ToWinErrorString(GetLastError()).value_or(L"Unspecified error")
                        + L"\n\nYou can use the application normally, but consider deleting the folder manually to avoid issues with future updates";
                    errorSceneOpt->showExit = false;
                    waitSuccessful = false;
                    break;
                }

                if (exitCode == STILL_ACTIVE)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                else
                {
                    break;
                }
            }
            CloseHandle(hProcess);
        }

        if (waitSuccessful)
        {
            namespace fs = std::filesystem;

            fs::path mainPath = fs::current_path();

            int attemptCount = 1;
            int maxAttempts = 5;
            do
            {
                if (attemptCount > maxAttempts)
                {
                    errorSceneOpt = zcom::UpdateErrorSceneOptions{};
                    errorSceneOpt->errorText = L"Failed to remove temporary folder '.update-temp'\n\nYou can use the application normally, but consider deleting the folder manually to avoid issues with future updates";
                    errorSceneOpt->showExit = false;
                    break;
                }

                try
                {
                    std::cout << "temp files removed: " << fs::remove_all(mainPath / ".updatetemp") << '\n';
                }
                catch (fs::filesystem_error e)
                {
                    std::cout << e.what() << '\n';
                }

                attemptCount++;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            while (fs::exists(mainPath / ".updatetemp"));
        }
    }

    // Enable networking
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cout << "WSAStartup failed\n";
        // TODO: Logging
    }

    App app(hInst);
    auto versionStringOpt = app.config.GetValue(L"version");
    if (versionStringOpt)
    {
        auto tagOpt = VersionTag::Parse(wstring_to_string(versionStringOpt.value()));
        if (tagOpt)
        {
            // Do version specific processing
        }
    }
    app.config.SetValue(L"version", string_to_wstring(OVERLAY_ENGINE_VERSION.ToString()), true);

    SharedContext shared;
    shared.overlayManager.Init(&app);
    shared.settingsWindow.Init(&app);
    shared.webApi.SetUrl(app.config.GetConfigValue(webapi::WebApiConfig::API_URL));
    app.SetSharedContext(&shared);
    
    std::optional<zwnd::WindowId> id = app.CreateTopWindow(
        zwnd::WindowProperties()
            .WindowClassName(L"mainWindow")
            .InitialSize(720, 720)
            .MinSize(600, 400)
            .MainWindow(),
        [errorSceneOpt](zwnd::Window* wnd)
        {
            wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
            wnd->resourceManager.InitAllImages();
            wnd->LoadNonClientAreaScene<zcom::DefaultNonClientAreaScene>(nullptr);
            zcom::TitleBarSceneOptions opt;
            opt.windowIconResourceName = "cursor_icon";
            opt.windowTitle = L"Overlay engine";
            opt.darkMode = true;
            if (errorSceneOpt && !errorSceneOpt->showClose)
                opt.showUpdate = false;
            wnd->LoadTitleBarScene<zcom::TitleBarScene>(&opt);
            wnd->LoadStartingScene<zcom::EntryScene>(nullptr);
            if (errorSceneOpt)
            {
                zcom::UpdateErrorSceneOptions errOpt = errorSceneOpt.value();
                wnd->LoadStartingScene<zcom::UpdateErrorScene>(&errOpt);
                wnd->MoveSceneToFront<zcom::UpdateErrorScene>();
            }
        }
    );

    while (true)
    {
        if (app.WindowsClosed())
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Stopping app..\n";

    return 0;
}
