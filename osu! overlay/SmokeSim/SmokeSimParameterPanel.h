#pragma once

#include "Components/Base/ScrollPanel.h"
#include "Components/Base/NumberInput.h"
#include "Components/Base/Checkbox.h"
#include "SmokeSimType.h"

namespace zcom
{
    class SmokeSimParameterPanel : public ScrollPanel
    {
        DEFINE_COMPONENT(SmokeSimParameterPanel, ScrollPanel)
        DEFAULT_DESTRUCTOR(SmokeSimParameterPanel)
    protected:
        void Init(SmokeSimType simType);

    private:
        std::unique_ptr<NumberInput> _cellSizeInput = nullptr;
        std::unique_ptr<NumberInput> _threadCountInput = nullptr;
        std::unique_ptr<Checkbox> _fullMonitorCheckbox = nullptr;
        std::unique_ptr<NumberInput> _widthInput = nullptr;
        std::unique_ptr<NumberInput> _heightInput = nullptr;
        std::unique_ptr<NumberInput> _xOffsetInput = nullptr;
        std::unique_ptr<NumberInput> _yOffsetInput = nullptr;

        int _trail_cellSize = 0;
        int _trail_threadCount = 0;
        bool _trail_fullMonitor = 0;
        int _trail_width = 0;
        int _trail_height = 0;
        int _trail_xOffset = 0;
        int _trail_yOffset = 0;
        int _smoke_cellSize = 0;
        int _smoke_threadCount = 0;
        bool _smoke_fullMonitor = 0;
        int _smoke_width = 0;
        int _smoke_height = 0;
        int _smoke_xOffset = 0;
        int _smoke_yOffset = 0;

        SmokeSimType _simType = SmokeSimType::CURSOR_TRAIL;
        std::optional<zwnd::WindowId> _overlayWindowId = std::nullopt;
        std::optional<zwnd::WindowId> _colorSelectorWindowId = std::nullopt;

        std::unique_ptr<AsyncEventSubscription<void, zwnd::WindowId>> _windowClosedEventSubscription = nullptr;
        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configChangedEventSubscription = nullptr;

        TimePoint _lastColorInputUpdate = TimePoint(0);
        Component* _colorInput = nullptr;

        void _UpdateColorInput();
        void _OpenOverlayWindow();
        void _UpdateActiveItems();
        void _OpenColorSelector();
    };
}