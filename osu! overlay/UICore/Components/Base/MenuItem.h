#pragma once

#include "Label.h"
#include "Image.h"
#include "MenuTemplate.h"
#include "Helper/ResourceManager.h"

namespace zcom
{
    class MenuPanel;

    class MenuItem : public Component
    {
    public:
        struct Id
        {
            uint64_t value = 0;

            bool operator==(const Id& other)
            {
                return value == other.value;
            }
            bool operator!=(const Id& other)
            {
                return value != other.value;
            }
        };

#pragma region base_class
    protected:
        void _OnDraw(Graphics g) override
        {
            // Create grayscale version of _icon
            if (_icon && !_iconGrayscale)
            {
                auto size = _icon->GetSize();

                // Draw to separate bitmap and apply grayscale and brightness effects
                ID2D1Image* stash = nullptr;
                g.target->CreateBitmap(
                    D2D1::SizeU(size.width, size.height),
                    nullptr,
                    0,
                    D2D1::BitmapProperties1(
                        D2D1_BITMAP_OPTIONS_TARGET,
                        { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
                    ),
                    &_iconGrayscale
                );
                g.refs->push_back({ (IUnknown**)&_iconGrayscale, std::string("Grayscale menu icon. Text: ") + wstring_to_string(_label->GetText()) });

                ID2D1Effect* grayscaleEffect;
                g.target->CreateEffect(CLSID_D2D1Grayscale, &grayscaleEffect);
                grayscaleEffect->SetInput(0, _icon);
                ID2D1Effect* brightnessEffect;
                g.target->CreateEffect(CLSID_D2D1Brightness, &brightnessEffect);
                brightnessEffect->SetInputEffect(0, grayscaleEffect);
                brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_WHITE_POINT, D2D1::Vector2F(1.0f, 0.6f));
                brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(1.0f, 0.6f));

                g.target->GetTarget(&stash);
                g.target->SetTarget(_iconGrayscale);
                g.target->Clear();
                g.target->DrawImage(brightnessEffect);
                g.target->SetTarget(stash);

                stash->Release();
                brightnessEffect->Release();
                grayscaleEffect->Release();
            }

            if (_separator)
            {
                ID2D1SolidColorBrush* brush = nullptr;
                g.target->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.2f, 0.2f), &brush);
                g.target->FillRectangle(D2D1::RectF(30.0f, 1.0f, g.target->GetSize().width - 5.0f, 2.0f), brush);
                brush->Release();
            }
            else if (_label)
            {
                auto size = g.target->GetSize();
                if (size.width <= 55)
                    size.width = 56;

                g.target->DrawBitmap(
                    _label->Draw(g),
                    D2D1::RectF(30, 0, size.width - 25, size.height)
                );
            }

            // Draw icon/checkmark
            if (_icon && !_checked)
            {
                if (!_disabled)
                    _iconImage->SetImage(_icon);
                else
                    _iconImage->SetImage(_iconGrayscale);
                _iconImage->Draw(g);
                g.target->DrawBitmap(_iconImage->Component::ContentImage(), D2D1::RectF(2.0f, 0.0f, 27.0f, 25.0f));
            }
            if (_checkmarkIcon && _checked)
            {
                g.target->DrawBitmap(_checkmarkIcon, D2D1::RectF(6.25f, 6.25f, 18.75f, 18.75f));
            }

            // Draw expand arrow
            if (_menu)
            {
                if (_menuExpandImage->Redraw())
                    _menuExpandImage->Draw(g);

                auto size = g.target->GetSize();
                g.target->DrawBitmap(_menuExpandImage->Component::ContentImage(), D2D1::RectF(
                    size.width - 25,
                    0,
                    size.width,
                    size.height
                ));
            }
        }

        void _OnResize(int width, int height) override
        {
            if (_label)
                _label->Resize(width - 55, height);
        }

        //EventTargets _OnLeftPressed(int x, int y)
        //{
        //    _onClick();
        //    return EventTargets().Add(this, x, y);
        //}

    public:
        const char* GetName() const override { return Name(); }
        static const char* Name() { return "menu_item"; }
#pragma endregion

    private:
        Id _id;

        std::unique_ptr<Label> _label = nullptr;
        std::optional<MenuTemplate::Menu> _menu = std::nullopt;
        std::unique_ptr<zcom::Image> _menuExpandImage = nullptr;
        std::function<void(bool)> _onClick;
        bool _closeOnClick = true;
        bool _separator = false;
        ID2D1Bitmap* _icon = nullptr;
        ID2D1Bitmap1* _iconGrayscale = nullptr;
        ID2D1Bitmap* _checkmarkIcon = nullptr;

        std::unique_ptr<zcom::Image> _iconImage = nullptr;

        // Exactly 1 item from a check group must be checked at a time
        int _checkGroup = -1;
        bool _checked = false;
        bool _checkable = false;

        bool _disabled = false;

    protected:
        friend class Scene;
        friend class Component;
        MenuItem(Scene* scene) : Component(scene), _id(_GenerateId()) {}
        // Separator
        void Init();
        // Regular button/check item
        void Init(std::wstring text, std::function<void(bool)> onClick = [](bool) {});
        // Deeper menu
        void Init(MenuTemplate::Menu menu, std::wstring text);

        Id _GenerateId()
        {
            static uint64_t id{ 1 };
            return Id{ id++ };
        }
    public:
        ~MenuItem()
        {
            SafeFullRelease((IUnknown**)&_iconGrayscale);
        }
        MenuItem(MenuItem&&) = delete;
        MenuItem& operator=(MenuItem&&) = delete;
        MenuItem(const MenuItem&) = delete;
        MenuItem& operator=(const MenuItem&) = delete;

        Id GetId() const
        {
            return _id;
        }

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

        // Calculates the width of this item, which displays the text without cutoff
        int CalculateWidth()
        {
            if (!_label) return 0;

            _label->SetCutoff(L"");
            int totalWidth = ceilf(_label->GetTextWidth()) + 55;
            _label->SetCutoff(L"...");
            return totalWidth;
        }

        bool IsSeparator() const
        {
            return _separator;
        }

        void SetIcon(ID2D1Bitmap* icon)
        {
            if (icon != _icon)
            {
                if (_icon)
                    SafeFullRelease((IUnknown**)&_iconGrayscale);
                _icon = icon;
                InvokeRedraw();
            }
        }

        void SetCloseOnClick(bool close)
        {
            _closeOnClick = close;
        }

        bool CloseOnClick() const
        {
            return _closeOnClick;
        }

        void SetCheckGroup(int group)
        {
            _checkGroup = group;
        }

        int CheckGroup() const
        {
            return _checkGroup;
        }

        void SetChecked(bool checked)
        {
            _checked = checked;
            InvokeRedraw();
        }

        bool Checked() const
        {
            return _checked;
        }

        void SetCheckable(bool checkable)
        {
            _checkable = checkable;
        }

        bool Checkable() const
        {
            return _checkable;
        }

        void SetDisabled(bool disabled)
        {
            if (disabled != _disabled)
            {
                _disabled = disabled;
                if (_disabled)
                    _label->SetFontColor(D2D1::ColorF(0.4f, 0.4f, 0.4f));
                else
                    _label->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
                InvokeRedraw();
            }
        }

        bool Disabled() const
        {
            return _disabled;
        }
    };
}