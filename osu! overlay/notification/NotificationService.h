#pragma once

#include "NotificationInfo.h"

#include "UICore/Helper/EventEmitter.h"

class NotificationService
{
    EventEmitter<void, NotificationInfo> _showNotificationEventEmitter = EventEmitter<void, NotificationInfo>(EventEmitterThreadMode::MULTITHREADED);

public:
    void ShowNotification(const NotificationInfo& notificationInfo);

    [[nodiscard]] std::unique_ptr<AsyncEventSubscription<void, NotificationInfo>> SubscribeToShowNotificationEvents();
};