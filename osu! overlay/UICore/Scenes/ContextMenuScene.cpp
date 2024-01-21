#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "ContextMenuScene.h"

zcom::ContextMenuScene::ContextMenuScene(App* app, zwnd::Window* window)
    : Scene(app, window)
{}

void zcom::ContextMenuScene::_Init(SceneOptionsBase* options)
{
    if (options)
    {
        ContextMenuSceneOptions* opt = reinterpret_cast<ContextMenuSceneOptions*>(options);
        _menuPanel = Create<MenuPanel>(std::move(opt->params));
        _menuPanel->Resize(_menuPanel->GetBaseWidth(), _menuPanel->GetBaseHeight());
        _canvas->AddComponent(_menuPanel.get());
    }

    _canvas->SetBackgroundColor(D2D1::ColorF(0.05f, 0.05f, 0.05f));
    _canvas->BasePanel()->SubscribeOnMouseMove([](zcom::Component* item, int x, int y) {
        //std::cout << item->GetMousePosX() << ":" << item->GetMousePosY() << '\n';
    }).Detach();
}

void zcom::ContextMenuScene::_Uninit()
{
    _canvas->ClearComponents();
}

void zcom::ContextMenuScene::_Focus()
{

}

void zcom::ContextMenuScene::_Unfocus()
{

}

void zcom::ContextMenuScene::_Update()
{
    _canvas->Update();
}

void zcom::ContextMenuScene::_Resize(int width, int height, ResizeInfo info)
{
    
}