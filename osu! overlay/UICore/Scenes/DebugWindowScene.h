#pragma once

#include "Scene.h"

#include "Components/Base/Label.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/ScrollPanel.h"
#include "Components/Base/TextInput.h"
#include "Components/Base/NumberInput.h"
#include "Components/Base/DropdownSelector.h"
#include "Components/Base/Checkbox.h"
#include "Components/Base/ColorSelector.h"
#include "Components/Base/Button.h"
#include "WindowEvent.h"

namespace zcom
{
    struct DebugWindowSceneOptions : public SceneOptionsBase
    {

    };

    // Class that overrides children getters to allow inspector window to inspect itself without blowing up
    class InspectorPanel : public ScrollPanel
    {
        DEFINE_COMPONENT(InspectorPanel, ScrollPanel)
        DEFAULT_DESTRUCTOR(InspectorPanel)
    protected:
        void Init() { ScrollPanel::Init(); }
    public:
        std::vector<Component*> GetChildren() override { return Component::GetChildren(); }
        std::vector<Component*> GetAllChildren() override { return Component::GetAllChildren(); }
    };

    class DebugWindowScene : public Scene
    {
        DEFINE_SCENE(DebugWindowScene, Scene)
    protected:
        void Init(SceneOptionsBase* options);

    private:
        std::unique_ptr<Label> _listLabel = nullptr;
        std::unique_ptr<FlexPanel> _inspectorTabPanel = nullptr;
        std::unique_ptr<ScrollPanel> _windowListScrollPanel = nullptr;
        std::unique_ptr<FlexPanel> _windowListContentPanel = nullptr;
        std::unique_ptr<ScrollPanel> _inspectorPanel = nullptr;
        std::unique_ptr<ScrollPanel> _valueEditorPanel = nullptr;
        std::unique_ptr<FlexPanel> _valueEditorItems = nullptr;
        std::unique_ptr<Label> _noValuesLabel = nullptr;

        bool _categoriesChanged = true;

        zwnd::WindowId _selectedWindowId;
        Value<uint64_t> _selectedComponentId = 0;
        std::atomic<uint64_t> _hoveredComponentId = 0;
        std::atomic<bool> _hoverEnded = false;

        struct ComponentView
        {
            uint64_t id{};
            const char* name = "";
            std::vector<ComponentView> children;
        };
        ComponentView _UIRootView;
        bool _rootViewGot = false;
        std::mutex _m_uiView;
        struct ComponentNode
        {
            uint64_t componentId{};
            std::unique_ptr<Value<bool>> expanded;
            std::unique_ptr<Panel> nodePanel;
            std::unique_ptr<Button> expandButton;
            std::unique_ptr<Label> nameLabel;
            std::vector<ComponentNode> nodes;
        };
        ComponentNode _rootNode;
        bool _rootNodeLoaded = false;

        struct PropertyView
        {
            ValueProxy::ValueType type = ValueProxy::TEXT;
            std::string name;
            std::optional<std::wstring> text;
            std::optional<int64_t> selection;
            std::optional<ValueProxy::Number> number;
            std::optional<bool> boolState;
            std::optional<Color> color;
            bool optional = false;

            int numberPrecision = 0;
            ValueProxy::Number numberMinValue = ValueProxy::Number(std::numeric_limits<int>::min());
            ValueProxy::Number numberMaxValue = ValueProxy::Number(std::numeric_limits<int>::max());
            ValueProxy::Number numberStepSize = ValueProxy::Number(1);

            std::vector<std::pair<int64_t, std::wstring>> selectionValues;
            bool dropdownAbove = false;
        };
        std::vector<std::pair<std::string, std::vector<PropertyView>>> _UIPropertyCategoryViews;

        struct Property
        {
            ValueProxy::ValueType type = ValueProxy::TEXT;
            std::string name;
            std::unique_ptr<FlexPanel> propertyPanel;
            std::unique_ptr<Checkbox> enableCheckbox;
            std::unique_ptr<Label> nameLabel;
            std::unique_ptr<TextInput> textInput;
            std::unique_ptr<NumberInput> numberInput;
            std::unique_ptr<DropdownSelector> selectionInput;
            std::unique_ptr<Checkbox> boolInput;
            std::unique_ptr<ColorSelector> colorInput;
            
            bool editing = false;
            bool updated = false;
        };
        struct PropertyCategory
        {
            std::string name;
            std::unique_ptr<Label> label;
            std::unique_ptr<Panel> panel;
            std::vector<Property> properties;
        };
        std::vector<PropertyCategory> _categories;

        struct Window
        {
            zwnd::WindowId windowId;
            std::wstring windowTitle;
            std::wstring windowName;
            std::unique_ptr<Label> windowTab;
        };

        EventSubscription<void, zwnd::Window*> _uiDebugHookEventSubscription;
        EventSubscription<void, zwnd::Window*, bool*> _uiDebugRenderCheckHookEventSubscription;
        EventSubscription<void, zwnd::Window*, Graphics*> _uiDebugRenderHookEventSubscription;

        std::unique_ptr<AsyncEventSubscription<void, WindowEvent>> _windowEventSubscription;
        std::mutex _m_windowEvent;
        std::vector<Window> _openWindows;
        bool _openWindowsLoaded = false;

        void _Update();
        void _UpdateWindowList();
        void _SelectWindow(zwnd::WindowId id);
        void _SelectComponent(uint64_t id);
        void _OnUIDebugHookEvent(zwnd::Window* window);
        void _OnUIDebugRenderCheckHookEvent(zwnd::Window* window, bool* redraw);
        void _OnUIDebugRenderHookEvent(zwnd::Window* window, Graphics* g);
        void _UpdateInspectorPanel();
        void _BuildUIView(Component* component, ComponentView& view);
        void _BuildComponentDataView(Component* component);
        void _ApplyChangesToProperties(std::vector<std::pair<std::string, std::vector<ValueProxy>>>& reflectionData);
        void _UpdateNodes(ComponentView* component, ComponentNode& node, int depth);
        ComponentNode _CreateNode(uint64_t componentId, std::string componentName, int depth);
        bool _ExpandNode(ComponentNode& node, uint64_t componentId);
        int _PositionNodes(ComponentNode& node, int offset, bool hidden);
        void _UpdateProperties(std::vector<std::pair<std::string, std::vector<PropertyView>>>& categoryViews);
        void _PositionProperties(std::vector<std::pair<std::string, std::vector<PropertyView>>>& categoryViews); 
        void _OnPropertySelected(std::string category, std::string name);
        void _OnPropertyDeselected(std::string category, std::string name);
        void _OnPropertyUpdated(std::string category, std::string name);
    };
}