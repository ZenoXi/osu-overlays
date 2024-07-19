#pragma once

#include "ComponentBase.h"

namespace zcom
{
    // Completely barebones component, only contains base component functionality
    class Dummy : public Component
    {
        DEFINE_COMPONENT(Dummy, Component)
        DEFAULT_DESTRUCTOR(Dummy)
        DEFAULT_INIT
    };
}