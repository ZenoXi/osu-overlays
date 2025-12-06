#include "MenuItem.h"
#include "Dummy.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

void zcom::MenuItem::Init()
{
    FlexPanel::Init(FlexDirection::RIGHT);

    _GenerateId();
    _separator = true;
    
    size = { 0, 3 };
    parentSize = { 1.0f, 0.0f };
    padding = { 30, 1, 5, 1 };

    auto dummy = Create<Dummy>();
    dummy->parentSize = { 1.0f, 1.0f };
    dummy->backgroundColor = Color(0x343434);
    AddItem(std::move(dummy));
}

void zcom::MenuItem::Init(std::wstring text, std::function<void(bool)> onClick)
{
    FlexPanel::Init(FlexDirection::RIGHT);

    _GenerateId();
    _onClick = onClick;

    size = { 0, 25 };
    parentSize = { 1.0f, 0.0f };

    _label = Create<Label>(text);
    _label->autoWidth = true;
    _label->minAutoWidth = minWidth - 50;
    _label->maxAutoWidth = maxWidth - 50;
    _label->parentSize = { 0.0f, 1.0f };
    _label->padding = RectF{ 5.0f };
    _label->yTextAlign = Alignment::CENTER;
    _label->cutoff = L"...";
    _label->font = L"Segoe UI";
    _label->fontSize = 13.0f;
    _label->fontColor.ComputedFrom([](bool disabled) {
        if (disabled)
            return Color(0x666666);
        else
            return Color(0xCCCCCC);
    }, disabled);

    _iconImage = Create<zcom::Image>();
    _iconImage->size = { 25, 25 };
    _iconImage->imagePlacement = ImagePlacement::CENTER;
    _iconImage->snapToPixels = true;
    _iconImage->image.ComputedFrom([](bool disabled, bool checked, std::optional<Bitmap> icon, std::optional<Bitmap> iconGrayscale, std::optional<Bitmap> checkmarkIcon) -> std::optional<Bitmap> {
        if (checked)
            return checkmarkIcon;
        else if (disabled && iconGrayscale)
            return iconGrayscale;
        else
            return icon;
    }, disabled, checked, icon, _iconGrayscale, _checkmarkIcon);
    _iconImage->targetRect.ComputedFrom([](bool checked) -> std::optional<RectF> {
        if (checked)
            return RectF(6.25f, 6.25f, 18.75f, 18.75f);
        else
            return std::nullopt;
    }, checked);

    AddItem(_iconImage.get());
    AddItem(_label.get());

    _checkmarkIcon = _scene->GetWindow()->resourceManager.GetImage("checkmark_50x50");
}

void zcom::MenuItem::Init(MenuTemplate::Menu menu, std::wstring text)
{
    Init(text);

    _menu = std::move(menu);
    _menuExpandImage = Create<zcom::Image>(_scene->GetWindow()->resourceManager.GetImage("menu_arrow_right_7x7"));
    _menuExpandImage->size = { 25, 25 };
    _menuExpandImage->imagePlacement = ImagePlacement::CENTER;
    _menuExpandImage->snapToPixels = true;
    _menuExpandImage->tintColor = Color(0x808080);
    AddItem(_menuExpandImage.get());
}