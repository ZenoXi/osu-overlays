#pragma once

#include "UICore/Components/Base/ScrollPanel.h"
#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Button.h"
#include "UICore/Components/Base/NumberInput.h"
#include "UICore/Components/Base/Image.h"
#include "UICore/Components/Base/Dummy.h"
#include "UICore/Components/Base/Slider.h"
#include "UICore/Components/Base/ColorSelector.h"
#include "UICore/Components/Custom/ColorSliderInputGroup.h"

#include "Overlays/Overlay.h"
#include "Overlays/OverlayView.h"

namespace zcom
{
    class CursorTrailSetupComponent : public ScrollPanel
    {
        DEFINE_COMPONENT(CursorTrailSetupComponent, ScrollPanel)
        DEFAULT_DESTRUCTOR(CursorTrailSetupComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);
        void _OnUpdate() override;

    private:
        std::shared_ptr<const Overlay> _overlay;
        std::optional<OverlayView> _overlayView;

        std::unique_ptr<ColorSelector> _headColorInput = nullptr;
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

        void _CreateInsertionMarker();
        void _SetInsertionMarkerColors(Button* head, Image* transition, Dummy* separator);
        std::unique_ptr<FlexPanel> _SetUpGeneralPanel();
        std::unique_ptr<FlexPanel> _SetUpAppearancePanel();
        std::unique_ptr<FlexPanel> _SetUpColorsPanel();

        struct _GradientStop
        {
            Color color;
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
            EventSubscription<void, Component*, std::vector<EventContext::Params>, Point> postLeftPressedEventSubscription;
            EventSubscription<void> removeEventSubscription;
        };

        bool _paletteSliderHovered = false;
        std::vector<_GradientStop> _currentPalette;
        Color _currentColor = Color();
        int _currentColorIndex = 0;
        ID2D1ImageBrush* _CreateCheckeredPatternBrush(Graphics* g, D2D1_COLOR_F cellColor);
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
    };
}