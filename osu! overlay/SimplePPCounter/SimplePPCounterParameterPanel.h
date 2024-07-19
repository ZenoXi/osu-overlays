#include "Components/Base/ScrollPanel.h"
#include "Components/Base/Button.h"
#include "OsuDataProvider/DataProvider.h"
#include "Window/WindowId.h"

namespace zcom
{
    class SimplePPCounterParameterPanel : public ScrollPanel
    {
        DEFINE_COMPONENT(SimplePPCounterParameterPanel, ScrollPanel)
        DEFAULT_DESTRUCTOR(SimplePPCounterParameterPanel)
    protected:
        void Init();

    private:
        std::unique_ptr<Button> _enableButton = nullptr;

        std::optional<zwnd::WindowId> _overlayWindowId = std::nullopt;
        std::unique_ptr<AsyncEventSubscription<void, osu::DataProvider::ConnectionEvent>> _dataProviderConnectionEvent = nullptr;

        void _OpenOverlayWindow();
        void _CloseOverlayWindow();
    };
}