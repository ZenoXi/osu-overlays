#pragma once

#include "Scenes/Scene.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Button.h"

namespace zcom
{
    struct UpdateErrorSceneOptions : public SceneOptionsBase
    {
        std::optional<std::wstring> errorText;
        bool showExit = true;
        bool showClose = true;
        bool clearBackground = true;
    };

    class UpdateErrorScene : public Scene
    {
        DEFINE_SCENE(UpdateErrorScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
    };
}