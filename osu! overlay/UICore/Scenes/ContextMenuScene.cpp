#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "ContextMenuScene.h"

void zcom::ContextMenuScene::Init(SceneOptionsBase* options)
{
    if (options)
    {
        ContextMenuSceneOptions* opt = reinterpret_cast<ContextMenuSceneOptions*>(options);
        _menuPanel = Create<MenuPanel>(std::move(opt->params));
        _menuPanel->parentSize = { 1.0f, 1.0f };
        _basePanel->AddItem(_menuPanel.get());
    }

    _basePanel->backgroundColor = Color(0x0D0D0D);
    _basePanel->SubscribeOnMouseMove([](zcom::Component* item, Point point, Point deltaPos) {
        //std::cout << item->GetMousePosX() << ":" << item->GetMousePosY() << '\n';
    }).Detach();
}