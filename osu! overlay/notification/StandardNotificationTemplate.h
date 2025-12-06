#pragma once

#include "NotificationInfo.h"

class StandardNotificationTemplate
{
public:
    enum class NotificationType
    {
        NEUTRAL,
        SUCCESS,
        WARNING,
        FAULT
    };

    Duration showDuration = Duration(5, SECONDS);
    std::optional<std::wstring> title = L"This is a popup";
    std::wstring text = L"These are the contents";
    NotificationType type = NotificationType::NEUTRAL;
    std::optional<zcom::Color> customBorderColor = std::nullopt;

    NotificationInfo ToNotificationInfo() const;
};