#pragma once

#include "OsuWebApi/WebApi.h"
#include "OsuDataProvider/DataProvider.h"
#include "Overlays/OverlayManager.h"
#include "Settings/SettingsWindowController.h"
#include "notification/NotificationService.h"
#include "versioning/VersionManager.h"

class SharedContext
{
public:
    osu::DataProvider dataProvider;
    webapi::WebApi webApi;

    OverlayManager overlayManager;
    SettingsWindowController settingsWindow;
    NotificationService notificationService;
    VersionManager versionManager;

    std::atomic<zcom::Point> gameCursorPosition = zcom::Point();
};