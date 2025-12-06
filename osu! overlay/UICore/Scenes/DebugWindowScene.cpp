#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "DebugWindowScene.h"

void zcom::DebugWindowScene::Init(SceneOptionsBase* options)
{
    DebugWindowSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const DebugWindowSceneOptions*>(options);

    _windowEventSubscription = _app->SubscribeOnWindowEvent();

    _inspectorTabPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    _inspectorTabPanel->parentSize = { 1.0f, 1.0f };
    _inspectorTabPanel->size = { 0, -1 };
    _inspectorTabPanel->position = { 0, 1 };

    _windowListScrollPanel = Create<ScrollPanel>();
    _windowListScrollPanel->parentSize = { 0.0f, 1.0f };
    _windowListScrollPanel->size = { 200, 0 };
    _windowListScrollPanel->backgroundColor = Color(0x101010);

    _windowListContentPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _windowListContentPanel->parentSize = { 1.0f, 0.0f };
    _windowListContentPanel->autoHeight = true;

    _inspectorPanel = Create<InspectorPanel>();
    _inspectorPanel->parentSize = { 0.0f, 1.0f };
    _inspectorPanel->SetProperty(FlexGrow());
    _inspectorPanel->backgroundColor = Color(0x202020);
    _inspectorPanel->yScrollbar.scrollable = true;
    _inspectorPanel->yScrollbar.backgroundVisible = true;

    _valueEditorPanel = Create<ScrollPanel>();
    _valueEditorPanel->parentSize = { 0.0f, 1.0f };
    _valueEditorPanel->size = { 300, 0 };
    _valueEditorPanel->SetProperty(FlexMarginBefore(1));
    _valueEditorPanel->backgroundColor = Color(0x202020);
    _valueEditorPanel->yScrollbar.scrollable = true;
    _valueEditorPanel->yScrollbar.backgroundVisible = true;

    _noValuesLabel = Create<Label>(L"Select a component to view values");
    _noValuesLabel->autoWidth = true;
    _noValuesLabel->autoHeight = true;
    _noValuesLabel->xAlign = Alignment::CENTER;
    _noValuesLabel->yAlign = Alignment::CENTER;
    _noValuesLabel->fontStyle = FontStyle::ITALIC;

    _valueEditorItems = Create<FlexPanel>(FlexDirection::DOWN);
    _valueEditorItems->parentSize = { 1.0f, 0.0f };
    _valueEditorItems->autoHeight = true;
    
    _windowListScrollPanel->AddItem(_windowListContentPanel.get());
    _valueEditorPanel->AddItem(_noValuesLabel.get());
    _valueEditorPanel->AddItem(_valueEditorItems.get());
    _inspectorTabPanel->AddItem(_windowListScrollPanel.get());
    _inspectorTabPanel->AddItem(_inspectorPanel.get());
    _inspectorTabPanel->AddItem(_valueEditorPanel.get());

    _basePanel->AddItem(_inspectorTabPanel.get());
    _basePanel->backgroundColor = Color(0);
    _basePanel->SubscribePostUpdate([=]() { _Update(); }).Detach();
}

void zcom::DebugWindowScene::_Update()
{
    bool windowAdded = false;

    if (!_openWindowsLoaded)
    {
        auto windowList = _app->GetWindowList();
        for (auto& windowId : windowList)
        {
            Handle<zwnd::Window> windowHandle = _app->GetWindow(windowId);
            if (windowHandle.Valid())
                _openWindows.push_back({ windowId, L"", windowHandle->Properties().windowClassName, nullptr });
        }
        windowAdded = true;
        _openWindowsLoaded = true;
    }

    _windowEventSubscription->HandlePendingEvents([&](const WindowEvent& e) {
        if (e.eventType == WindowEvent::CREATED)
        {
            bool windowExists = false;
            for (int i = 0; i < _openWindows.size(); i++)
            {
                if (_openWindows[i].windowId == e.windowId)
                {
                    windowExists = true;
                    break;
                }
            }
            if (!windowExists)
            {
                _openWindows.push_back({ e.windowId, L"", e.windowProperties->windowClassName, nullptr });
                windowAdded = true;
            }
        }
        else if (e.eventType == WindowEvent::CLOSED)
        {
            for (int i = 0; i < _openWindows.size(); i++)
            {
                if (_openWindows[i].windowId == e.windowId)
                {
                    _windowListContentPanel->RemoveItem(_openWindows[i].windowTab.get());
                    _openWindows.erase(_openWindows.begin() + i);
                    break;
                }
            }
        }
    });

    if (windowAdded)
    {
        _UpdateWindowList();
    }

    _UpdateInspectorPanel();
}

void zcom::DebugWindowScene::_UpdateWindowList()
{
    for (auto& window : _openWindows)
    {
        if (window.windowTab)
            continue;

        std::wostringstream ss(L"");
        ss << "[" << window.windowId.StringValue().c_str() << "] " << window.windowName;
        window.windowTab = Create<Label>(ss.str());
        window.windowTab->parentSize = { 1.0f, 0.0f };
        window.windowTab->size = { 0, 26 };
        window.windowTab->padding = { 5, 0, 0, 0 };
        window.windowTab->yTextAlign = Alignment::CENTER;
        window.windowTab->SubscribeOnLeftPressed([=, id = window.windowId](Component*, Point) {
            _SelectWindow(id);
        }).Detach();
        _windowListContentPanel->AddItem(window.windowTab.get());
    }
}

void zcom::DebugWindowScene::_SelectWindow(zwnd::WindowId id)
{
    Handle<zwnd::Window> windowHandle = _app->GetWindow(id);
    if (!windowHandle.Valid())
        return;

    _selectedWindowId = id;
    _selectedComponentId = 0;
    _rootNodeLoaded = false;
    _rootViewGot = false;
    _UpdateNodes(nullptr, _rootNode, 0);
    _uiDebugHookEventSubscription = windowHandle->SubscribeToUIDebugHookEvents([=](zwnd::Window* window) {
        _OnUIDebugHookEvent(window);
    });
    _uiDebugRenderCheckHookEventSubscription = windowHandle->SubscribeToUIDebugRenderCheckHookEvents([=](zwnd::Window* window, bool* redraw) {
        _OnUIDebugRenderCheckHookEvent(window, redraw);
    });
    _uiDebugRenderHookEventSubscription = windowHandle->SubscribeToUIDebugRenderHookEvents([=](zwnd::Window* window, Graphics* g) {
        _OnUIDebugRenderHookEvent(window, g);
    });
}

void zcom::DebugWindowScene::_SelectComponent(uint64_t id)
{
    _selectedComponentId = id;
}

void zcom::DebugWindowScene::_OnUIDebugHookEvent(zwnd::Window* window)
{
    std::unique_lock<std::mutex> lock(_m_uiView);
    _rootViewGot = true;
    _BuildUIView(window->GetNonClientAreaScene()->GetBasePanel(), _UIRootView);
    _BuildComponentDataView(window->GetNonClientAreaScene()->GetBasePanel());
}

void zcom::DebugWindowScene::_OnUIDebugRenderCheckHookEvent(zwnd::Window* window, bool* redraw)
{
    uint64_t hoveredComponentId = _hoveredComponentId.load();
    if (hoveredComponentId != 0 || _hoverEnded.exchange(false))
        *redraw = true;
}

void zcom::DebugWindowScene::_OnUIDebugRenderHookEvent(zwnd::Window* window, Graphics* g)
{
    uint64_t hoveredComponentId = _hoveredComponentId.load();
    if (hoveredComponentId == 0)
        return;

    // Highlight the hovered component

    auto children = window->GetNonClientAreaScene()->GetBasePanel()->GetAllChildren();
    for (auto& child : children)
    {
        if (child->GetId() == hoveredComponentId)
        {
            // Target is entire window, so child->windowPosition_ doesn't need any mapping
            Size windowSize = g->GetTargetSize();
            RectF componentRect{};
            componentRect.left = float(child->windowPosition_->x);
            componentRect.top = float(child->windowPosition_->y);
            componentRect.right = float(componentRect.left + child->size_->width);
            componentRect.bottom = float(componentRect.top + child->size_->height);

            StrokeStyle strokeStyle{};
            strokeStyle.dashes = { 4.0f, 2.0f };

            g->FillRectangle(componentRect, Color(0x00A0FF, 0.15f));
            g->DrawLine({ componentRect.left + 0.5f, 0.0f }, { componentRect.left + 0.5f, (float)windowSize.height }, Color(0x00A0FF, 0.5f), 1.0f, strokeStyle); // Left vertical
            g->DrawLine({ componentRect.right - 0.5f, 0.0f }, { componentRect.right - 0.5f, (float)windowSize.height }, Color(0x00A0FF, 0.5f), 1.0f, strokeStyle); // Right vertical
            g->DrawLine({ 0.0f, componentRect.top + 0.5f }, { (float)windowSize.width, componentRect.top + 0.5f }, Color(0x00A0FF, 0.5f), 1.0f, strokeStyle); // Top horizontal
            g->DrawLine({ 0.0f, componentRect.bottom - 0.5f }, { (float)windowSize.width, componentRect.bottom - 0.5f }, Color(0x00A0FF, 0.5f), 1.0f, strokeStyle); // Bottom horizontal

            break;
        }
    }
}

void zcom::DebugWindowScene::_UpdateInspectorPanel()
{
    _noValuesLabel->visible = _selectedComponentId.Get() == 0;
    if (_selectedWindowId == zwnd::WindowId() || !_rootViewGot)
        return;

    std::unique_lock<std::mutex> lock(_m_uiView);
    ComponentView viewCopy = _UIRootView;
    std::vector<std::pair<std::string, std::vector<PropertyView>>> propCategoriesCopy = _UIPropertyCategoryViews;
    lock.unlock();

    _inspectorPanel->DeferLayoutUpdates();

    if (!_rootNodeLoaded)
    {
        _rootNode = _CreateNode(viewCopy.id, viewCopy.name, 0);
        _inspectorPanel->AddItem(_rootNode.nodePanel.get());
        _rootNodeLoaded = true;
    }

    _UpdateNodes(&viewCopy, _rootNode, 0);
    _PositionNodes(_rootNode, 0, false);

    _inspectorPanel->ResumeLayoutUpdates();

    _valueEditorPanel->DeferLayoutUpdates();
    _UpdateProperties(propCategoriesCopy);
    _PositionProperties(propCategoriesCopy);
    _valueEditorPanel->ResumeLayoutUpdates(true, true);
}

void zcom::DebugWindowScene::_BuildUIView(Component* component, ComponentView& view)
{
    view.id = component->GetId();
    view.name = component->GetName();
    auto children = component->GetChildren();
    view.children.clear();
    view.children.reserve(children.size());
    for (auto& item : children)
    {
        ComponentView childView;
        _BuildUIView(item, childView);
        view.children.push_back(std::move(childView));
    }
}

void zcom::DebugWindowScene::_BuildComponentDataView(Component* component)
{
    _UIPropertyCategoryViews.clear();
    if (_selectedComponentId.Get() == 0)
        return;

    auto children = component->GetAllChildren();
    for (auto& child : children)
    {
        if (child->GetId() == _selectedComponentId)
        {
            std::wostringstream wss(L"");
            auto data = child->GetReflectionData();
            _ApplyChangesToProperties(data);
            for (auto& propertyList : data)
            {
                std::vector<PropertyView> views;
                for (auto& property : propertyList.second)
                {
                    PropertyView view;
                    view.name = property.name;
                    view.type = property.type;
                    view.optional = property.optional;
                    if (property.type == ValueProxy::TEXT)
                    {
                        view.text = property.textMapper->fromValue(property.valuePtr);
                    }
                    else if (property.type == ValueProxy::NUMBER)
                    {
                        view.number = property.numberMapper->fromValue(property.valuePtr);
                        view.numberPrecision = property.numberPrecision;
                        view.numberMinValue = property.numberMinValue;
                        view.numberMaxValue = property.numberMaxValue;
                        view.numberStepSize = property.numberStepSize;
                    }
                    else if (property.type == ValueProxy::SELECTION)
                    {
                        view.selection = property.selectionMapper->fromValue(property.valuePtr);
                        view.selectionValues = property.selectionValues;
                        view.dropdownAbove = property.dropdownAbove;
                    }
                    else if (property.type == ValueProxy::BOOL)
                    {
                        view.boolState = property.boolMapper->fromValue(property.valuePtr);
                    }
                    else if (property.type == ValueProxy::COLOR)
                    {
                        view.color = property.colorMapper->fromValue(property.valuePtr);
                    }
                    views.push_back(std::move(view));
                }
                _UIPropertyCategoryViews.push_back({ propertyList.first, std::move(views) });
            }
            return;
        }
    }
}

void zcom::DebugWindowScene::_ApplyChangesToProperties(std::vector<std::pair<std::string, std::vector<ValueProxy>>>& reflectionData)
{
    for (auto& category : _categories)
    {
        for (auto& property : category.properties)
        {
            if (property.updated)
            {
                for (auto& category_ : reflectionData)
                {
                    if (category.name != category_.first)
                        continue;

                    for (auto& property_ : category_.second)
                    {
                        if (property.name != property_.name || property.type != property_.type)
                            continue;

                        if (property.type == ValueProxy::TEXT)
                        {
                            if (property.enableCheckbox && !property.enableCheckbox->checked)
                                property_.textMapper->toValue(std::nullopt, property_.valuePtr);
                            else
                                property_.textMapper->toValue(property.textInput->text, property_.valuePtr);
                        }
                        else if (property.type == ValueProxy::NUMBER)
                        {
                            if (property.enableCheckbox && !property.enableCheckbox->checked)
                                property_.numberMapper->toValue(std::nullopt, property_.valuePtr);
                            else
                                property_.numberMapper->toValue(property.numberInput->value, property_.valuePtr);
                        }
                        else if (property.type == ValueProxy::SELECTION)
                        {
                            if (property.enableCheckbox && !property.enableCheckbox->checked)
                                property_.selectionMapper->toValue(std::nullopt, property_.valuePtr);
                            else
                                property_.selectionMapper->toValue(property.selectionInput->selectedItemId, property_.valuePtr);
                        }
                        else if (property.type == ValueProxy::BOOL)
                        {
                            if (property.enableCheckbox && !property.enableCheckbox->checked)
                                property_.boolMapper->toValue(std::nullopt, property_.valuePtr);
                            else
                                property_.boolMapper->toValue(property.boolInput->checked, property_.valuePtr);
                        }
                        else if (property.type == ValueProxy::COLOR)
                        {
                            if (property.enableCheckbox && !property.enableCheckbox->checked)
                                property_.colorMapper->toValue(std::nullopt, property_.valuePtr);
                            else
                                property_.colorMapper->toValue(property.colorInput->color, property_.valuePtr);
                        }
                    }
                }
                property.updated = false;
            }
        }
    }
}

void zcom::DebugWindowScene::_UpdateNodes(ComponentView* component, ComponentNode& node, int depth)
{
    if (component == nullptr)
    {
        for (auto& n : node.nodes)
            _UpdateNodes(nullptr, n, depth + 1);
        node.nodes.clear();
        _inspectorPanel->RemoveItem(node.nodePanel.get());
        return;
    }

    auto& children = component->children;

    // Remove excess nodes
    for (int i = 0; i < node.nodes.size(); i++)
    {
        if (std::find_if(children.begin(), children.end(), [&](const ComponentView& c) { return c.id == node.nodes[i].componentId; }) == children.end())
        {
            _UpdateNodes(nullptr, node.nodes[i], depth + 1);
            node.nodes.erase(node.nodes.begin() + i);
            i--;
        }
    }
    // Rearrange nodes, add missing ones, and update them
    for (int i = 0; i < children.size(); i++)
    {
        int nodeIndex = -1;
        for (int j = i; j < node.nodes.size(); j++)
        {
            if (node.nodes[j].componentId == children[i].id)
            {
                nodeIndex = j;
                break;
            }
        }

        if (nodeIndex == -1)
        {
            ComponentNode newNode = _CreateNode(children[i].id, children[i].name, depth + 1);
            _inspectorPanel->AddItem(newNode.nodePanel.get());
            node.nodes.insert(node.nodes.begin() + i, std::move(newNode));
        }
        else if (nodeIndex != i)
        {
            std::swap(node.nodes[i], node.nodes[nodeIndex]);
        }
        _UpdateNodes(&children[i], node.nodes[i], depth + 1);
    }
}

zcom::DebugWindowScene::ComponentNode zcom::DebugWindowScene::_CreateNode(uint64_t componentId, std::string componentName, int depth)
{
    ComponentNode newNode;
    newNode.componentId = componentId;
    newNode.expanded = std::make_unique<Value<bool>>();
    (*newNode.expanded) = false;
    auto nodePanel = Create<Panel>();
    nodePanel->parentSize = { 1.0f, 0.0f };
    nodePanel->size = { 0, 22 };
    nodePanel->backgroundColor.ComputedFrom([componentId](bool hovered, uint64_t selectedComponentId) {
        if (selectedComponentId == componentId)
            return Color(0x00A0FF, 0.15f);
        else if (hovered)
            return Color(0xFFFFFF, 0.1f);
        else
            return Color();
    }, nodePanel->hovered_, _selectedComponentId);
    nodePanel->SubscribePostLeftPressed([=, id = newNode.componentId](Component*, std::vector<EventContext::Params> params, Point) {
        // Only select component if a button (expand/collapse) isn't clicked
        if (std::find_if(params.begin(), params.end(), [](EventContext::Params& target) { return target.target->GetName() == Button::Name(); }) == params.end())
            _SelectComponent(id);
    }).Detach();
    nodePanel->SubscribeOnMouseEnter([=, id = newNode.componentId](Component*) {
        _hoveredComponentId.store(id);
    }).Detach();
    nodePanel->SubscribeOnMouseLeave([=, id = newNode.componentId](Component*) {
        uint64_t currentId = _hoveredComponentId.load();
        if (currentId == id)
        {
            _hoveredComponentId.store(0);
            _hoverEnded.store(true);
        }
    }).Detach();
    auto expandButton = Create<Button>(L"", ButtonPreset::NO_EFFECTS);
    expandButton->size = { 15, 22 };
    expandButton->position = { 15 * depth, 0 };
    expandButton->activation = ButtonActivation::PRESS;
    expandButton->border.selectedColor = Color();
    expandButton->Image()->imagePlacement = ImagePlacement::CENTER;
    expandButton->Image()->snapToPixels = true;
    auto rightArrowBitmap = _window->resourceManager.GetImage("menu_arrow_right_7x7");
    auto downArrowBitmap = _window->resourceManager.GetImage("menu_arrow_down_7x7");
    expandButton->Image()->image.ComputedFrom([rightArrowBitmap, downArrowBitmap](bool expanded) {
        return expanded ? downArrowBitmap : rightArrowBitmap;
    }, *(newNode.expanded));
    expandButton->ValueFromButtonState<Color>(expandButton->Image()->tintColor, Color(0xD0D0D0), Color(0x0070DD), Color(0x0070DD));
    expandButton->SubscribeOnActivated([=, expandedValue = newNode.expanded.get()]() {
        if (!expandedValue->Get())
            _ExpandNode(_rootNode, componentId);
        else
            *expandedValue = false;
    }).Detach();
    auto nameLabel = Create<Label>(string_to_wstring(componentName));
    nameLabel->parentSize = { 1.0f, 0.0f };
    nameLabel->size = { -15 - (15 * depth), 22 };
    nameLabel->padding = { 5.0f, 0, 0, 0 };
    nameLabel->xAlign = Alignment::END;
    nameLabel->yTextAlign = Alignment::CENTER;

    nodePanel->AddItem(expandButton.get());
    nodePanel->AddItem(nameLabel.get());
    newNode.expandButton = std::move(expandButton);
    newNode.nameLabel = std::move(nameLabel);
    newNode.nodePanel = std::move(nodePanel);
    return newNode;
}

bool zcom::DebugWindowScene::_ExpandNode(ComponentNode& node, uint64_t componentId)
{
    if (node.componentId == componentId)
    {
        ComponentNode* currentNode = &node;
        while (true)
        {
            (*currentNode->expanded) = true;
            if (currentNode->nodes.size() == 1)
            {
                currentNode = &currentNode->nodes[0];
                continue;
            }
            break;
        }
        return true;
    }

    for (auto& n : node.nodes)
    {
        if (_ExpandNode(n, componentId))
            return true;
    }
    return false;
}

int zcom::DebugWindowScene::_PositionNodes(ComponentNode& node, int offset, bool hidden)
{
    int nodeHeight = 0;
    if (!hidden)
    {
        node.nodePanel->position.Assign(Y(offset + nodeHeight));
        nodeHeight += node.nodePanel->size_->height;
        node.expandButton->visible = node.nodes.size() > 0;
    }
    node.nodePanel->visible = !hidden;

    for (auto& n : node.nodes)
    {
        nodeHeight += _PositionNodes(n, offset + nodeHeight, !node.expanded->Get() || hidden);
    }
    return nodeHeight;
}

void zcom::DebugWindowScene::_UpdateProperties(std::vector<std::pair<std::string, std::vector<PropertyView>>>& categoryViews)
{
    // Match categories
    for (auto it = _categories.begin(); it != _categories.end();)
    {
        if (std::find_if(categoryViews.begin(), categoryViews.end(), [&](std::pair<std::string, std::vector<PropertyView>>& pair) { return pair.first == it->name; }) == categoryViews.end())
        {
            it = _categories.erase(it);
            _categoriesChanged = true;
        }
        else
            it++;
    }
    for (int i = 0; i < categoryViews.size(); i++)
    {
        int categoryIndex = -1;
        for (int j = i; j < _categories.size(); j++)
        {
            if (_categories[j].name == categoryViews[i].first)
            {
                categoryIndex = j;
                break;
            }
        }

        if (categoryIndex == -1)
        {
            PropertyCategory category;
            category.name = categoryViews[i].first;
            category.label = Create<Label>(string_to_wstring(categoryViews[i].first));
            category.label->parentSize = { 1.0f, 0.0f };
            category.label->size = { 0, 26 };
            category.label->padding = { 5.0f, 0.0f, 5.0f, 0.0f };
            category.label->yTextAlign = Alignment::CENTER;
            category.label->fontSize = 18.0f;
            category.panel = Create<Panel>();
            category.panel->parentSize = { 1.0f, 0.0f };
            _categories.insert(_categories.begin() + i, std::move(category));
            _categoriesChanged = true;
        }
        else if (categoryIndex != i)
        {
            std::swap(_categories[i], _categories[categoryIndex]);
            _categoriesChanged = true;
        }
    }

    // Match each category properties
    for (int index = 0; index < _categories.size(); index++)
    {
        std::vector<Property>& properties = _categories[index].properties;
        std::vector<PropertyView>& views = categoryViews[index].second;

        for (auto it = properties.begin(); it != properties.end();)
        {
            if (std::find_if(views.begin(), views.end(), [&](PropertyView& view) { return view.name == it->name; }) == views.end())
                it = properties.erase(it);
            else
                it++;
        }
        for (int i = 0; i < views.size(); i++)
        {
            int propIndex = -1;
            for (int j = i; j < properties.size(); j++)
            {
                if (properties[j].name == views[i].name)
                {
                    propIndex = j;
                    break;
                }
            }

            if (propIndex == -1)
            {
                Property prop;
                prop.type = views[i].type;
                prop.name = views[i].name;
                prop.propertyPanel = Create<FlexPanel>(FlexDirection::RIGHT);
                prop.propertyPanel->parentSize = { 1.0f, 0.0f };
                prop.propertyPanel->size = { 0, 20 };
                prop.propertyPanel->spacing = 5;
                prop.propertyPanel->padding = { 5, 0, 5, 0 };
                prop.propertyPanel->itemAlignment = Alignment::CENTER;
                if (views[i].optional)
                {
                    prop.enableCheckbox = Create<Checkbox>();
                    prop.enableCheckbox->size = { 18, 18 };
                    prop.enableCheckbox->backgroundColor = Color(0x101010);
                    prop.enableCheckbox->border.cornerRadius = 2.0f;
                    prop.enableCheckbox->checked = false;
                    prop.enableCheckbox->SubscribeOnStateChanged([=, category = _categories[index].name, property = prop.name](bool) { _OnPropertyUpdated(category, property); }).Detach();
                    prop.propertyPanel->AddItem(prop.enableCheckbox.get());
                }
                prop.nameLabel = Create<Label>(string_to_wstring(prop.name));
                prop.nameLabel->parentSize = { 0.0f, 1.0f };
                prop.nameLabel->SetProperty(FlexGrow());
                prop.nameLabel->yTextAlign = Alignment::CENTER;
                if (views[i].optional)
                    prop.nameLabel->disabled.ComputedFrom([](bool enabled) { return !enabled; }, prop.enableCheckbox->checked);
                prop.propertyPanel->AddItem(prop.nameLabel.get());
                if (prop.type == ValueProxy::TEXT)
                {
                    prop.textInput = Create<TextInput>();
                    prop.textInput->size = { 100, 20 };
                    prop.textInput->backgroundColor = Color(0x101010);
                    prop.textInput->border.cornerRadius = 2.0f;
                    if (views[i].optional)
                        prop.textInput->disabled.ComputedFrom([](bool enabled) { return !enabled; }, prop.enableCheckbox->checked);
                    prop.textInput->SubscribeOnSelected([=, category = _categories[index].name, property = prop.name](Component*, bool) { _OnPropertySelected(category, property); }).Detach();
                    prop.textInput->SubscribeOnDeselected([=, category = _categories[index].name, property = prop.name](Component*) { _OnPropertyDeselected(category, property); }).Detach();
                    prop.textInput->SubscribeOnTextChanged([=, category = _categories[index].name, property = prop.name](std::wstring*) { _OnPropertyUpdated(category, property); }).Detach();
                    prop.propertyPanel->AddItem(prop.textInput.get());
                }
                else if (prop.type == ValueProxy::NUMBER)
                {
                    prop.numberInput = Create<NumberInput>();
                    prop.numberInput->size = { 70, 20 };
                    prop.numberInput->backgroundColor = Color(0x101010);
                    prop.numberInput->border.cornerRadius = 2.0f;
                    prop.numberInput->arrowImageGap = 0.0f;
                    prop.numberInput->precision = views[i].numberPrecision;
                    prop.numberInput->minValue = views[i].numberMinValue;
                    prop.numberInput->maxValue = views[i].numberMaxValue;
                    prop.numberInput->stepSize = views[i].numberStepSize;
                    if (views[i].optional)
                        prop.numberInput->disabled.ComputedFrom([](bool enabled) { return !enabled; }, prop.enableCheckbox->checked);
                    prop.numberInput->SubscribeOnSelected([=, category = _categories[index].name, property = prop.name](Component*, bool) { _OnPropertySelected(category, property); }).Detach();
                    prop.numberInput->SubscribeOnDeselected([=, category = _categories[index].name, property = prop.name](Component*) { _OnPropertyDeselected(category, property); }).Detach();
                    prop.numberInput->SubscribeOnValueChanged([=, category = _categories[index].name, property = prop.name](NumberInputValue) { _OnPropertyUpdated(category, property); }).Detach();
                    prop.propertyPanel->AddItem(prop.numberInput.get());
                }
                else if (prop.type == ValueProxy::SELECTION)
                {
                    prop.selectionInput = Create<DropdownSelector>();
                    prop.selectionInput->size = { 100, 20 };
                    prop.selectionInput->backgroundColor = Color(0x101010);
                    prop.selectionInput->border.cornerRadius = 2.0f;
                    std::vector<DropdownItem> dropdownItems;
                    for (auto& item : views[i].selectionValues)
                        dropdownItems.push_back({ item.first, item.second });
                    prop.selectionInput->SetDropdownItems(dropdownItems);
                    prop.selectionInput->dropdownPlacement = views[i].dropdownAbove ? DropdownPlacement::ABOVE : DropdownPlacement::BELOW;
                    if (views[i].optional)
                        prop.selectionInput->disabled.ComputedFrom([](bool enabled) { return !enabled; }, prop.enableCheckbox->checked);
                    prop.selectionInput->SubscribeOnSelected([=, category = _categories[index].name, property = prop.name](Component*, bool) { _OnPropertySelected(category, property); }).Detach();
                    prop.selectionInput->SubscribeOnDeselected([=, category = _categories[index].name, property = prop.name](Component*) { _OnPropertyDeselected(category, property); }).Detach();
                    prop.selectionInput->SubscribeOnValueSelected([=, category = _categories[index].name, property = prop.name](std::optional<int64_t>) { _OnPropertyUpdated(category, property); }).Detach();
                    prop.propertyPanel->AddItem(prop.selectionInput.get());
                }
                else if (prop.type == ValueProxy::BOOL)
                {
                    prop.boolInput = Create<Checkbox>(views[i].boolState.value());
                    prop.boolInput->size = { 18, 18 };
                    prop.boolInput->backgroundColor = Color(0x101010);
                    prop.boolInput->border.cornerRadius = 2.0f;
                    if (views[i].optional)
                        prop.boolInput->disabled.ComputedFrom([](bool enabled) { return !enabled; }, prop.enableCheckbox->checked);
                    prop.boolInput->SubscribeOnStateChanged([=, category = _categories[index].name, property = prop.name](bool) { _OnPropertyUpdated(category, property); }).Detach();
                    prop.propertyPanel->AddItem(prop.boolInput.get());
                }
                else if (prop.type == ValueProxy::COLOR)
                {
                    prop.colorInput = Create<ColorSelector>();
                    prop.colorInput->size = { 50, 20 };
                    prop.colorInput->border.cornerRadius = 2.0f;
                    if (views[i].optional)
                        prop.colorInput->disabled.ComputedFrom([](bool enabled) { return !enabled; }, prop.enableCheckbox->checked);
                    prop.colorInput->SubscribeOnColorChanged([=, category = _categories[index].name, property = prop.name](Color) { _OnPropertyUpdated(category, property); }).Detach();
                    prop.propertyPanel->AddItem(prop.colorInput.get());
                }
                
                _categories[index].panel->AddItem(prop.propertyPanel.get());
                properties.insert(properties.begin() + i, std::move(prop));
            }
            else if (propIndex != i)
            {
                std::swap(properties[i], properties[propIndex]);
            }

            if (!properties[i].editing && !properties[i].updated)
            {
                if (properties[i].type == ValueProxy::TEXT)
                {
                    if (views[i].text.has_value())
                        properties[i].textInput->text = views[i].text.value();
                    if (views[i].optional)
                        properties[i].enableCheckbox->checked = views[i].text.has_value();
                }
                else if (properties[i].type == ValueProxy::NUMBER)
                {
                    properties[i].numberInput->precision = views[i].numberPrecision;
                    properties[i].numberInput->minValue = views[i].numberMinValue;
                    properties[i].numberInput->maxValue = views[i].numberMaxValue;
                    properties[i].numberInput->stepSize = views[i].numberStepSize;
                    if (views[i].number.has_value())
                        properties[i].numberInput->value = views[i].number.value();
                    if (views[i].optional)
                        properties[i].enableCheckbox->checked = views[i].number.has_value();
                }
                else if (properties[i].type == ValueProxy::SELECTION)
                {
                    std::vector<DropdownItem> dropdownItems;
                    for (auto& item : views[i].selectionValues)
                        dropdownItems.push_back({ item.first, item.second });
                    properties[i].selectionInput->SetDropdownItems(dropdownItems);

                    if (views[i].selection.has_value())
                    {
                        properties[i].selectionInput->selectedItemId = views[i].selection.value();
                        for (auto& item : views[i].selectionValues)
                        {
                            if (item.first == views[i].selection.value())
                            {
                                properties[i].selectionInput->text = item.second;
                                break;
                            }
                        }
                    }
                    if (views[i].optional)
                        properties[i].enableCheckbox->checked = views[i].selection.has_value();
                }
                else if (properties[i].type == ValueProxy::BOOL)
                {
                    if (views[i].boolState.has_value())
                        properties[i].boolInput->checked = views[i].boolState.value();
                    if (views[i].optional)
                        properties[i].enableCheckbox->checked = views[i].boolState.has_value();
                }
                else if (properties[i].type == ValueProxy::COLOR)
                {
                    if (views[i].color.has_value())
                        properties[i].colorInput->color = views[i].color.value();
                    if (views[i].optional)
                        properties[i].enableCheckbox->checked = views[i].color.has_value();
                }
            }
        }
    }
}

void zcom::DebugWindowScene::_PositionProperties(std::vector<std::pair<std::string, std::vector<PropertyView>>>& categoryViews)
{
    _valueEditorItems->DeferLayoutUpdates();
    if (_categoriesChanged)
        _valueEditorItems->ClearItems();
    for (auto& category : _categories)
    {
        category.panel->DeferLayoutUpdates();
        int size = 0;
        for (auto& property : category.properties)
        {
            property.propertyPanel->position = { 0, size };
            size += property.propertyPanel->size_->height + 1;
        }
        category.panel->size = { -10, size + 4 };
        category.panel->position = { 10, 0 };
        category.panel->ResumeLayoutUpdates();

        if (_categoriesChanged)
        {
            _valueEditorItems->AddItem(category.label.get());
            _valueEditorItems->AddItem(category.panel.get());
        }
    }
    _valueEditorItems->ResumeLayoutUpdates();
    _categoriesChanged = false;
}

void zcom::DebugWindowScene::_OnPropertySelected(std::string category, std::string name)
{
    for (auto& cat : _categories)
    {
        if (cat.name != category)
            continue;

        for (auto& prop : cat.properties)
        {
            if (prop.name == name)
            {
                prop.editing = true;
                return;
            }
        }
    }
}

void zcom::DebugWindowScene::_OnPropertyDeselected(std::string category, std::string name)
{
    for (auto& cat : _categories)
    {
        if (cat.name != category)
            continue;

        for (auto& prop : cat.properties)
        {
            if (prop.name == name)
            {
                prop.editing = false;
                prop.updated = true;
                return;
            }
        }
    }
}

void zcom::DebugWindowScene::_OnPropertyUpdated(std::string category, std::string name)
{
    for (auto& cat : _categories)
    {
        if (cat.name != category)
            continue;

        for (auto& prop : cat.properties)
        {
            if (prop.name == name)
            {
                prop.updated = true;
                return;
            }
        }
    }
}
