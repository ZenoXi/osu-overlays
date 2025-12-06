#pragma once

#include "Helper/Value.h"
#include "Components/Base/ComponentBase.h"
#include "OsuDataProvider/DataProvider.h"

// The purpose of this class is to keep track whether osu! data provider is currently running and is connected to.
// Value<T> objects for the data provider state are provided, which get updated on owner component post-update event
class DataProviderView
{
public:
    DataProviderView(zcom::Component* owner);
    Value<bool> running_ = false;
private:
    EventSubscription<void> _updateEventSubscription;
    std::unique_ptr<AsyncEventSubscription<void, osu::DataProvider::ConnectionEvent>> _dataProviderConnectionEvent;
};