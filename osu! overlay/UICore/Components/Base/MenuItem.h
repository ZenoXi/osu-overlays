#pragma once

#include "FlexPanel.h"
#include "Label.h"
#include "Image.h"
#include "MenuTemplate.h"
#include "Helper/ResourceManager.h"

namespace zcom
{
    class MenuPanel;

    class MenuItem : public FlexPanel
    {
        DEFINE_COMPONENT(MenuItem, FlexPanel)
        DEFAULT_DESTRUCTOR(MenuItem)
        HIDE_PANEL_METHODS
    protected:
        // Separator
        void Init();
        // Regular button/check item
        void Init(std::wstring text, std::function<void(bool)> onClick = [](bool) {});
        // Deeper menu
        void Init(MenuTemplate::Menu menu, std::wstring text);

    public:
        struct Id
        {
            uint64_t value = 0;

            bool operator==(const Id& other) const
            {
                return value == other.value;
            }
            bool operator!=(const Id& other) const
            {
                return value != other.value;
            }
        };

        Id GetId() const
        {
            return _id;
        }
        
        Value<int> minWidth = Value<int>(70, [=](int& currentValue, const int& minWidth) {
            currentValue = minWidth;
            if (_label)
                _label->minAutoWidth = minWidth - 50;
        });
        Value<int> maxWidth = Value<int>(600, [=](int& currentValue, const int& maxWidth) {
            currentValue = maxWidth;
            if (_label)
                _label->maxAutoWidth = maxWidth - 50;
        });
        Value<bool> closeOnClick = true;

        // Exactly 1 item from a check group must be checked at a time
        Value<int> checkGroup = -1;
        Value<bool> checkable = false;
        Value<bool> checked = false;
        
        Value<std::optional<Bitmap>> icon = Value<std::optional<Bitmap>>(std::optional<Bitmap>(std::nullopt), [=](std::optional<Bitmap>& currentValue, std::optional<Bitmap> const& bitmap) {
            currentValue = bitmap;
            if (_iconGrayscale.Get())
            {
                SafeFullRelease((IUnknown**)&_iconGrayscale.Get());
                _iconGrayscale.NotifyChange();
            }
        });

        const std::optional<MenuTemplate::Menu>& GetMenu() const
        {
            return _menu;
        }

        // For non-checkable items, 'checked' will always be true
        void Invoke(bool checked = true)
        {
            if (_onClick)
                _onClick(checked);
        }

        bool IsSeparator() const
        {
            return _separator;
        }

    private:
        Id _id;

        std::optional<MenuTemplate::Menu> _menu = std::nullopt;
        std::function<void(bool)> _onClick;
        bool _separator = false;

        Value<std::optional<Bitmap>> _iconGrayscale = std::optional<Bitmap>(std::nullopt);
        Value<std::optional<Bitmap>> _checkmarkIcon = std::optional<Bitmap>(std::nullopt);

        std::unique_ptr<Label> _label = nullptr;
        std::unique_ptr<zcom::Image> _iconImage = nullptr;
        std::unique_ptr<zcom::Image> _menuExpandImage = nullptr;

    protected:
        void _OnDraw(Graphics* g) override
        {
            // Create grayscale version of _icon
            if (icon.Get() && !_iconGrayscale->has_value())
            {
                auto size = icon.Get()->GetSize();

                // TODO: possible that icon also is allocated on AUX3 - needs fixing
                _iconGrayscale = g->CreateBitmap((int)size.width, (int)size.height, SEGMENT_POOL_AUX3);
                if (_iconGrayscale.Get())
                {
                    BitmapSourceEffect sourceEffect = BitmapSourceEffect(&icon->value());
                    GrayscaleEffect grayscaleEffect = GrayscaleEffect(&sourceEffect);
                    BrightnessEffect brightnessEffect = BrightnessEffect(&grayscaleEffect, 0.6f);

                    g->PushAndClearTarget(_iconGrayscale->value());
                    g->DrawEffect(&brightnessEffect);
                    g->PopTarget();
                }
            }

            FlexPanel::_OnDraw(g);
        }

        void _GenerateId()
        {
            static uint64_t id{ 1 };
            _id = { id++ };
        }

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicIntValueProxy<int>("min width", std::make_any<Value<int>*>(&minWidth), ValueProxy::Number(0)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("max width", std::make_any<Value<int>*>(&maxWidth), ValueProxy::Number(0)));
            values.push_back(ValueProxy::BasicBoolValueProxy("close on click", std::make_any<Value<bool>*>(&closeOnClick)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("check group", std::make_any<Value<int>*>(&checkGroup)));
            values.push_back(ValueProxy::BasicBoolValueProxy("checkable", std::make_any<Value<bool>*>(&checkable)));
            values.push_back(ValueProxy::BasicBoolValueProxy("checked", std::make_any<Value<bool>*>(&checked)));

            auto data = FlexPanel::GetReflectionData();
            data.insert(data.begin(), { "Menu item", std::move(values) });
            return data;
        }
    };
}