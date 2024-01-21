#pragma once

#include "Scene.h"

#include "Components/Base/ComponentBase.h"
#include "Components/Base/Label.h"
#include "Components/Base/Button.h"
#include "Window/WindowId.h"

#include <optional>

namespace zcom
{
    struct EntrySceneOptions : public SceneOptionsBase
    {

    };

    class EntryScene : public Scene
    {
        std::unique_ptr<Label> _overlayListLabel = nullptr;
        std::unique_ptr<Label> _smokeSimOverlayLabel = nullptr;
        std::unique_ptr<Button> _smokeSimOverlayButton = nullptr;
        std::unique_ptr<Label> _smokeSimOverlayStatusLabel = nullptr;

        std::optional<zwnd::WindowId> _smokeSimOverlayWindowId = std::nullopt;

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
