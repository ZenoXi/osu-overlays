#include "Components/Base/ScrollPanel.h"
#include "Components/Base/Button.h"
#include "Components/Base/NumberInput.h"
#include "Components/Base/Dummy.h"
#include "Components/Base/Label.h"
#include "OsuDataProvider/DataProvider.h"
#include "Window/WindowId.h"

namespace zcom
{
    class ComboBarParameterPanel : public ScrollPanel
    {
        DEFINE_COMPONENT(ComboBarParameterPanel, ScrollPanel)
        DEFAULT_DESTRUCTOR(ComboBarParameterPanel)
    protected:
        void Init();

    private:
        std::unique_ptr<Button> _enableButton = nullptr;
        std::unique_ptr<Label> _interactionHelpLabel = nullptr;
        std::unique_ptr<NumberInput> _widthInput = nullptr;
        std::unique_ptr<NumberInput> _heightInput = nullptr;
        std::unique_ptr<NumberInput> _xOffsetInput = nullptr;
        std::unique_ptr<NumberInput> _yOffsetInput = nullptr;
        std::unique_ptr<Button> _applyCurrentButton = nullptr;
        std::unique_ptr<Button> _useCurrentButton = nullptr;
        std::unique_ptr<Dummy> _barColorInput = nullptr;
        std::unique_ptr<Dummy> _textColorInput = nullptr;
        std::unique_ptr<Dummy> _gridColorInput = nullptr;

        std::optional<zwnd::WindowId> _overlayWindowId = std::nullopt;
        std::optional<zwnd::WindowId> _barColorSelectorWindowId = std::nullopt;
        std::optional<zwnd::WindowId> _textColorSelectorWindowId = std::nullopt;
        std::optional<zwnd::WindowId> _gridColorSelectorWindowId = std::nullopt;

        std::unique_ptr<AsyncEventSubscription<void, zwnd::WindowId>> _windowClosedEventSubscription = nullptr;
        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configChangedEventSubscription = nullptr;
        std::unique_ptr<AsyncEventSubscription<void, osu::DataProvider::ConnectionEvent>> _dataProviderConnectionEvent = nullptr;

        void _OpenOverlayWindow();
        void _CloseOverlayWindow();
        void _UpdateButtonsBasedOnOverlayState();
        void _UpdateColorInput();
        std::optional<zwnd::WindowId> _OpenColorSelector(std::wstring windowTitle, std::wstring wndClass, ConfigValue<int> colorConfigValue);
    };
}