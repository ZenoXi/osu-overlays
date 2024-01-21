#include "MenuItem.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

void zcom::MenuItem::Init()
{
    _separator = true;

    SetBaseHeight(3);
    SetParentWidthPercent(1.0f);
}

void zcom::MenuItem::Init(std::wstring text, std::function<void(bool)> onClick)
{
    _onClick = onClick;

    _label = Create<Label>(text);
    _label->Resize(GetWidth() - 50, GetHeight());
    _label->SetVerticalTextAlignment(zcom::Alignment::CENTER);
    _label->SetMargins({ 5.0f });
    _label->SetCutoff(L"...");
    _label->SetFont(L"Segoe UI");
    _label->SetFontSize(13.0f);
    _label->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));

    _iconImage = Create<zcom::Image>();
    _iconImage->SetSize(25, 25);
    _iconImage->SetPlacement(ImagePlacement::CENTER);

    _checkmarkIcon = _scene->GetWindow()->resourceManager.GetImage("checkmark_50x50");

    SetBaseHeight(25);
    SetParentWidthPercent(1.0f);
}

void zcom::MenuItem::Init(MenuTemplate::Menu menu, std::wstring text)
{
    Init(text);

    _menu = std::move(menu);
    _menuExpandImage = Create<zcom::Image>(_scene->GetWindow()->resourceManager.GetImage("menu_arrow_right_7x7"));
    _menuExpandImage->SetSize(25, 25);
    _menuExpandImage->SetPlacement(ImagePlacement::CENTER);
    _menuExpandImage->SetTintColor(D2D1::ColorF(0.5f, 0.5f, 0.5f));
}