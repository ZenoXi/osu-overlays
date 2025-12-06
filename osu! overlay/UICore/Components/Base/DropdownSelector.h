#pragma once

#include "TextInput.h"
#include "Image.h"
#include "Label.h"
#include "ScrollPanel.h"
#include "FlexPanel.h"

namespace zcom
{
    enum class DropdownPlacement
    {
        BELOW,
        ABOVE
    };
    constexpr std::vector<std::pair<int64_t, std::wstring>> DropdownPlacementValueProxySelectionValues()
    {
        return {
            { (int64_t)DropdownPlacement::BELOW, L"Below" },
            { (int64_t)DropdownPlacement::ABOVE, L"Above" }
        };
    }

    struct DropdownItem
    {
        int64_t id;
        std::wstring text;
    };

    class DropdownSelector : public TextInput
    {
        DEFINE_COMPONENT(DropdownSelector, TextInput)
    public:
        ~DropdownSelector();
    protected:
        void Init();

    public:
        Value<DropdownPlacement> dropdownPlacement = DropdownPlacement::BELOW;
        Value<int> maxListHeight = 200;

        Value<std::optional<int64_t>> selectedItemId = std::optional<int64_t>();

        // Returns a pointer to the parent panel containing the dropdown
        // Do not remove the contained panel, as doing so is almost guaranteed to cause a crash
        FlexPanel* GetDropdownPanel() const { return _dropdownPanel.get(); }
        // Returns a pointer to the panel containing selectable dropdown items
        // NOTE: Height portion of the 'size' property shouldn't be modified as it is computed from other variables
        // If some specific height behaviour is required, the existing 'ComputedFrom' handler needs to be overwritten
        ScrollPanel* GetDropdownItemsPanel() const { return _dropdownScrollPanel.get(); }
        // Returns a label that is shown when no selections are available in the dropdown
        Label* GetNoResultsLabel() const { return _noResultsLabel.get(); }

        void SetDropdownItems(const std::vector<DropdownItem>& items)
        {
            _baseDropdownItems = items;
        }
        void SetSearchMapper(const std::function<std::vector<DropdownItem>(DropdownSelector*, std::wstring, std::vector<DropdownItem>)>& mapper)
        {
            _searchMapper = mapper;
        }

        [[nodiscard]]
        EventSubscription<void, std::optional<int64_t>> SubscribeOnValueSelected(std::function<void(std::optional<int64_t>)> handler)
        {
            return _valueSelectedEventEmitter->Subscribe(handler);
        }

    private:
        class DropdownItemComponent : public Label
        {
            DEFINE_COMPONENT(DropdownItemComponent, Label)
            DEFAULT_DESTRUCTOR(DropdownItemComponent)
        protected:
            void Init() { Label::Init(); }
        public:
            Value<int64_t> itemId;

        public:
            std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
            {
                std::vector<ValueProxy> values;
                values.push_back(ValueProxy::BasicIntValueProxy<int64_t>("item id", std::make_any<Value<int64_t>*>(&itemId)));

                auto data = Label::GetReflectionData();
                data.insert(data.begin(), { "Dropdown item", std::move(values) });
                return data;
            }
        };

        std::unique_ptr<FlexPanel> _dropdownPanel;
        std::unique_ptr<ScrollPanel> _dropdownScrollPanel;
        std::unique_ptr<FlexPanel> _dropdownItemsPanel;
        std::unique_ptr<Label> _noResultsLabel;

        std::vector<DropdownItem> _baseDropdownItems;
        std::function<std::vector<DropdownItem>(DropdownSelector*, std::wstring, std::vector<DropdownItem>)> _searchMapper;

        std::optional<DropdownItem> _initialValue = std::nullopt;
        bool _internalChange = false;
        EventEmitter<void, std::optional<int64_t>> _valueSelectedEventEmitter;

    private:
        void _DoSearch(const std::wstring& searchString);

    protected:
        EventContext _OnLeftPressed(Point point) override;
        void _OnSelected(bool reverse) override;
        void _OnDeselected() override;

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicEnumValueProxy<DropdownPlacement>("dropdown placement", std::make_any<Value<DropdownPlacement>*>(&dropdownPlacement), DropdownPlacementValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("max list height", std::make_any<Value<int>*>(&maxListHeight)));
            values.push_back(ValueProxy::BasicOptionalIntValueProxy<int64_t>("selected item id", std::make_any<Value<std::optional<int64_t>>*>(&selectedItemId)));

            auto data = TextInput::GetReflectionData();
            data.insert(data.begin(), { "Dropdown selector", std::move(values) });
            return data;
        }
    };
}