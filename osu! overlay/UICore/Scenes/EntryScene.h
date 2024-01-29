#pragma once

#include "Scene.h"

#include "Components/Base/ComponentBase.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Button.h"
#include "Window/WindowId.h"
#include "Window/WindowType.h"
#include "Window/WindowProperties.h"

#include <optional>

namespace zcom
{
    struct EntrySceneOptions : public SceneOptionsBase
    {

    };

    class EntryScene : public Scene
    {
        std::unique_ptr<FlexPanel> _mainPanel = nullptr;

        std::unique_ptr<Panel> _selectionPanel = nullptr;
        std::unique_ptr<Label> _overlayListLabel = nullptr;
        std::unique_ptr<FlexPanel> _overlayListPanel = nullptr;
        Component* _currentPropertyPanel = nullptr;

        struct _OverlaySelector
        {
            std::optional<zwnd::WindowId> overlayWindowId = std::nullopt;
            std::wstring overlayWindowClassName = L"";
            Component* statusIndicator = nullptr;
        };
        std::vector<_OverlaySelector> _overlaySelectors;
        void _CreateOverlaySelector(std::wstring buttonText, std::wstring windowClassName, std::function<std::unique_ptr<Component>()> parameterPanelInitFunc);
        std::unique_ptr<AsyncEventSubscription<void, zwnd::WindowId, zwnd::WindowType, zwnd::WindowProperties>> _windowCreatedEventSubscription = nullptr;
        std::unique_ptr<AsyncEventSubscription<void, zwnd::WindowId>> _windowClosedEventSubscription = nullptr;
        void _HandleWindowCreatedEvent(zwnd::WindowId windowId, zwnd::WindowProperties props);
        void _HandleWindowClosedEvent(zwnd::WindowId windowId);

    public:
        EntryScene(App* app, zwnd::Window* window);

        const char* GetName() const { return "entry"; }
        static const char* StaticName() { return "entry"; }

    private:
        void _Init(SceneOptionsBase* options);
        void _Uninit();
        void _Focus();
        void _Unfocus();
        void _Update();
        void _Resize(int width, int height, ResizeInfo info);
    };
}
