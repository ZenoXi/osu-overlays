#pragma once

#include "Components/Base/ScrollPanel.h"
#include "Components/Base/NumberInput.h"
#include "Components/Base/Checkbox.h"
#include "Shared/Util/ValueOrDefault.h"
#include "SmokeSimType.h"

namespace zcom
{
    class SmokeSimParameterPanel : public ScrollPanel
    {
    public:
    protected:
        friend class Scene;
        friend class Component;
        SmokeSimParameterPanel(Scene* scene) : ScrollPanel(scene) {}
        void Init(SmokeSimType simType);
    public:
        ~SmokeSimParameterPanel() {}
        SmokeSimParameterPanel(SmokeSimParameterPanel&&) = delete;
        SmokeSimParameterPanel& operator=(SmokeSimParameterPanel&&) = delete;
        SmokeSimParameterPanel(const SmokeSimParameterPanel&) = delete;
        SmokeSimParameterPanel& operator=(const SmokeSimParameterPanel&) = delete;

#pragma region base_class
    public:
        const char* GetName() const { return Name(); }
        static const char* Name() { return "smoke_sim_parameter_panel"; }
#pragma endregion

    private:
        std::unique_ptr<NumberInput> _cellSizeInput = nullptr;
        std::unique_ptr<NumberInput> _threadCountInput = nullptr;
        std::unique_ptr<Checkbox> _fullMonitorCheckbox = nullptr;
        std::unique_ptr<NumberInput> _widthInput = nullptr;
        std::unique_ptr<NumberInput> _heightInput = nullptr;
        std::unique_ptr<NumberInput> _xOffsetInput = nullptr;
        std::unique_ptr<NumberInput> _yOffsetInput = nullptr;

        zutil::ValueOrDefault<int> _trail_cellSize = zutil::ValueOrDefault<int>(4);
        zutil::ValueOrDefault<int> _trail_threadCount = zutil::ValueOrDefault<int>(4);
        zutil::ValueOrDefault<bool> _trail_fullMonitor = zutil::ValueOrDefault<bool>(true);
        zutil::ValueOrDefault<int> _trail_width = zutil::ValueOrDefault<int>(1920);
        zutil::ValueOrDefault<int> _trail_height = zutil::ValueOrDefault<int>(1080);
        zutil::ValueOrDefault<int> _trail_xOffset = zutil::ValueOrDefault<int>(0);
        zutil::ValueOrDefault<int> _trail_yOffset = zutil::ValueOrDefault<int>(0);

        zutil::ValueOrDefault<int> _smoke_cellSize = zutil::ValueOrDefault<int>(4);
        zutil::ValueOrDefault<int> _smoke_threadCount = zutil::ValueOrDefault<int>(4);
        zutil::ValueOrDefault<bool> _smoke_fullMonitor = zutil::ValueOrDefault<bool>(true);
        zutil::ValueOrDefault<int> _smoke_width = zutil::ValueOrDefault<int>(1920);
        zutil::ValueOrDefault<int> _smoke_height = zutil::ValueOrDefault<int>(1080);
        zutil::ValueOrDefault<int> _smoke_xOffset = zutil::ValueOrDefault<int>(0);
        zutil::ValueOrDefault<int> _smoke_yOffset = zutil::ValueOrDefault<int>(0);

        SmokeSimType _simType;
        std::optional<zwnd::WindowId> _overlayWindowId = std::nullopt;

        void _OpenOverlayWindow();
        void _UpdateActiveItems();
    };
}