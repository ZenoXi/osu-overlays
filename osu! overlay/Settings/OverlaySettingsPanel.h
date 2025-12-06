#pragma once

#include "Components/Base/ScrollPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/TextInput.h"
#include "Shared/Components/SectionHeader.h"
#include "OsuDataProvider/DataProvider.h"
#include "IntegrationSettingsTabOptions.h"

namespace zcom
{
    class OverlaySettingsPanel : public ScrollPanel
    {
        DEFINE_COMPONENT(OverlaySettingsPanel, ScrollPanel)
        DEFAULT_DESTRUCTOR(OverlaySettingsPanel)
    protected:
        void Init(std::optional<std::any> extraOptions);
    public:
        void Reinit(std::optional<std::any> extraOptions);

    private:
        std::unique_ptr<FlexPanel> _contentPanel;

        static BOOL _EnumMonitorsProc(HMONITOR, HDC, LPRECT, LPARAM);
    };
}
