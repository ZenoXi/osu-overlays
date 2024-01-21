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
        std::unique_ptr<MenuPanel> _menuPanel = nullptr;

    public:
        ContextMenuScene(App* app, zwnd::Window* window);

        const char* GetName() const { return "ContextMenuScene"; }
        static const char* StaticName() { return "ContextMenuScene"; }

    private:
        void _Init(SceneOptionsBase* options);
        void _Uninit();
        void _Focus();
        void _Unfocus();
        void _Update();
        void _Resize(int width, int height, ResizeInfo info);
    };
}