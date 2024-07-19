#pragma once

#include "Scene.h"

#include "Components/Base/ComponentBase.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Button.h"
#include "Components/Base/Toggle.h"
#include "Shared/Components/LoadingAnimation.h"
#include "Window/WindowId.h"
#include "Window/WindowType.h"
#include "Window/WindowProperties.h"
#include "OsuDataProvider/DataProvider.h"

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
        //std::unique_ptr<Label> _overlayListLabel = nullptr;
        std::unique_ptr<FlexPanel> _overlayListPanel = nullptr;
        Component* _currentPropertyPanel = nullptr;

        std::unique_ptr<Toggle> _osuMemoryToggle = nullptr;
        std::unique_ptr<AsyncEventSubscription<void, osu::DataProvider::ConnectionEvent>> _dataProviderConnectionEvent = nullptr;
        bool _waitingForDataProvider = false;

        std::optional<zwnd::WindowId> _dataProviderSetupWindowId = std::nullopt;
        void _OpenDataProviderSetup(bool showError);

        struct _OverlaySelector
        {
            std::optional<zwnd::WindowId> overlayWindowId = std::nullopt;
            std::wstring overlayWindowClassName = L"";
            Component* statusIndicator = nullptr;
        };
        std::vector<_OverlaySelector> _overlaySelectors;
        void _CreateOverlaySelector(std::wstring buttonText, std::wstring windowClassName, std::function<std::unique_ptr<Component>()> parameterPanelInitFunc, bool dataProviderRequired = false);
        std::unique_ptr<AsyncEventSubscription<void, zwnd::WindowId, zwnd::WindowType, zwnd::WindowProperties>> _windowCreatedEventSubscription = nullptr;
        std::unique_ptr<AsyncEventSubscription<void, zwnd::WindowId>> _windowClosedEventSubscription = nullptr;
        void _HandleWindowCreatedEvent(zwnd::WindowId windowId, zwnd::WindowProperties props);
        void _HandleWindowClosedEvent(zwnd::WindowId windowId);

        void _Update();
    };
}
