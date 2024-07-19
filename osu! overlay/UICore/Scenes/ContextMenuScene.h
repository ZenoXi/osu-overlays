#pragma once

#include "Scene.h"
#include "Components/Base/MenuItem.h"
#include "Components/Base/MenuPanel.h"
#include "Components/Base/MenuTemplate.h"

namespace zcom
{
    struct ContextMenuSceneOptions : public SceneOptionsBase
    {
        MenuParams params;
    };

    class ContextMenuScene : public Scene
    {
        DEFINE_SCENE(ContextMenuScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;

    private:
        std::unique_ptr<MenuPanel> _menuPanel = nullptr;
    };
}