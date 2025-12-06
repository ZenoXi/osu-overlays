#pragma once

#include "UICore/Components/Base/Label.h"
#include "Overlays/Overlay.h"
#include "OsuDataProvider/DataProviderView.h"

namespace zcom
{
    class URCounterOverlayComponent : public Label
    {
        DEFINE_COMPONENT(URCounterOverlayComponent, Label)
        DEFAULT_DESTRUCTOR(URCounterOverlayComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        void _OnUpdate() override;

        std::shared_ptr<const Overlay> _overlay;

        std::unique_ptr<DataProviderView> _dataProviderView;

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configValueChangedEventSubscription = nullptr;
    };
}