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
        DEFINE_SCENE(TestScene, Scene)
    protected:
        void Init(SceneOptionsBase* options);
    };
}