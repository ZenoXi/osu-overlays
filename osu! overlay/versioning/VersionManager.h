#pragma once

#include "Version.h"

#include "UICore/Helper/EventEmitter.h"

#include <thread>
#include <mutex>
#include <atomic>

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