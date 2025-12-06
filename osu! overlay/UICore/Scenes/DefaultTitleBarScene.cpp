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

    _contentPanel = Create<FlexPanel>(FlexDirection::RIGHT);
    _contentPanel->parentSize = { 1.0f, 1.0f };
    _basePanel->AddItem(_contentPanel.get());

    // The following functions set up the default title bar look
    // See the function implementations for details on achieving
    // the default look

    if (!_darkMode)
    {
        SetBackground(Color(0xFFFFFF));
    }
    else
    {
        SetBackground(Color(0x202020));
        _activeItemTint = Color(0xE0E0E0);
    }

    if (opt.showIcon)
        AddIcon(_window->resourceManager.GetImage(opt.windowIconResourceName.value_or("window_app_icon")));
    AddMenuButton(L"File");
    AddMenuButton(L"Edit");
    AddMenuButton(L"View");
    if (opt.showTitle)
        AddTitle(opt.windowTitle);
    if (opt.showMinimizeButton)
        AddMinimizeButton();
    if (opt.showMaximizeButton)
        AddMaximizeButton();
    if (opt.showCloseButton)
        AddCloseButton();
    // After the 'Add*Item*()' calls, the default item appearance and behavior can be modified through their variables

    SubscribeToWindowMessages();
    _basePanel->SubscribePostUpdate([=]() {
        HandleWindowMessages();
    }).Detach();
}

void zcom::DefaultTitleBarScene::Uninit()
{
    _basePanel->ClearItems();
}


void zcom::DefaultTitleBarScene::SetBackground(Color color)
{
    _basePanel->backgroundColor = color;
    if (_titleLabel && _useCleartype)
        _titleLabel->backgroundColor = color;
}

void zcom::DefaultTitleBarScene::AddCloseButton()
{
    _closeButton = Create<Button>(ButtonPreset::NO_EFFECTS);
    _closeButton->AddTag("close_button");
    _closeButton->size = { 45, 29 };
    _closeButton->Image()->image = _window->resourceManager.GetImage("window_close");
    _closeButton->Image()->imagePlacement = ImagePlacement::CENTER;
    _closeButton->Image()->snapToPixels = true;
    _closeButton->ValueFromButtonState<Color>(_closeButton->Image()->tintColor, _activeItemTint, Color(0xFFFFFF), Color(0xFFFFFF));
    _closeButton->ValueFromButtonState<Color>(_closeButton->buttonColor, Color(0, 0.0f), Color(0xE81123, 1.0f), Color(0xE81123, 0.54f));
    _closeButton->selectable = false;
    _closeButton->SubscribeOnActivated([&]() {
        _window->Close();
    }).Detach();

    _contentPanel->AddItem(_closeButton.get());
}

void zcom::DefaultTitleBarScene::AddMaximizeButton()
{
    _maximizeButton = Create<Button>(ButtonPreset::NO_EFFECTS);
    _maximizeButton->AddTag("maximize_button");
    _maximizeButton->size = { 45, 29 };
    _maximizeButton->Image()->image = _window->resourceManager.GetImage("window_maximize");
    _maximizeButton->Image()->imagePlacement = ImagePlacement::CENTER;
    _maximizeButton->Image()->snapToPixels = true;
    _maximizeButton->ValueFromButtonState<Color>(_maximizeButton->Image()->tintColor, _activeItemTint, Color(0xFFFFFF), Color(0xFFFFFF));
    _maximizeButton->ValueFromButtonState<Color>(_maximizeButton->buttonColor, Color(0, 0.0f), Color(0, 0.1f), Color(0, 0.2f));
    _maximizeButton->selectable = false;
    _maximizeButton->SubscribeOnActivated([&]() {
        if (_window->Backend().Maximized())
            _window->Backend().Restore();
        else
            _window->Backend().Maximize();
    }).Detach();

    _contentPanel->AddItem(_maximizeButton.get());
}

void zcom::DefaultTitleBarScene::AddMinimizeButton()
{
    _minimizeButton = Create<Button>(ButtonPreset::NO_EFFECTS);
    _minimizeButton->AddTag("minimize_button");
    _minimizeButton->size = { 45, 29 };
    _minimizeButton->Image()->image = _window->resourceManager.GetImage("window_minimize");
    _minimizeButton->Image()->imagePlacement = ImagePlacement::CENTER;
    _minimizeButton->Image()->snapToPixels = true;
    _minimizeButton->ValueFromButtonState<Color>(_minimizeButton->Image()->tintColor, _activeItemTint, Color(0xFFFFFF), Color(0xFFFFFF));
    _minimizeButton->ValueFromButtonState<Color>(_minimizeButton->buttonColor, Color(0, 0.0f), Color(0, 0.1f), Color(0, 0.2f));
    _minimizeButton->selectable = false;
    _minimizeButton->SubscribeOnActivated([&]() {
        _window->Backend().Minimize();
    }).Detach();

    _contentPanel->AddItem(_minimizeButton.get());
}

void zcom::DefaultTitleBarScene::AddIcon(std::optional<Bitmap> icon)
{
    _iconImage = Create<Image>(icon);
    _iconImage->AddTag("icon_image");
    _iconImage->size = { 29, 29 };
    _iconImage->imagePlacement = ImagePlacement::CENTER;
    _iconImage->snapToPixels = true;
    if (_tintIcon)
        _iconImage->tintColor = Color(0);

    _contentPanel->AddItem(_iconImage.get());
}

void zcom::DefaultTitleBarScene::AddTitle(std::wstring title)
{
    _titleLabel = Create<Label>(title);
    _titleLabel->AddTag("title_label");
    _titleLabel->font = L"Segoe UI";
    _titleLabel->fontSize = 12.0f;
    _titleLabel->fontColor = _activeItemTint;
    _titleLabel->SetProperty(FlexGrow());
    _titleLabel->size = { 0, 29 };
    _titleLabel->padding = { 5.0f, 0.0f, 1.0f, 0.0f };
    _titleLabel->xTextAlign = TextAlignment::LEADING;
    _titleLabel->yTextAlign = Alignment::CENTER;

    // Enable ClearType
    if (_useCleartype)
    {
        _titleLabel->ignoreAlpha = true;
        _titleLabel->backgroundColor = _basePanel->backgroundColor.Get();
    }

    _contentPanel->AddItem(_titleLabel.get());
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
            _iconImage->position_->x,
            _iconImage->position_->y,
            _iconImage->position_->x + _iconImage->size_->width,
            _iconImage->position_->y + _iconImage->size_->height
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
            _closeButton->position_->x,
            _closeButton->position_->y,
            _closeButton->position_->x + _closeButton->size_->width,
            _closeButton->position_->y + _closeButton->size_->height
        });
    }

    // Add minimixe button
    if (_minimizeButton)
    {
        excludedRects.push_back({
            _minimizeButton->position_->x,
            _minimizeButton->position_->y,
            _minimizeButton->position_->x + _minimizeButton->size_->width,
            _minimizeButton->position_->y + _minimizeButton->size_->height
        });
    }

    // Add maximize button
    if (_maximizeButton)
    {
        excludedRects.push_back({
            _maximizeButton->position_->x,
            _maximizeButton->position_->y,
            _maximizeButton->position_->x + _maximizeButton->size_->width,
            _maximizeButton->position_->y + _maximizeButton->size_->height
        });
    }

    // Add menu buttons
    for (int i = 0; i < _menuButtons.size(); i++)
    {
        excludedRects.push_back({
            _menuButtons[i]->position_->x,
            _menuButtons[i]->position_->y,
            _menuButtons[i]->position_->x + _menuButtons[i]->size_->width,
            _menuButtons[i]->position_->y + _menuButtons[i]->size_->height
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
                Color newColor{};
                if (msg.activationType == zwnd::WindowActivateMessage::ACTIVATED || msg.activationType == zwnd::WindowActivateMessage::CLICK_ACTIVATED)
                {
                    newColor = _activeItemTint;
                    _windowIsActive = true;
                    _basePanel->InvokeRedraw();
                }
                else
                {
                    newColor = _inactiveItemTint;
                    _windowIsActive = false;
                    _basePanel->InvokeRedraw();
                }

                if (_closeButton)
                    _closeButton->ValueFromButtonState<Color>(_closeButton->Image()->tintColor, newColor, Color(0xFFFFFF), Color(0xFFFFFF));
                if (_maximizeButton)
                    _maximizeButton->ValueFromButtonState<Color>(_maximizeButton->Image()->tintColor, newColor, Color(0xFFFFFF), Color(0xFFFFFF));
                if (_minimizeButton)
                    _minimizeButton->ValueFromButtonState<Color>(_minimizeButton->Image()->tintColor, newColor, Color(0xFFFFFF), Color(0xFFFFFF));
                if (_titleLabel)
                    _titleLabel->fontColor = newColor;
            }
            else if (message.id == zwnd::WindowSizeExMessage::ID())
            {
                if (_maximizeButton)
                {
                    zwnd::WindowSizeExMessage msg{};
                    msg.Decode(message);

                    if (msg.flags.windowMaximized)
                        _maximizeButton->Image()->image = _window->resourceManager.GetImage("window_restore");
                    else if (msg.flags.windowRestored)
                        _maximizeButton->Image()->image = _window->resourceManager.GetImage("window_maximize");
                }
            }
        });
    }
}