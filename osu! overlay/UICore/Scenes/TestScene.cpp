#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "TestScene.h"

void zcom::TestScene::Init(SceneOptionsBase* options)
{
    TestSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const TestSceneOptions*>(options);

    auto button = Create<Button>(L"Test");
    button->size = { 100, 40 };
    button->position = { 100, 100 };

    _basePanel->AddItem(std::move(button));
    _basePanel->backgroundColor = Color(0x1A1A1A);
}