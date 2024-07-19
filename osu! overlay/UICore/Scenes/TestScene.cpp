#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "TestScene.h"

void zcom::TestScene::Init(SceneOptionsBase* options)
{
    TestSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const TestSceneOptions*>(options);

    auto button = Create<Button>(L"Test");
    button->SetBaseSize(100, 40);
    button->SetOffsetPixels(100, 100);

    _basePanel->AddItem(std::move(button));
    _basePanel->SetBackgroundColor(D2D1::ColorF(0.1f, 0.1f, 0.1f, 0.0f));
}