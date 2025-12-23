#pragma once

#include "Version.h"

#include "UICore/Helper/EventEmitter.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <sstream>

struct UpdateData
{
    VersionTag versionTag;
    VersionTag minUpdatableVersionTag;
    std::string htmlUrl;
    std::string zipUrl;
    std::string zipName;
    std::string publishedAt;
    std::string description;
};

struct HttpGetRequestData
{
    std::string baseUrl;
    std::string path;
    std::unordered_map<std::string, std::string> headers;

    std::string ToJson()
    {
        int headerIndex = 0;
        std::ostringstream ss("");
        ss << "{";
            ss << "\"BaseUrl\":\"" << baseUrl << "\",";
            ss << "\"Path\":\"" << path << "\",";
            ss << "\"Headers\":{";
            for (auto& header : headers)
                ss << ((headerIndex++ != 0) ? "," : "") << "\"" << header.first << "\":\"" << header.second << "\"";
            ss << "}";
        ss << "}";
        return ss.str();
    }
};

class VersionManager
{
    std::mutex _mtx;
    std::vector<UpdateData> _availableUpdates;

    std::unique_ptr<AsyncEventSubscription<void, std::vector<UpdateData>>> _updateCheckEventSubscription;

public:
    ~VersionManager();

    std::unique_ptr<AsyncEventSubscription<void, std::vector<UpdateData>>> CheckForUpdates();
    std::vector<UpdateData> GetCachedAvailableUpdates();

    std::unique_ptr<AsyncEventSubscription<void, std::optional<std::wstring>>> InitiateUpdateToVersion(UpdateData update);
};