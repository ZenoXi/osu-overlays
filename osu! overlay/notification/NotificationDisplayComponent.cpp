#include "App.h"
#include "SharedContext.h"
#include "Window/Window.h"
#include "NotificationDisplayComponent.h"

#include "UICore/Helper/AnimationHelper.h"

void zcom::NotificationDisplayComponent::Init()
{
    FlexPanel::Init(FlexDirection::DOWN);
    padding = { 10, 10, 10, 10 };
    spacing = 5;

    _showNotificationSubscription = _scene->GetApp()->Shared<SharedContext*>()->notificationService.SubscribeToShowNotificationEvents();
}

void zcom::NotificationDisplayComponent::_OnUpdate()
{
    Panel::_OnUpdate();

    _showNotificationSubscription->HandlePendingEvents([&](NotificationInfo info) {
        _Notification notification;
        notification.component = info.componentBuilder(this);
        notification.component->parentSize = { 1.0f, 0.0f };
        notification.component->SetProperty(FlexIgnore());
        notification.createTime = ztime::Main();
        notification.showDuration = info.showDuration;
        _notifications.insert(_notifications.begin(), std::move(notification));
        _notificationLayoutChanged = true;

        InsertItem(_notifications.front().component.get(), 0);
    });

    for (int i = 0; i < _notifications.size(); i++)
    {
        // Create animation
        auto millisAlive = (ztime::Main() - _notifications[i].createTime).GetDuration(MILLISECONDS);
        if (millisAlive < 300)
        {
            _notifications[i].component->opacity = millisAlive / 300.0f;
            int heightWithSpacing = _notifications[i].component->size_->height + spacing;
            _notifications[i].component->SetProperty(FlexMarginAfter(int(-heightWithSpacing + heightWithSpacing * zanim::EaseOutQuad(millisAlive / 300.0f))));
            int widthWithPadding = _notifications[i].component->size_->width + 50;
            _notifications[i].component->position = { widthWithPadding - int(widthWithPadding * zanim::EaseOutQuad(millisAlive / 300.0f)), 0 };
        }
        else
        {
            _notifications[i].component->opacity = 1.0f;
            _notifications[i].component->SetProperty(FlexMarginAfter(0));
            _notifications[i].component->position = { 0, 0 };
        }
        _notifications[i].component->RemoveProperty<FlexIgnore>();

        // Destroy animation
        auto millisLeftToLive = ((_notifications[i].createTime + _notifications[i].showDuration) - ztime::Main()).GetDuration(MILLISECONDS);
        if (millisLeftToLive < 1000)
        {
            _notifications[i].component->opacity = millisLeftToLive / 1000.0f;
        }
        if (millisLeftToLive < 300)
        {
            int heightWithSpacing = _notifications[i].component->size_->height + spacing;
            _notifications[i].component->SetProperty(FlexMarginAfter(int(-heightWithSpacing * zanim::EaseOutQuad(1.0f - millisLeftToLive / 300.0f))));
        }
        if (millisLeftToLive <= 0)
        {
            RemoveItem(_notifications[i].component.get());
            _notifications.erase(_notifications.begin() + i);
            i--;
        }
    }
}
