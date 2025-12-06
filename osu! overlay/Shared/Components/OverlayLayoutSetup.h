#pragma once

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Checkbox.h"
#include "UICore/Components/Base/DropdownSelector.h"

namespace zcom
{
    enum class AnchorPosition
    {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        CENTER_LEFT,
        CENTER,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT
    };

    class OverlayLayoutSetup : public FlexPanel
    {
        DEFINE_COMPONENT(OverlayLayoutSetup, FlexPanel)
        DEFAULT_DESTRUCTOR(OverlayLayoutSetup)
    protected:
        void Init(const std::wstring& layoutString);

    public:

        [[nodiscard]]
        EventSubscription<void, std::wstring> SubscribeOnOverlayLayoutStringChanged(std::function<void(std::wstring)> handler)
        {
            return _overlayLayoutStringChangedEvent->Subscribe(handler);
        }

    private:
        bool _fillDisplayArea = true;
        int _width = 1;
        int _height = 1;
        int _xOffset = 0;
        int _yOffset = 0;
        AnchorPosition _overlayAnchor = AnchorPosition::TOP_LEFT;
        AnchorPosition _displayAreaAnchor = AnchorPosition::TOP_LEFT;

        Value<bool> _showAdvancedPositionSettings = Value<bool>(false);

        EventEmitter<void, std::wstring> _overlayLayoutStringChangedEvent;

        void _ParseLayoutString(const std::wstring& str);
        void _UpdateLayoutString();

        std::vector<DropdownItem> _CreateAnchorPositionDropdownItems() const;
        std::wstring _AnchorPositionToString(AnchorPosition position) const;    
    };

    void ApplyLayoutStringToComponent(const std::wstring& layoutString, Component* component);
}