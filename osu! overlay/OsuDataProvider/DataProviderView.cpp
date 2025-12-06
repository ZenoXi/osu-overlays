#include "App.h"
#include "SharedContext.h"
#include "DataProviderView.h"

DataProviderView::DataProviderView(zcom::Component* owner)
{
    _updateEventSubscription = owner->SubscribePostUpdate([=]() {
        // Create copy of events since that should be very cheap compared to potentially updating lots of UI and hogging the mutex when values change
        std::vector<osu::DataProvider::ConnectionEvent> connectionEvents;
        _dataProviderConnectionEvent->HandlePendingEvents([&](const osu::DataProvider::ConnectionEvent& e) {
            connectionEvents.push_back(e);
        });

        for (auto& event : connectionEvents)
        {
            if (event == osu::DataProvider::CONNECTION_SUCCESSFUL)
                running_ = true;
            else
                running_ = false;
        }
    });
    _dataProviderConnectionEvent = owner->GetScene()->GetApp()->Shared<SharedContext*>()->dataProvider.SubscribeOnConnectionEvent();
    running_ = owner->GetScene()->GetApp()->Shared<SharedContext*>()->dataProvider.Ready();
}