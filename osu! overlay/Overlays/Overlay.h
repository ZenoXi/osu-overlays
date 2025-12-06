#pragma once

#include "UICore/Components/Base/ComponentBase.h"

class Overlay
{
public:
    Overlay()
    {
        static uint64_t COUNTER = 0;
        _id = COUNTER++;
    }

    virtual std::unique_ptr<zcom::Component> CreateSetupComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const = 0;
    virtual std::unique_ptr<zcom::Component> CreateOverlayComponent(std::shared_ptr<const Overlay> sharedThis, zcom::Component* parent) const = 0;

    virtual std::wstring GetTitle() const = 0;
    virtual bool RequiresGameData() const = 0;
    virtual bool RequiresUncappedFramerate() const = 0;

    uint64_t Id() const { return _id; }
private:

    uint64_t _id;
};