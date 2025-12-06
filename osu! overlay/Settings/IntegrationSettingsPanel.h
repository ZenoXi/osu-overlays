#pragma once

#include "Components/Base/ScrollPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/TextInput.h"
#include "Shared/Components/SectionHeader.h"
#include "OsuWebApi/Response/WebResponse.h"
#include "OsuWebApi/Response/PlayerData.h"
#include "OsuDataProvider/DataProvider.h"
#include "IntegrationSettingsTabOptions.h"

namespace zcom
{
    class IntegrationSettingsPanel : public ScrollPanel
    {
        DEFINE_COMPONENT(IntegrationSettingsPanel, ScrollPanel)
        DEFAULT_DESTRUCTOR(IntegrationSettingsPanel)
    protected:
        void Init(std::optional<std::any> extraOptions);
    public:
        void Reinit(std::optional<std::any> extraOptions);

    private:
        std::unique_ptr<FlexPanel> _contentPanel;
        std::unique_ptr<TextInput> _apiUrlInput;
        std::unique_ptr<TextInput> _dataProviderUrlInput;

        enum class _ApiTestResponse
        {
            OK,
            FAILED
        };
        std::unique_ptr<AsyncEventSubscription<void, webapi::resp::WebResponse<webapi::resp::PlayerData>>> _playerDataGetEvent = nullptr;
        Value<bool> _apiSettingsChanged = false;
        Value<bool> _waitingForApiConnection = false;
        Value<bool> _showApiConnectionResult = false;

        Value<_ApiTestResponse> _apiTestResponse = _ApiTestResponse::OK;
        Value<bool> _showWebApiHelp = Value<bool>(false, [=](bool& currentValue, const bool& show) {
            currentValue = show;
            _SaveShowDataProviderHelpValue(show);
        });

        std::unique_ptr<AsyncEventSubscription<void, osu::DataProvider::ConnectionEvent>> _dataProviderConnectionEvent = nullptr;
        Value<bool> _waitingForDataProvider = false;
        Value<bool> _dataProviderConnected = false;
        Value<bool> _showDataProviderError = false;
        Value<bool> _showDataProviderHelp = Value<bool>(false, [=](bool& currentValue, const bool& show) {
            currentValue = show;
            _SaveShowDataProviderHelpValue(show);
        });

        void _CreateWebSection();
        void _CreateDataProviderSection();
        void _SaveApiSettings();
        void _SaveShowWebApiHelpValue(bool value);
        void _SaveShowDataProviderHelpValue(bool value);

    protected:
        void _OnUpdate() override;
    };
}