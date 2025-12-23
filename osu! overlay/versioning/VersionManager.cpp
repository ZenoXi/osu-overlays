#include "VersionManager.h"

#include "Shared/Util/Http.h"
#include "Shared/Util/JsonParser.h"
#include "Shared/Util/ProcessRunner.h"
#include "Shared/Util/base64.h"
#include "UICore/Helper/Time.h"

#include <filesystem>
#include <fstream>

size_t GetHostLength(const std::string& url)
{
    int slashesFound = 0;
    for (size_t i = 0; i < url.length(); i++)
    {
        if (url[i] == '/')
        {
            slashesFound++;
            if (slashesFound == 3)
                return i;
        }
    }
    return url.length();
}

VersionManager::~VersionManager()
{
    _updateCheckEventSubscription.reset();
}

std::unique_ptr<AsyncEventSubscription<void, std::vector<UpdateData>>> VersionManager::CheckForUpdates()
{
    auto updateCheckEventEmitter = EventEmitter<void, std::vector<UpdateData>>(EventEmitterThreadMode::MULTITHREADED);
    auto updateCheckEventSubscription = updateCheckEventEmitter->SubscribeAsync();
    _updateCheckEventSubscription = updateCheckEventEmitter->SubscribeAsync([=](std::vector<UpdateData> availableUpdates) {
        std::lock_guard<std::mutex> lock(_mtx);
        _availableUpdates = availableUpdates;
    });

    std::thread([updateCheckEventEmitter]() {

        HttpGetRequestData requestData{};
        requestData.baseUrl = "https://api.github.com";
        requestData.path = "/repos/ZenoXi/osu-overlays/releases";
        requestData.headers = {
            { "Accept", "application/json" },
            { "User-Agent", "ZenoXi" }
        };
        std::string requestString = requestData.ToJson() + "\n";
        std::cout << requestString;

        auto requestRunner = ProcessRunner("bin/http-forwarder");
        if (!requestRunner.StartProcess())
        {
            updateCheckEventEmitter->InvokeAll({});
            return;
        }

        unsigned long position = 0;
        while (1)
        {
            unsigned long written;
            requestRunner.WriteToProcess(requestString.substr(position), &written);
            position += written;
            if (position >= requestString.length())
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        SimpleTimer timeoutTimer;
        while (!requestRunner.Finished())
        {
            if (timeoutTimer.SecondsElapsed() > 10)
            {
                updateCheckEventEmitter->InvokeAll({});
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (requestRunner.ExitCode() != 0)
        {
            updateCheckEventEmitter->InvokeAll({});
            return;
        }

        std::string response = requestRunner.GetOutput();
        if (response.length() <= 3)
        {
            updateCheckEventEmitter->InvokeAll({});
            return;
        }

        std::string body = response.substr(4);

        try
        {
            std::vector<UpdateData> availableUpdates;

            zjson::JsonParser releasesParser(body);
            size_t releaseCount = releasesParser.GetArraySize("");
            for (int i = 0; i < releaseCount; i++)
            {
                std::string versionTagString;
                if (!releasesParser.IsNull("[" + std::to_string(i) + "].tag_name"))
                    versionTagString = releasesParser.GetString("[" + std::to_string(i) + "].tag_name");
                if (versionTagString.empty())
                    continue;
                auto versionTagOpt = VersionTag::Parse(versionTagString);
                if (!versionTagOpt)
                    continue;
                if (versionTagOpt.value() <= OVERLAY_ENGINE_VERSION)
                    break;

                UpdateData data{};
                data.versionTag = versionTagOpt.value();
                data.htmlUrl = releasesParser.GetString("[" + std::to_string(i) + "].html_url");
                data.zipName = releasesParser.GetString("[" + std::to_string(i) + "].assets[0].name");
                data.zipUrl = releasesParser.GetString("[" + std::to_string(i) + "].assets[0].url");
                data.publishedAt = releasesParser.GetString("[" + std::to_string(i) + "].published_at");
                data.description = releasesParser.GetString("[" + std::to_string(i) + "].body");

                size_t minVerTagStartPos = data.description.find("MIN_VER=");
                if (minVerTagStartPos != std::string::npos)
                {
                    minVerTagStartPos += std::string("MIN_VER=").length();
                    size_t minVerTagEndPos = data.description.find("]", minVerTagStartPos);
                    if (minVerTagEndPos != std::string::npos)
                    {
                        std::string minVersionTagString = data.description.substr(minVerTagStartPos, minVerTagEndPos - minVerTagStartPos);
                        auto minVersionTagOpt = VersionTag::Parse(minVersionTagString);
                        if (minVersionTagOpt)
                            data.minUpdatableVersionTag = minVersionTagOpt.value();
                    }
                }

                availableUpdates.push_back(data);
            }

            updateCheckEventEmitter->InvokeAll(availableUpdates);
        }
        catch (zjson::ParseException e)
        {
            updateCheckEventEmitter->InvokeAll({});
        }
    }).detach();

    return updateCheckEventSubscription;
}

std::vector<UpdateData> VersionManager::GetCachedAvailableUpdates()
{
    std::lock_guard<std::mutex> lock(_mtx);
    return _availableUpdates;
}

std::unique_ptr<AsyncEventSubscription<void, std::optional<std::wstring>>> VersionManager::InitiateUpdateToVersion(UpdateData update)
{
    auto updateInitiatedEventEmitter = EventEmitter<void, std::optional<std::wstring>>(EventEmitterThreadMode::MULTITHREADED);
    auto updateInitiatedEventSubscription = updateInitiatedEventEmitter->SubscribeAsync();

    std::thread([updateInitiatedEventEmitter, update]() {
        std::cout << "updating...\n";

        std::string url = update.zipUrl;

        size_t hostLength = GetHostLength(url);
        if (hostLength == 0)
        {
            updateInitiatedEventEmitter->InvokeAll(L"Could not parse the host part, please download the update manually\nURL: " + string_to_wstring(url));
            return;
        }
        std::string host = url.substr(0, hostLength);
        std::string path = url.substr(hostLength);

        HttpGetRequestData requestData{};
        requestData.baseUrl = host;
        requestData.path = path;
        requestData.headers = {
            { "Accept", "application/octet-stream" },
            { "User-Agent", "ZenoXi" }
        };
        std::string requestString = requestData.ToJson() + "\n";
        std::cout << requestString;

        auto requestRunner = ProcessRunner("bin/http-forwarder");
        if (!requestRunner.StartProcess())
        {
            updateInitiatedEventEmitter->InvokeAll(L"Failed to start the downloader. Try again or download the update manually");
            return;
        }

        unsigned long position = 0;
        while (1)
        {
            unsigned long written;
            requestRunner.WriteToProcess(requestString.substr(position), &written);
            position += written;
            if (position >= requestString.length())
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        SimpleTimer timeoutTimer;
        while (!requestRunner.Finished())
        {
            if (timeoutTimer.SecondsElapsed() > 300)
            {
                updateInitiatedEventEmitter->InvokeAll(L"Download timeout. Try again or download the update manually");
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (requestRunner.ExitCode() == -13)
        {
            updateInitiatedEventEmitter->InvokeAll(L"No response from the server. Try again or download the update manually");
            return;
        }
        else if (requestRunner.ExitCode() != 0)
        {
            updateInitiatedEventEmitter->InvokeAll(L"Unexpected error occured while trying to download update. Try again or download the update manually");
            return;
        }

        std::string response = requestRunner.GetOutput();
        if (response.length() <= 3)
        {
            updateInitiatedEventEmitter->InvokeAll(L"Unexpected response received while trying to download update. Try again or download the update manually");
            return;
        }

        std::string status = response.substr(0, 3);
        int statusCode = 0;
        try {
            statusCode = std::stoi(status);
        } catch (std::exception) { }

        if (status != "200")
        {
            updateInitiatedEventEmitter->InvokeAll(L"[" + string_to_wstring(status) + L": " + string_to_wstring(std::string(httplib::status_message(statusCode))) + L"] Try again or download the update manually");
            return;
        }

        std::string base64body = response.substr(4);
        int endIndex = (int)base64body.length();
        while (endIndex > 0 && (base64body[endIndex - 1] == '\n' || base64body[endIndex - 1] == '\r' || base64body[endIndex - 1] == ' '))
            endIndex--;
        if (endIndex <= 0)
        {
            updateInitiatedEventEmitter->InvokeAll(L"Update file not present in response. Try again or download the update manually");
            return;
        }
        std::string zipBytes = base64::from_base64(base64body.substr(0, endIndex));

        namespace fs = std::filesystem;

        fs::path tempPath = fs::current_path() / ".updatetemp";
        try
        {
            if (!fs::create_directory(tempPath))
            {
                std::cout << "[WARN] .updatetemp folder already exists\n";
            }
        }
        catch (fs::filesystem_error e)
        {
            updateInitiatedEventEmitter->InvokeAll(L"Could not create directory for update files. Error: " + string_to_wstring(std::string(e.what())));
            return;
        }

        std::ofstream zipFile(tempPath / update.zipName, std::ios::binary);
        zipFile.write(zipBytes.data(), zipBytes.size());
        zipFile.close();

        std::cout << "extracting..\n";
        int result = system(("cd .updatetemp && tar -xf " + update.zipName).c_str());
        std::cout << "exit code: " << result << '\n';
        if (result != 0)
        {
            fs::remove_all(tempPath);
            updateInitiatedEventEmitter->InvokeAll(L"Could not extract downloaded binaries, please download the update manually");
            return;
        }

        std::ofstream versionFile(tempPath / "old-version");
        versionFile << OVERLAY_ENGINE_VERSION.ToString();
        versionFile.close();

        std::optional<fs::path> exePath;
        for (auto& entry : fs::directory_iterator(tempPath))
        {
            std::cout << entry.path().extension() << '\n';
            if (entry.path().extension() == ".exe")
            {
                exePath = entry.path();
                break;
            }
        }
        if (!exePath)
        {
            fs::remove_all(tempPath);
            updateInitiatedEventEmitter->InvokeAll(L"Could not find any executable in extracted binaries, please download the update manually");
            return;
        }

        STARTUPINFO info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;
        std::wstring args = L"\"" + exePath->wstring() + L"\" -update " + std::to_wstring(GetCurrentProcessId());
        std::wstring exePathStr = exePath->wstring();
        std::wstring tempPathStr = tempPath.wstring();
        if (CreateProcess(exePathStr.c_str(), args.data(), NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, tempPathStr.c_str(), &info, &processInfo))
        {
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
            updateInitiatedEventEmitter->InvokeAll(std::nullopt);
            return;
        }

        updateInitiatedEventEmitter->InvokeAll(L"Could not start the new version. Try again, or download and run the update manually");
    }).detach();

    return updateInitiatedEventSubscription;
}
