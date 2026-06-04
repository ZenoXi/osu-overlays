#pragma once

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Checkbox.h"
#include "UICore/Components/Base/DropdownSelector.h"

namespace zcom
{
    class CursorSensitivitySetup : public FlexPanel
    {
        DEFINE_COMPONENT(CursorSensitivitySetup, FlexPanel)
        DEFAULT_DESTRUCTOR(CursorSensitivitySetup)
    protected:
        void Init();

    public:

        [[nodiscard]]
        EventSubscription<void, std::wstring> SubscribeOnOverlayLayoutStringChanged(std::function<void(std::wstring)> handler)
        {
            return _overlayLayoutStringChangedEvent->Subscribe(handler);
        }

    private:
        EventEmitter<void, std::wstring> _overlayLayoutStringChangedEvent;

    };
}