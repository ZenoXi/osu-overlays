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
        std::shared_ptr<std::unique_ptr<AsyncEventSubscription<void, zcom::TooltipParams>>> showRequestSubscriptionWrapper = nullptr;
    };

    class TooltipScene : public Scene
    {
        DEFINE_SCENE(TooltipScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;

    private:
        bool _currentlyDisplayed = false;
        std::optional<uint64_t> _displayId = std::nullopt;

        std::unique_ptr<Label> _label = nullptr;

        std::unique_ptr<AsyncEventSubscription<void, TooltipParams>> _showRequestSubscription = nullptr;
        std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _windowMessageSubscription = nullptr;

        void _Update();
    };
}