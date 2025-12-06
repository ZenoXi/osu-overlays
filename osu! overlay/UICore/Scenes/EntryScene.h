#pragma once

#include "Scene.h"

#include "Components/Base/ComponentBase.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Button.h"
#include "Components/Base/Toggle.h"
#include "Shared/Components/LoadingAnimation.h"
#include "OsuDataProvider/DataProvider.h"
#include "Overlays/Overlay.h"
#include "Overlays/OverlayView.h"

#include <optional>

namespace zcom
{
    struct EntrySceneOptions : public SceneOptionsBase
    {

    };

    class EntryScene : public Scene
    {
        DEFINE_SCENE(EntryScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
    private:
        std::unique_ptr<FlexPanel> _mainPanel = nullptr;

        std::unique_ptr<LoadingAnimation> _loadingBar = nullptr;
        std::unique_ptr<FlexPanel> _selectionPanel = nullptr;
        std::unique_ptr<FlexPanel> _overlayListPanel = nullptr;
        Component* _currentPropertyPanel = nullptr;

        std::unique_ptr<Toggle> _osuMemoryToggle = nullptr;
        std::unique_ptr<AsyncEventSubscription<void, osu::DataProvider::ConnectionEvent>> _dataProviderConnectionEvent = nullptr;
        bool _waitingForDataProvider = false;

        std::vector<std::shared_ptr<const Overlay>> _registeredOverlays;

        struct _OverlaySelector
        {
            std::unique_ptr<Panel> selectorComponent;
            std::unique_ptr<OverlayView> overlayView;
        };
        std::vector<_OverlaySelector> _overlaySelectors;

        void _Update();
    };
}
