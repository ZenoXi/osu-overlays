#pragma once

#include "Scene.h"

#include "Components/Base/Button.h"

namespace zcom
{
    struct TestSceneOptions : public SceneOptionsBase
    {

    };

    class TestScene : public Scene
    {
        std::unique_ptr<Button> _button = nullptr;

    public:
        TestScene(App* app, zwnd::Window* window);

        const char* GetName() const { return "TestScene"; }
        static const char* StaticName() { return "TestScene"; }

    private:
        void _Init(SceneOptionsBase* options);
        void _Uninit();
        void _Focus();
        void _Unfocus();
        void _Update();
        void _Resize(int width, int height, ResizeInfo info);
    };
}