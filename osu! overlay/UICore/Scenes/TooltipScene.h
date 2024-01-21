#pragma once

#include "Scene.h"
#include "Components/Base/Label.h"
#include "Helper/EventEmitter.h"
#include "Window/WindowMessage.h"
#include "TooltipParams.h"

#include <string>
#include <optional>

namespace zcom
{
    struct TooltipSceneOptions : public SceneOptionsBase
    {
        //EventEmitter<void, TooltipParams> showRequestEmitter;
        //std::unique_ptr<AsyncEventSubscription<void, TooltipParams>> showRequestSubscription = nullptr;
        std::shared_ptr<std::unique_ptr<AsyncEventSubscription<void, zcom::TooltipParams>>> showRequestSubscriptionWrapper = nullptr;
    };

    class TooltipScene : public Scene
    {
        bool _currentlyDisplayed = false;
        std::optional<uint64_t> _displayId = std::nullopt;

        std::unique_ptr<Label> _label = nullptr;

        std::unique_ptr<AsyncEventSubscription<void, TooltipParams>> _showRequestSubscription = nullptr;
        std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _windowMessageSubscription = nullptr;

    public:
        TooltipScene(App* app, zwnd::Window* window);

        const char* GetName() const { return "TooltipScene"; }
        static const char* StaticName() { return "TooltipScene"; }

    private:
        void _Init(SceneOptionsBase* options);
        void _Uninit();
        void _Focus();
        void _Unfocus();
        void _Update();
        void _Resize(int width, int height, ResizeInfo info);
    };
}