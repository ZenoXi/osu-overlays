#pragma once

#include "UICore/Components/Base/ComponentBase.h"

#include <memory>
#include <functional>

struct NotificationInfo
{
    Duration showDuration;
    std::function<std::unique_ptr<zcom::Component>(zcom::Component*)> componentBuilder;
};