#pragma once

#include "NotificationInfo.h"

#include "UICore/Components/Base/FlexPanel.h"

namespace zcom
{
    class NotificationDisplayComponent : public FlexPanel
    {
        DEFINE_COMPONENT(NotificationDisplayComponent, FlexPanel)
        DEFAULT_DESTRUCTOR(NotificationDisplayComponent)
    protected:
        void Init();
    protected:
        void _OnUpdate() override;

    private:
        std::unique_ptr<AsyncEventSubscription<void, NotificationInfo>> _showNotificationSubscription;

        struct _Notification
        {
            TimePoint createTime;
            Duration showDuration;
            std::unique_ptr<Component> component;
        };
        std::vector<_Notification> _notifications;
        bool _notificationLayoutChanged = false;
    };
}