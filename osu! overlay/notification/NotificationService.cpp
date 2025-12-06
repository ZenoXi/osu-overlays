#include "NotificationService.h"

void NotificationService::ShowNotification(const NotificationInfo& notificationInfo)
{
    _showNotificationEventEmitter->InvokeAll(notificationInfo);
}

std::unique_ptr<AsyncEventSubscription<void, NotificationInfo>> NotificationService::SubscribeToShowNotificationEvents()
{
    return _showNotificationEventEmitter->SubscribeAsync();
}
