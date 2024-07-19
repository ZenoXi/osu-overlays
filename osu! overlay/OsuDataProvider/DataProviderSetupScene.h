#pragma once

#include "Scenes/Scene.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/ScrollPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Dummy.h"
#include "Components/Base/Toggle.h"
#include "Components/Base/TextInput.h"
#include "Shared/Components/LoadingAnimation.h"

namespace zcom
{
    struct DataProviderSetupSceneOptions : public SceneOptionsBase
    {
        bool showError = false;
    };

    class DataProviderSetupScene : public Scene
    {
        DEFINE_SCENE(DataProviderSetupScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
        void Uninit() override;

    private:
        std::unique_ptr<FlexPanel> _mainPanel = nullptr;
        std::unique_ptr<LoadingAnimation> _loadingBar = nullptr;
        std::unique_ptr<Toggle> _osuMemoryToggle = nullptr;
        std::unique_ptr<Label> _statusLabel = nullptr;
        std::unique_ptr<TextInput> _urlInput = nullptr;
        std::unique_ptr<Label> _errorLabel = nullptr;
        std::unique_ptr<ScrollPanel> _descriptionScrollWrapper = nullptr;

        std::unique_ptr<AsyncEventSubscription<void, osu::DataProvider::ConnectionEvent>> _dataProviderConnectionEvent = nullptr;
        bool _waitingForDataProvider = false;

        void _Update();
    };
}