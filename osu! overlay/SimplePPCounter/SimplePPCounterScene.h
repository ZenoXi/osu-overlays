#pragma once

#include "Scenes/Scene.h"

#include "Components/Base/Label.h"

#include "OsuDataProvider/DataProvider.h"

namespace zcom
{
    struct SimplePPCounterSceneOptions : public SceneOptionsBase
    {

    };

    class SimplePPCounterScene : public Scene
    {
        DEFINE_SCENE(SimplePPCounterScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;

    private:
        std::unique_ptr<Label> _ppLabel = nullptr;
        int _currentPPValue = 0;

        std::unique_ptr<osu::DataProvider> _dataProvider;

        void _Update();
    };
}