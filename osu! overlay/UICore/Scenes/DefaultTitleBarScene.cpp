#include "App.h" // App.h and Window.h must be included first
#include "Window/Window.h"
#include "DefaultTitleBarScene.h"

#include "Helper/ResourceManager.h"

void zcom::DefaultTitleBarScene::Init(SceneOptionsBase* options)
{
    DefaultTitleBarSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const DefaultTitleBarSceneOptions*>(options);

    _titleBarHeight = opt.titleBarHeight;
    _captionHeight = opt.captionHeight;
    _tintIcon = !opt.windowIconResourceName;
    _useCleartype = opt.useCleartype;
    _darkMode = opt.darkMode;

    // The following functions set up the default title bar look
    // See the function implementations for details on achieving
    // the default look

    if (!_darkMode)
    {
        SetBackground(D2D1::ColorF(0xFFFFFF));
    }
    else
    {
        SetBackground(D2D1::ColorF(0x202020));
        _activeItemTint = D2D1::ColorF(0xE0E0E0);
    }

    if (opt.showCloseButton)
        AddCloseButton();
    if (opt.showMaximizeButton)
        AddMaximizeButton();
    if (opt.showMinimizeButton)
        AddMinimizeButton();
    if (opt.showIcon)
        AddIcon(_window->resourceManager.GetImage(opt.windowIconResourceName.value_or("window_app_icon")));
    if (opt.showTitle)
        AddTitle(opt.windowTitle);
    AddMenuButton(L"File");
    AddMenuButton(L"Edit");
    AddMenuButton(L"View");
    // After the 'Add*Item*()' calls, the default item appearance and behavior can be modified through their variables

    SubscribeToWindowMessages();
    _basePanel->SubscribePostUpdate([=]() {
        HandleWindowMessages();
    }).Detach();
}


void zcom::DefaultTitleBarScene::SetBackground(D2D1_COLOR_F color)
{
    _basePanel->SetBackgroundColor(color);
    if (_titleLabel && _useCleartype)
        _titleLabel->SetBackgroundColor(color);
}

void zcom::DefaultTitleBarScene::AddCloseButton()
{
    _closeButton = Create<Button>(ButtonPreset::NO_EFFECTS);
    _closeButton->SetBaseSize(45, 29);
    _closeButton->SetHorizontalAlignment(Alignment::END);
    _closeButton->SetButtonImageAll(_window->resourceManager.GetImage("window_close"));
    _closeButton->ButtonImage()->SetPlacement(ImagePlacement::CENTER);
    _closeButton->ButtonImage()->SetPixelSnap(true);
    _closeButton->UseImageParamsForAll(_closeButton->ButtonImage());
    _closeButton->SetButtonColor(D2D1::ColorF(0, 0.0f));
    _closeButton->SetButtonHoverColor(D2D1::ColorF(0xE81123));
    _closeButton->SetButtonClickColor(D2D1::ColorF(0xE81123, 0.54f));
    _closeButton->ButtonImage()->SetTintColor(_activeItemTint);
    _closeButton->ButtonHoverImage()->SetTintColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));
    _closeButton->ButtonClickImage()->SetTintColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));
    _closeButton->SetSelectable(false);
    //_closeButton->SetProperty(PROP_Shadow());
    _closeButton->SetActivation(ButtonActivation::RELEASE);
    _closeButton->SubscribeOnActivated([&]() {
        _window->Close();
    }).Detach();

    _basePanel->AddItem(_closeButton.get());
}

void zcom::DefaultTitleBarScene::AddMaximizeButton()
{
    _maximizeButton = Create<Button>(ButtonPreset::NO_EFFECTS);
    _maximizeButton->SetBaseSize(45, 29);
    _maximizeButton->SetHorizontalAlignment(Alignment::END);
    if (_closeButton)
        _maximizeButton->SetHorizontalOffsetPixels(-45);
    _maximizeButton->SetButtonImageAll(_window->resourceManager.GetImage("window_maximize"));
    _maximizeButton->ButtonImage()->SetPlacement(ImagePlacement::CENTER);
    _maximizeButton->ButtonImage()->SetPixelSnap(true);
    _maximizeButton->ButtonImage()->SetTintColor(_activeItemTint);
    _maximizeButton->UseImageParamsForAll(_maximizeButton->ButtonImage());
    _maximizeButton->SetButtonColor(D2D1::ColorF(0, 0.0f));
    _maximizeButton->SetButtonHoverColor(D2D1::ColorF(0, 0.1f));
    _maximizeButton->SetButtonClickColor(D2D1::ColorF(0, 0.2f));
    _maximizeButton->SetSelectable(false);
    _maximizeButton->SetActivation(ButtonActivation::RELEASE);
    _maximizeButton->SubscribeOnActivated([&]() {
        if (_window->Backend().Maximized())
            _window->Backend().Restore();
        else
            _window->Backend().Maximize();
    }).Detach();

    _basePanel->AddItem(_maximizeButton.get());
}

void zcom::DefaultTitleBarScene::AddMinimizeButton()
{
    _minimizeButton = Create<Button>(ButtonPreset::NO_EFFECTS);
    _minimizeButton->SetBaseSize(45, 29);
    _minimizeButton->SetHorizontalAlignment(Alignment::END);
    if (_closeButton && _maximizeButton)
        _minimizeButton->SetHorizontalOffsetPixels(-90);
    else if (_closeButton || _maximizeButton)
        _minimizeButton->SetHorizontalOffsetPixels(-45);
    _minimizeButton->SetButtonImageAll(_window->resourceManager.GetImage("window_minimize"));
    _minimizeButton->ButtonImage()->SetPlacement(ImagePlacement::CENTER);
    _minimizeButton->ButtonImage()->SetPixelSnap(true);
    _minimizeButton->ButtonImage()->SetTintColor(_activeItemTint);
    _minimizeButton->UseImageParamsForAll(_minimizeButton->ButtonImage());
    _minimizeButton->SetButtonColor(D2D1::ColorF(0, 0.0f));
    _minimizeButton->SetButtonHoverColor(D2D1::ColorF(0, 0.1f));
    _minimizeButton->SetButtonClickColor(D2D1::ColorF(0, 0.2f));
    _minimizeButton->SetSelectable(false);
    _minimizeButton->SetActivation(ButtonActivation::RELEASE);
    _minimizeButton->SubscribeOnActivated([&]() {
        _window->Backend().Minimize();
    }).Detach();

    _basePanel->AddItem(_minimizeButton.get());
}

void zcom::DefaultTitleBarScene::AddIcon(ID2D1Bitmap* icon)
{
    _iconImage = Create<Image>(icon);
    _iconImage->SetBaseSize(29, 29);
    _iconImage->SetPlacement(ImagePlacement::CENTER);
    _iconImage->SetPixelSnap(true);
    if (_tintIcon)
        _iconImage->SetTintColor(D2D1::ColorF(0));

    _basePanel->AddItem(_iconImage.get());
}

void zcom::DefaultTitleBarScene::AddTitle(std::wstring title)
{
    _titleLabel = Create<Label>(title);
    _titleLabel->SetFont(L"Segoe UI");
    _titleLabel->SetFontSize(12.0f);
    _titleLabel->SetFontColor(_activeItemTint);
    _titleLabel->SetBaseSize((int)_titleLabel->GetTextWidth() + 1, 29);
    _titleLabel->SetHorizontalOffsetPixels(5);
    if (_iconImage)
        _titleLabel->SetHorizontalOffsetPixels(_titleLabel->GetHorizontalOffsetPixels() + 29);
    _titleLabel->SetHorizontalTextAlignment(TextAlignment::LEADING);
    _titleLabel->SetVerticalTextAlignment(Alignment::CENTER);

    // Enable ClearType
    if (_useCleartype)
    {
        _titleLabel->IgnoreAlpha(true);
        _titleLabel->SetBackgroundColor(_basePanel->GetBackgroundColor());
    }

    _basePanel->AddItem(_titleLabel.get());
}

void zcom::DefaultTitleBarScene::AddMenuButton(std::wstring name)
{

}

int zcom::DefaultTitleBarScene::TitleBarSceneHeight()
{
    return _titleBarHeight;
}

int zcom::DefaultTitleBarScene::CaptionHeight()
{
    return _captionHeight;
}


RECT zcom::DefaultTitleBarScene::WindowMenuButtonRect()
{
    if (_iconImage)
    {
        return {
            _iconImage->GetX(),
            _iconImage->GetY(),
            _iconImage->GetX() + _iconImage->GetWidth(),
            _iconImage->GetY() + _iconImage->GetHeight()
        };
    }
    else {
        return { 0, 0, 0, 0 };
    }
}

std::vector<RECT> zcom::DefaultTitleBarScene::ExcludedCaptionRects()
{
    std::vector<RECT> excludedRects;

    // Add close button
    if (_closeButton)
    {
        excludedRects.push_back({
            _closeButton->GetX(),
            _closeButton->GetY(),
            _closeButton->GetX() + _closeButton->GetWidth(),
            _closeButton->GetY() + _closeButton->GetHeight()
        });
    }

    // Add minimixe button
    if (_minimizeButton)
    {
        excludedRects.push_back({
            _minimizeButton->GetX(),
            _minimizeButton->GetY(),
            _minimizeButton->GetX() + _minimizeButton->GetWidth(),
            _minimizeButton->GetY() + _minimizeButton->GetHeight()
        });
    }

    // Add maximize button
    if (_maximizeButton)
    {
        excludedRects.push_back({
            _maximizeButton->GetX(),
            _maximizeButton->GetY(),
            _maximizeButton->GetX() + _maximizeButton->GetWidth(),
            _maximizeButton->GetY() + _maximizeButton->GetHeight()
        });
    }

    // Add menu buttons
    for (int i = 0; i < _menuButtons.size(); i++)
    {
        excludedRects.push_back({
            _menuButtons[i]->GetX(),
            _menuButtons[i]->GetY(),
            _menuButtons[i]->GetX() + _menuButtons[i]->GetWidth(),
            _menuButtons[i]->GetY() + _menuButtons[i]->GetHeight()
        });
    }

    return excludedRects;
}

void zcom::DefaultTitleBarScene::SubscribeToWindowMessages()
{
    _windowMessageSubscription = _window->SubscribeToWindowMessages(nullptr);
}

void zcom::DefaultTitleBarScene::HandleWindowMessages()
{
    if (_windowMessageSubscription)
    {
        _windowMessageSubscription->HandlePendingEvents([=](zwnd::WindowMessage message) {
            if (message.id == zwnd::WindowActivateMessage::ID())
            {
                zwnd::WindowActivateMessage msg{};
                msg.Decode(message);
                D2D1_COLOR_F newColor{};
                if (msg.activationType == zwnd::WindowActivateMessage::ACTIVATED || msg.activationType == zwnd::WindowActivateMessage::CLICK_ACTIVATED)
                {
                    newColor = _activeItemTint;
                    _basePanel->InvokeRedraw();
                }
                else
                {
                    newColor = _inactiveItemTint;
                    _basePanel->InvokeRedraw();
                }

                if (_closeButton)
                    _closeButton->ButtonImage()->SetTintColor(newColor);
                if (_maximizeButton)
                    _maximizeButton->ButtonImage()->SetTintColor(newColor);
                if (_minimizeButton)
                    _minimizeButton->ButtonImage()->SetTintColor(newColor);
                if (_titleLabel)
                    _titleLabel->SetFontColor(newColor);
            }
            else if (message.id == zwnd::WindowSizeExMessage::ID())
            {
                if (_maximizeButton)
                {
                    zwnd::WindowSizeExMessage msg{};
                    msg.Decode(message);

                    if (msg.flags.windowMaximized)
                        _maximizeButton->SetButtonImageAll(_window->resourceManager.GetImage("window_restore"));
                    else if (msg.flags.windowRestored)
                        _maximizeButton->SetButtonImageAll(_window->resourceManager.GetImage("window_maximize"));
                }
            }
        });
    }
}