#pragma once

#include "UICore/Scenes/DefaultTitleBarScene.h"
#include "UICore/Components/Base/Button.h"
#include "UICore/Components/Base/Label.h"

#include "versioning/VersionManager.h"

namespace zcom
{
    struct TitleBarSceneOptions : public DefaultTitleBarSceneOptions
    {

    };

    class TitleBarScene : public DefaultTitleBarScene
    {
        DEFINE_SCENE(TitleBarScene, DefaultTitleBarScene)
    public:
        ~TitleBarScene();
    protected:
        void Init(SceneOptionsBase* options) override;

    public:
        std::vector<RECT> ExcludedCaptionRects() override;

    private:
        std::unique_ptr<Label> _updateLabel = nullptr;
        std::unique_ptr<Button> _updateButton = nullptr;

        std::unique_ptr<AsyncEventSubscription<void, std::vector<UpdateData>>> _updateCheckSubscription;
    };
}