#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "TestScene.h"

zcom::TestScene::TestScene(App* app, zwnd::Window* window)
    : Scene(app, window)
{}

void zcom::TestScene::_Init(SceneOptionsBase* options)
{
    TestSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const TestSceneOptions*>(options);

    _button = Create<Button>(L"Test");
    _button->SetBaseSize(100, 40);
    _button->SetOffsetPixels(100, 100);

    _canvas->AddComponent(_button.get());
    _canvas->SetBackgroundColor(D2D1::ColorF(0.1f, 0.1f, 0.1f, 0.0f));
}

void zcom::TestScene::_Uninit()
{
    _canvas->ClearComponents();
}

void zcom::TestScene::_Focus()
{

}

void zcom::TestScene::_Unfocus()
{

}

void zcom::TestScene::_Update()
{
    _canvas->Update();
}

void zcom::TestScene::_Resize(int width, int height, ResizeInfo info)
{

}