#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "ContextMenuScene.h"

void zcom::ContextMenuScene::Init(SceneOptionsBase* options)
{
    if (options)
    {
        ContextMenuSceneOptions* opt = reinterpret_cast<ContextMenuSceneOptions*>(options);
        _menuPanel = Create<MenuPanel>(std::move(opt->params));
        _menuPanel->Resize(_menuPanel->GetBaseWidth(), _menuPanel->GetBaseHeight());
        _basePanel->AddItem(_menuPanel.get());
    }

    _basePanel->SetBackgroundColor(D2D1::ColorF(0.05f, 0.05f, 0.05f));
    _basePanel->SubscribeOnMouseMove([](zcom::Component* item, int x, int y, int dx, int dy) {
        //std::cout << item->GetMousePosX() << ":" << item->GetMousePosY() << '\n';
    }).Detach();
}