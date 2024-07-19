#include "Components/Base/ScrollPanel.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Button.h"
#include "Components/Base/NumberInput.h"
#include "Components/Base/Dummy.h"
#include "Components/Base/Label.h"
#include "Components/Base/Checkbox.h"
#include "Components/Base/Slider.h"
#include "Shared/Util/Color.h"
#include "Shared/Components/ColorSliderInputGroup.h"
#include "OsuDataProvider/DataProvider.h"
#include "Window/WindowId.h"

namespace zcom
{
    class CursorTrailParameterPanel : public ScrollPanel
    {
        DEFINE_COMPONENT(CursorTrailParameterPanel, ScrollPanel)
        DEFAULT_DESTRUCTOR(CursorTrailParameterPanel)
    protected:
        void Init();

    private:
        std::unique_ptr<Button> _enableButton = nullptr;
        std::unique_ptr<Checkbox> _fullMonitorCheckbox = nullptr;
        NumberInput* _widthInput = nullptr;
        NumberInput* _heightInput = nullptr;
        NumberInput* _xOffsetInput = nullptr;
        NumberInput* _yOffsetInput = nullptr;
        std::unique_ptr<Dummy> _headColorInput = nullptr;
        std::unique_ptr<Slider> _paletteSlider = nullptr;
        std::unique_ptr<NumberInput> _positionInput = nullptr;
        std::unique_ptr<ColorSliderInputGroup> _redColorInputGroup = nullptr;
        std::unique_ptr<ColorSliderInputGroup> _greenColorInputGroup = nullptr;
        std::unique_ptr<ColorSliderInputGroup> _blueColorInputGroup = nullptr;
        std::unique_ptr<ColorSliderInputGroup> _opacityInputGroup = nullptr;
        std::unique_ptr<FlexPanel> _gradientStopPanel = nullptr;

        std::unique_ptr<FlexPanel> _insertionMarker = nullptr;
        int _hoveredColorSelectorIndex = 0;
        bool _hoveredColorSelectorIsTopHalf = false;

        std::optional<zwnd::WindowId> _overlayWindowId = std::nullopt;
        std::optional<zwnd::WindowId> _headColorSelectorWindowId = std::nullopt;

        std::unique_ptr<AsyncEventSubscription<void, zwnd::WindowId>> _windowClosedEventSubscription = nullptr;
        std::unique_ptr<AsyncEventSubscription<void, osu::DataProvider::ConnectionEvent>> _dataProviderConnectionEvent = nullptr;

        void _CreateInsertionMarker();
        void _SetInsertionMarkerColors(Button* head, Image* transition, Dummy* separator);
        std::unique_ptr<FlexPanel> _SetUpGeneralPanel();
        std::unique_ptr<FlexPanel> _SetUpAppearancePanel();
        std::unique_ptr<FlexPanel> _SetUpColorsPanel();

        void _UpdateActiveItems();
        void _UpdateColorInput();
        void _OpenOverlayWindow();
        void _CloseOverlayWindow();
        void _UpdateButtonsBasedOnOverlayState();

        struct _GradientStop
        {
            zutil::Color color;
            float position;

            std::unique_ptr<FlexPanel> selectorItem;
            std::unique_ptr<Dummy> colorIndicator;
            std::unique_ptr<Label> redLabel;
            std::unique_ptr<Label> greenLabel;
            std::unique_ptr<Label> blueLabel;
            std::unique_ptr<Label> opacityLabel;
            std::unique_ptr<Label> positionLabel;
            std::unique_ptr<Dummy> redUnderline;
            std::unique_ptr<Dummy> greenUnderline;
            std::unique_ptr<Dummy> blueUnderline;
            std::unique_ptr<Dummy> opacityUnderline;
            std::unique_ptr<Button> removeButton;
            EventSubscription<void, Component*> mouseEnterEventSubscription;
            EventSubscription<void, Component*> mouseLeaveEventSubscription;
            EventSubscription<void, Component*, std::vector<EventTargets::Params>, int, int> postLeftPressedEventSubscription;
            EventSubscription<void> removeEventSubscription;
        };

        bool _paletteSliderHovered = false;
        std::vector<_GradientStop> _currentPalette;
        zutil::Color _currentColor = zutil::Color();
        int _currentColorIndex = 0;
        ID2D1LinearGradientBrush* _CreateGradientBrush(Component* item, Graphics g, const std::vector<_GradientStop>& stops);
        ID2D1ImageBrush* _CreateCheckeredPatternBrush(Graphics g, D2D1_COLOR_F cellColor);
        void _BuildItemsForPalette();
        void _CreateInnerGradientStopComponents(_GradientStop& stop);
        void _ReorderColorList();
        void _UpdateColorListBackgrounds();
        void _OnGradientStopSelected(int index);
        void _OnGradientStopRemoved(int index);
        void _OnColorChanged();
        void _OnColorPositionChanged(float position);

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configChangedEventSubscription = nullptr;
        TimePoint _lastSaveTime = TimePoint(0);
        bool _configSaved = true;
        void _LoadPalette();
        void _ParsePaletteString(const std::wstring& str);
        void _SavePalette();

        std::optional<zwnd::WindowId> _OpenColorSelector(std::wstring windowTitle, std::wstring wndClass, ConfigValue<int> colorConfigValue);
    };
}