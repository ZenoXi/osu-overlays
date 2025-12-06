#pragma once

#include "Panel.h"
#include "Label.h"
#include "Image.h"

#include "Helper/EventEmitter.h"
#include "Window/KeyboardEventHandler.h"

namespace zcom
{
    enum class ButtonPreset
    {
        NO_EFFECTS,
        MINIMAL,
        DEFAULT
    };

    enum class ButtonActivation
    {
        PRESS,
        RELEASE,
        PRESS_OR_RELEASE
    };
    constexpr std::vector<std::pair<int64_t, std::wstring>> ButtonActivationValueProxySelectionValues()
    {
        return {
            { (int64_t)ButtonActivation::PRESS, L"Press" },
            { (int64_t)ButtonActivation::RELEASE, L"Release" },
            { (int64_t)ButtonActivation::PRESS_OR_RELEASE, L"Press or release" }
        };
    }

    class Button : public Panel, public KeyboardEventHandler
    {
    public:
        template<typename T>
        void ValueFromButtonState(Value<T>& value, T defaultValue, T hoveredValue, T clickedValue)
        {
            value.ComputedFrom([defaultValue, hoveredValue, clickedValue](bool hovered, bool clicked) {
                if (clicked)
                    return clickedValue;
                else if (hovered)
                    return hoveredValue;
                else
                    return defaultValue;
            }, buttonHovered_, buttonClicked_);
        }

        DEFINE_COMPONENT(Button, Panel)
        DEFAULT_DESTRUCTOR(Button)
        HIDE_PANEL_METHODS
    protected:
        void Init(std::wstring text, ButtonPreset preset = ButtonPreset::DEFAULT)
        {
            cursorIcon = zwnd::CursorIcon::HAND;
            selectable = true;
            buttonHovered_.ComputedFrom([](bool hovered, bool clicked) {
                return !clicked && hovered;
            }, hovered_, leftClicked_);
            buttonClicked_.ComputedFrom([](bool hoveredArea, bool clicked) {
                return clicked && hoveredArea;
            }, hoveredArea_, leftClicked_);

            _label = Create<zcom::Label>(text);
            _label->parentSize = SizeF{ 1.0f, 1.0f };
            _label->xTextAlign = TextAlignment::CENTER;
            _label->yTextAlign = Alignment::CENTER;

            _image = Create<zcom::Image>();
            _image->parentSize = SizeF{ 1.0f, 1.0f };
            _image->imagePlacement = ImagePlacement::FIT;

            AddItem(_label.get());
            AddItem(_image.get());

            _SetPreset(preset);
        }
        void Init(ButtonPreset preset = ButtonPreset::DEFAULT)
        {
            Init(L"", preset);
        }

    public:
        Value<Color> buttonColor = Value<Color>(Color(), [=](Color& currentValue, const Color& color) {
            currentValue = color;
            InvokeRedraw();
        });
        Value<ButtonActivation> activation = ButtonActivation::RELEASE;

        Value<bool> buttonHovered_ = false;
        Value<bool> buttonClicked_ = false;

        Label* Label()
        {
            return _label.get();
        }

        Image* Image()
        {
            return _image.get();
        }

        [[nodiscard]]
        EventSubscription<void> SubscribeOnActivated(const std::function<void()>& func)
        {
            return _onActivated->Subscribe(func);
        }

    private:
        EventEmitter<void> _onActivated;
        std::unique_ptr<zcom::Label> _label = nullptr;
        std::unique_ptr<zcom::Image> _image = nullptr;

    protected:
        void _SetPreset(ButtonPreset preset)
        {
            switch (preset)
            {
            case ButtonPreset::NO_EFFECTS:
            {
                break;
            }
            case ButtonPreset::MINIMAL:
            {
                ValueFromButtonState<Color>(buttonColor, Color(), Color(0xFFFFFF, 0.1f), Color(0x000000, 0.1f));
                break;
            }
            case ButtonPreset::DEFAULT:
            {
                border.visible = true;
                border.color = Color(0x323232);
                ValueFromButtonState<Color>(buttonColor, Color(0x0C0C0C), Color(0x1A1A1A), Color(0x050505));
                break;
            }
            default:
                break;
            }
        }

        void _OnDraw(Graphics* g) override
        {
            g->FillRectangle(size_->ToRect().ToRectF(), buttonColor);
            Panel::_OnDraw(g);
        }

        EventContext _OnLeftPressed(Point point) override
        {
            if (activation == ButtonActivation::PRESS || activation == ButtonActivation::PRESS_OR_RELEASE)
                _onActivated->InvokeAll();
            return EventContext().Add(this, point);
        }

        EventContext _OnLeftReleased(std::optional<Point> point) override
        {
            if (hoveredArea_)
            {
                if (activation == ButtonActivation::RELEASE || activation == ButtonActivation::PRESS_OR_RELEASE)
                    _onActivated->InvokeAll();
            }
            return EventContext().Add(this, point);
        }

        void _OnSelected(bool reverse) override;

        void _OnDeselected() override;

        bool _OnKeyDown(BYTE vkCode) override
        {
            if (vkCode == VK_RETURN)
            {
                _onActivated->InvokeAll();
                return true;
            }
            return false;
        }

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(ValueProxy::BasicColorValueProxy("button color", std::make_any<Value<Color>*>(&buttonColor)));
            values.push_back(ValueProxy::BasicEnumValueProxy<ButtonActivation>("button activation", std::make_any<Value<ButtonActivation>*>(&activation), ButtonActivationValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicBoolValueProxy("hovered", std::make_any<Value<bool>*>(&buttonHovered_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("clicked", std::make_any<Value<bool>*>(&buttonClicked_)).Computed());

            auto data = Panel::GetReflectionData();
            data.insert(data.begin(), { "Button", std::move(values) });
            return data;
        }
    };
}