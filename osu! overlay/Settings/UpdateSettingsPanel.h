#pragma once

#include "Components/Base/ScrollPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Shared/Components/SectionHeader.h"
#include "OsuWebApi/Response/WebResponse.h"
#include "OsuWebApi/Response/PlayerData.h"
#include "OsuDataProvider/DataProvider.h"
#include "IntegrationSettingsTabOptions.h"

#include "versioning/VersionManager.h"

namespace zcom
{
    class UpdateSettingsPanel : public ScrollPanel
    {
        DEFINE_COMPONENT(UpdateSettingsPanel, ScrollPanel)
        DEFAULT_DESTRUCTOR(UpdateSettingsPanel)
    protected:
        void Init(std::optional<std::any> extraOptions);
    public:
        //void Reinit(std::optional<std::any> extraOptions);
    protected:
        void _OnUpdate() override;

    private:
        std::unique_ptr<FlexPanel> _contentPanel;
        std::unique_ptr<Label> _versionLabel;
        std::unique_ptr<FlexPanel> _updatesPanel;
        std::unique_ptr<Label> _errorLabel;
        
        std::unique_ptr<AsyncEventSubscription<void, std::vector<UpdateData>>> _checkForUpdatesSubscription;
        Value<bool> _waitingForUpdateCheck = false;
        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::wstring>>> _updateInititatedSubscription;
        Value<bool> _waitingForUpdateInitiation = false;

        void _BuildPageFromAvailableUpdateData(std::vector<UpdateData> availableUpdates);
    };
}